/*=============================================================================
Vitalnix User Management Suite
clutils/groupdel.c - Delete a group
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2007
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
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include <vitalnix/libvxpdb/xwfunc.h>
#include <vitalnix/libvxutil/libvxutil.h>

enum {
    E_SUCCESS = 0,
    E_OTHER,      // other error, see errno
    E_OPEN,       // unable to open back-end module or DB
    E_NOEXIST,    // group does not exist
    E_UPDATE,     // db->groupdel() did not return ok
    E_CLOSE,      // db->close() did not return ok
    E_PRIMARY,    // will not delete a user's primary group, unless -F is given
};

// Functions
static int groupdel_main2(struct vxpdb_state *);
static int groupdel_main3(struct vxpdb_state *);
static int groupdel_check_pri_group(struct vxpdb_state *, struct vxpdb_group *);
static int groupdel_get_options(int *, const char ***);
static int groupdel_read_config(void);

// Variables
static int force_deletion = 0;
static const char *action_before = NULL, *action_after = NULL,
    *driver_name = "*", *group_name;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct vxpdb_state *db;
    int ret;

    if(groupdel_read_config() <= 0 || groupdel_get_options(&argc, &argv) <= 0)
        return E_OTHER;

    if((db = vxpdb_load(driver_name)) == NULL) {
        fprintf(stderr, "Could not load PDB driver module \"%s\": %s\n",
                driver_name, strerror(errno));
        return E_OPEN;
    }

    ret = groupdel_main2(db);
    vxpdb_unload(db);
    return ret;
}

static int groupdel_main2(struct vxpdb_state *db)
{
    int ret;
    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open PDB back-end: %s\n", strerror(-ret));
        return E_OPEN;
    }

    ret = groupdel_main3(db);
    vxpdb_close(db);
    return ret;
}

static int groupdel_main3(struct vxpdb_state *db)
{
    struct vxpdb_group group_info;
    struct HXoption ext_catalog[] = {
        {.sh = 'G', .type = HXTYPE_STRING, .ptr = &group_name},
        HXOPT_TABLEEND,
    };
    int ret;

    if((ret = vxpdb_getgrnam(db, group_name, &group_info)) < 0) {
        fprintf(stderr, "Error querying the PDB: %s\n", strerror(-ret));
        return E_OTHER;
    } else if(ret == 0) {
        fprintf(stderr, "Group \"%s\" does not exist\n", group_name);
        return E_NOEXIST;
    }

    if(!force_deletion && groupdel_check_pri_group(db, &group_info)) {
        fprintf(stderr, "Will not remove a user's primary group\n");
        return E_PRIMARY;
    }

    if(action_before != NULL)
        vxutil_replace_run(action_before, ext_catalog);

    group_info.gr_name = static_cast(char *, group_name);
    group_info.gr_gid  = PDB_NOGID;
    if((ret = vxpdb_groupdel(db, &group_info)) <= 0) {
        fprintf(stderr, "Error: Deleting group failed: %s\n", strerror(-ret));
        return E_UPDATE;
    } else if(action_after != NULL) {
        vxutil_replace_run(action_after, ext_catalog);
    }

    return E_SUCCESS;
}

//-----------------------------------------------------------------------------
static int groupdel_check_pri_group(struct vxpdb_state *db,
  struct vxpdb_group *mm)
{
    struct vxpdb_user user;
    int pg = 0; // some user has this as primary group
    void *travp;

    if((travp = vxpdb_usertrav_init(db)) == NULL)
        return 0;

    while(vxpdb_usertrav_walk(db, travp, &user) > 0)
        if((user.pw_gid != PDB_NOGID && user.pw_gid == mm->gr_gid) ||
         (user.pw_igrp != NULL && mm->gr_name != NULL &&
         strcmp(user.pw_igrp, mm->gr_name) == 0)) {
            pg = 1;
            break;
        }

    vxpdb_usertrav_free(db, travp);
    return pg;
}

static int groupdel_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-userdel options
        {.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &action_after,
         .help = "Program to run after group deletion", .htyp = "cmd"},
        {.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &action_before,
         .help = "Program to run before group deletion", .htyp = "cmd"},
        {.sh = 'F', .type = HXTYPE_NONE, .ptr = &force_deletion,
         .help = "Force deletion of group even if users have it as primary group"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &driver_name,
         .help = "Use a different module than \"*\" (the default)", .htyp = "name"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;
    if(argv[1] == NULL) { /* Group name is mandatory */
        fprintf(stderr, "You need to specify a group name.\n");
        return 0;
    }

    return 1;
}

static int groupdel_read_config(void) {
    static const struct HXoption config_table[] = {
        {.ln = "GROUP_PREDEL", .type = HXTYPE_STRING, .ptr = &action_before},
        {.ln = "GROUP_PREDEL", .type = HXTYPE_STRING, .ptr = &action_after},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(CONFIG_SYSCONFDIR "/groupdel.conf", config_table);
}

//=============================================================================
