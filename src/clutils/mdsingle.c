/*=============================================================================
Vitalnix User Management Suite
clutils/mdsingle.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2006
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libHX.h>
#include "vitalnix-config.h"
#include "libvxcli/libvxcli.h"
#include "libvxeds/libvxeds.h"
#include "libvxmdsync/libvxmdsync.h"
#include "libvxpdb/config.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"
#include "libvxpdb/xwfunc.h"
#include "libvxutil/libvxutil.h"

// Structures
struct private_info {
    char *backend_module, *bday, *first_name, *group_name, *pref_username,
         *pvgrp, *surname, *uuid;
    int interactive, run_master;

    int open_status;
    struct vxpdb_state *module_handle;
    struct mdsync_workspace *mdsw;
};

// Functions
static int single_init(struct private_info *);
static int single_run(struct private_info *);
static void single_cleanup(struct private_info *);
static void single_interactive(struct private_info *);
static int validate_group(const struct vxcq_entry *);

static int get_options(int *, const char ***, struct private_info *);
static void show_version(const struct HXoptcb *);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct private_info priv;
    int ret = EXIT_SUCCESS;

    memset(&priv, 0, sizeof(priv));
    priv.backend_module = HX_strdup("*");

    fprintf(stderr, "Note that mdsingle will not check if a user with the "
                    "same UUID already exists. Make sure you do not add a "
                    "user twice by accident.\n");
    if(!get_options(&argc, &argv, &priv))
        return EXIT_FAILURE;
    if(priv.interactive)
        single_interactive(&priv);
    if(!single_init(&priv) || !single_run(&priv))
        return EXIT_FAILURE;

    single_cleanup(&priv);
    return ret;
}

static int single_init(struct private_info *priv) {
    struct mdsync_workspace *mdsw;
    int ret;

    if(priv->group_name == NULL || priv->first_name == NULL) {
        fprintf(stderr, "You have to specify either -f and -g, or -I. "
                "Use -? to get help.\n");
        return 0;
    }

    if((priv->module_handle = vxpdb_load(priv->backend_module)) == NULL) {
        fprintf(stderr, "Could not load PDB back-end: %s\n", strerror(errno));
        return 0;
    }

    if((ret = vxpdb_open(priv->module_handle, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open PDB back-end: %s\n", strerror(-ret));
        return 0;
    }

    priv->open_status = 1;

    if((priv->mdsw = mdsw = mdsync_init()) == NULL) {
        perror("mdsync_init()");
        return 0;
    }

    mdsw->database     = priv->module_handle;
    mdsw->user_private = priv;
    return 1;
}

static int single_run(struct private_info *priv) {
    struct mdsync_workspace *mdsw = priv->mdsw;
    char *username, *password, tmp[8];
    struct vxeds_entry ee;
    int ret;

    if((ret = mdsync_prepare_group(mdsw, priv->group_name)) < 0) {
        fprintf(stderr, "Error querying the PDB: %s\n", strerror(-ret));
        return 0;
    } else if(ret == 0) {
        fprintf(stderr, "Group \"%s\" does not exist\n", priv->group_name);
        return 0;
    }

    memset(&ee, 0, sizeof(ee));
    ee.first_name = HX_strdup(priv->first_name);
    ee.surname    = HX_strdup(priv->surname);
    ee.full_name  = vxeds_bfullname(ee.first_name, ee.surname);
    ee.pvgrp      = HX_strdup(priv->pvgrp);
    if(priv->pref_username == NULL)
        ee.username = HX_strdup(vxutil_propose_lname(tmp, sizeof(tmp),
                      ee.surname, ee.first_name));
    else
        ee.username = priv->pref_username;

    // The space allocated for ->pref_username is handed over to EDS.
    priv->pref_username = NULL;

    if((ee.uuid = priv->uuid) == NULL && priv->bday != NULL)
        ee.uuid = vxuuid_vx3(ee.full_name, vxutil_string_iday(priv->bday));

    HXbtree_add(mdsw->add_req, ee.uuid, HX_memdup(&ee, sizeof(ee)));
    mdsync_compare_simple(mdsw);
    mdsync_fixup(mdsw);

    if(!priv->run_master) {
        struct vxconfig_useradd *c = &mdsw->config.add_opts;
        HX_strclone(&c->master_preadd, NULL);
        HX_strclone(&c->master_postadd, NULL);
    }
    if((ret = mdsync_add(mdsw)) <= 0) {
        printf("mdsync_add(): %s\n", strerror(-ret));
        return 0;
    }

    HX_chomp(mdsw->output_data);
    username = mdsw->output_data;
    if((password = strchr(mdsw->output_data, ':')) != NULL)
        *password++ = '\0';
    printf("Successfully added \"%s\" with password \"%s\"\n",
           username, password);
    return 1;
}

static void single_cleanup(struct private_info *priv) {
    if(priv->mdsw != NULL)
        mdsync_free(priv->mdsw);
    if(priv->module_handle != NULL) {
        if(priv->open_status)
            vxpdb_close(priv->module_handle);
        vxpdb_unload(priv->module_handle);
    }
    free(priv->backend_module);
    free(priv->bday);
    free(priv->first_name);
    free(priv->group_name);
    /* Do not free(priv->pref_username); -- that pointer
    belongs to the EDS subsystem. */
    free(priv->pvgrp);
    free(priv->surname);
    free(priv->uuid);
    return;
}

