/*=============================================================================
Vitalnix User Management Suite
clutils/mdfixuuid.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include <vitalnix/libvxpdb/xwfunc.h>
#include <vitalnix/libvxutil/libvxutil.h>

// Definitions
struct mdf_priv {
    char *database, *bday, *realname, *username;
};

// Functions
static char *rebuild_uuid(const struct mdf_priv *, struct vxpdb_state *);
static int get_options(int *, const char ***, struct mdf_priv *);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct vxpdb_user sr_mask, mod_mask;
    struct vxpdb_state *db;
    struct mdf_priv p = {};
    int ret;

    p.database = "*";
    if(!get_options(&argc, &argv, &p))
        return EXIT_FAILURE;
    if((db = vxpdb_load(p.database)) == NULL) {
        fprintf(stderr, "Error loading database: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Error opening database: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    }

    vxpdb_user_clean(&sr_mask);
    vxpdb_user_nomodify(&mod_mask);
    sr_mask.pw_name  = p.username;
    mod_mask.pw_real = p.realname;
    mod_mask.vs_uuid = rebuild_uuid(&p, db);
    if(mod_mask.vs_uuid == NULL)
        return EXIT_FAILURE;

    if((ret = vxpdb_usermod(db, &sr_mask, &mod_mask)) <= 0) {
        fprintf(stderr, "User update unsuccessful: %s\n", strerror(-ret));
        return EXIT_FAILURE;
    }

    vxpdb_close(db);
    vxpdb_unload(db);
    return EXIT_SUCCESS;
}

static char *rebuild_uuid(const struct mdf_priv *p, struct vxpdb_state *db) {
    struct vxpdb_user info = {};
    char *res = NULL, *name;
    long xday = 0;
    int ret;

    if((ret = vxpdb_getpwnam(db, p->username, &info)) < 0) {
        fprintf(stderr, "Error querying database: %s\n", strerror(-ret));
        return NULL;
    } else if(ret == 0) {
        fprintf(stderr, "User \"%s\" not found\n", p->username);
        return NULL;
    }

    if(p->bday != NULL) {
        if((xday = vxutil_string_xday(p->bday)) == -1) {
            fprintf(stderr, "Invalid date \"%s\"\n", p->bday);
            goto out;
        }
    } else if(info.vs_uuid != NULL) {
        xday = vxuuid_vx3_get_xday(info.vs_uuid);
        if(xday == -1)
            xday = 0;
    }

    if((name = p->realname) == NULL)
        name = info.pw_real;
    if(name != NULL)
        res = vxuuid_vx3(name, xday);

 out:
    vxpdb_user_free(&info, 0);
    return res;
}

//-----------------------------------------------------------------------------
static int get_options(int *argc, const char ***argv, struct mdf_priv *p) {
    struct HXoption options_table[] = {
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &p->database,
         .help = "Use specified database", .htyp = "name"},
        {.sh = 'b', .type = HXTYPE_STRING, .ptr = &p->bday,
         .help = "New birthdate", .htyp = "date"},
        {.sh = 'r', .type = HXTYPE_STRING, .ptr = &p->realname,
         .help = "New realname"},
        {.sh = 'u', .type = HXTYPE_STRING, .ptr = &p->username,
         .help = "Username to operate on"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;
    if(p->username == NULL || (p->bday == NULL && p->realname == NULL)) {
        fprintf(stderr, "You need to specify the -u option and at least one "
                "of -b or -r\n");
        return 0;
    }
    return 1;
}

//=============================================================================