static void single_interactive(struct private_info *priv) {
    char tmp[9];
    struct vxcq_entry table_1[] = {
        {
            .msg    = "Enter the first name of the new user.\n",
            .prompt = "First name",
            .defl   = priv->first_name,
            .type   = HXTYPE_STRING,
            .ptr    = &priv->first_name,
            .flags  = VXCQ_ZNULL,
        },
        {
            .msg    = "Enter the last name of the new user.\n",
            .prompt = "Last name",
            .defl   = priv->surname,
            .type   = HXTYPE_STRING,
            .ptr    = &priv->surname,
            .flags  = VXCQ_EMPTY | VXCQ_ZNULL,
        },
        VXCQ_TABLE_END,
    };
    struct vxcq_entry table_2[] = {
        {
            .msg    = "This is the proposed username of the new account, "
                      "which you may change. If necessary, Vitalnix will add "
                      "an index number to the username or adjust it to avoid "
                      "conflict with an already existing username.\n",
            .prompt = "Preferred username",
            .defl   = tmp,
            .type   = HXTYPE_STRING,
            .ptr    = &priv->pref_username,
            .flags  = VXCQ_ZNULL,
        },
        {
            .msg      = "Enter the group name or GID to add the user to.\n",
            .prompt   = "System group",
            .defl     = priv->group_name,
            .type     = HXTYPE_STRING,
            .ptr      = &priv->group_name,
            .uptr     = priv->backend_module,
            .flags    = VXCQ_ZNULL,
            .validate = validate_group,
        },
        {
            .msg    = "Enter a user-defined subgroup of the new user.\n",
            .prompt = "Private group/Class",
            .defl   = priv->pvgrp,
            .type   = HXTYPE_STRING,
            .ptr    = &priv->pvgrp,
            .flags  = VXCQ_EMPTY | VXCQ_ZNULL,
        },
        {
            .msg    = "If there is no external UUID available to uniquely "
                      "identify the user, you can have one made up from the "
                      "name-day tuple. In that case, enter the birthdate of "
                      "the user, otherwise leave blank.\n",
            .prompt = "YYYY-MM-DD",
            .defl   = priv->bday,
            .type   = HXTYPE_STRING,
            .ptr    = &priv->bday,
            .flags  = VXCQ_EMPTY | VXCQ_ZNULL,
        },
        VXCQ_TABLE_END,
    };
    struct vxcq_entry table_3[] = {
        {
            .msg    = "Enter the external UUID of the user. If you specify "
                      "something here, it will override the automatic UUID "
                      "generation from the birthdate.\n",
            .prompt = "UUID string",
            .defl   = priv->uuid,
            .type   = HXTYPE_STRING,
            .ptr    = &priv->uuid,
            .flags  = VXCQ_EMPTY | VXCQ_ZNULL,
        },
        VXCQ_TABLE_END,
    };

    vxcli_query_v(table_1);
    if(!isupper(*priv->first_name))
        printf("WARNING: The first char of the name is not uppercase, which "
               "is incorrect in most cases.\n");

    vxutil_propose_lname(tmp, sizeof(tmp), priv->surname, priv->first_name);
    vxcli_query_v(table_2);
    if(priv->bday == NULL)
        vxcli_query_v(table_3);

    return;
}

static int validate_group(const struct vxcq_entry *e) {
    struct vxpdb_group group = {};
    struct vxpdb_state *mh;
    const char *group_name = *(const char **)e->ptr;
    int ret;

    if((mh = vxpdb_load(e->uptr)) == NULL) {
        fprintf(stderr, "vxpdb_load(): %s\n", strerror(errno));
        return 0;
    }
    if((ret = vxpdb_open(mh, 0)) <= 0) {
        fprintf(stderr, "vxpdb_open(): %s\n", strerror(-ret));
        goto out_open;
    }

    if(vxutil_only_digits(group_name))
        ret = vxpdb_getgrgid(mh, strtol(group_name, NULL, 0), &group);
    else
        ret = vxpdb_getgrnam(mh, group_name, &group);

    if(ret < 0)
        fprintf(stderr, "vxpdb_getgr*(): %s\n", strerror(-ret));
    else if(ret == 0)
        fprintf(stderr, "Group \"%s\" does not exist\n", group_name);

    vxpdb_group_free(&group, 0);

 out_open:
    vxpdb_unload(mh);
    return ret > 0;
}

//-----------------------------------------------------------------------------
static int get_options(int *argc, const char ***argv, struct private_info *p) {
    struct HXoption options_table[] = {
        {.ln = "master", .type = HXTYPE_NONE, .ptr = &p->run_master,
         .help = "Do run MASTER_* scripts"},
        {.sh = 'I', .type = HXTYPE_NONE, .ptr = &p->interactive,
         .help = "Run in interactive mode"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &p->backend_module,
         .help = "Backend module", .htyp = "NAME"},
        {.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
         .help = "Show version information"},
        {.sh = 'b', .type = HXTYPE_STRING, .ptr = &p->bday, .htyp = "BDAY",
         .help = "Generate UUID from birthdate (overridden by -x)"},
        {.sh = 'f', .type = HXTYPE_STRING, .ptr = &p->first_name,
         .help = "First name of the user"},
        {.sh = 'g', .type = HXTYPE_STRING, .ptr = &p->group_name,
         .help = "System group to put this user in", .htyp = "GROUP"},
        {.sh = 'p', .type = HXTYPE_STRING, .ptr = &p->pvgrp,
         .help = "User-defined private group id"},
        {.sh = 's', .type = HXTYPE_STRING, .ptr = &p->surname,
         .help = "Last name of the user"},
        {.sh = 'x', .type = HXTYPE_STRING, .ptr = &p->uuid,
         .help = "Unique identifier for user", .htyp = "UUID"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi) {
    printf("Vitalnix " VITALNIX_VERSION " mdsingle\n");
    exit(EXIT_SUCCESS);
}

//=============================================================================