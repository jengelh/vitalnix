/*=============================================================================
Vitalnix User Management Suite
clutils/groupadd.c - Create a new group
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
    E_OTHER,       // other error, see errno
    E_OPEN,        // unable to open module database or driver
    E_GID_USED,    // GID already used and -o was not specified
    E_NAME_USED,   // group already exists
    E_UPDATE,      // db->groupadd() did not return ok
    E_CLOSE,       // db->close() did not return ok
};

// Functions
static int groupadd_main2(struct vxpdb_state *);
static int groupadd_main3(struct vxpdb_state *);
static int groupadd_get_options(int *, const char ***);
static int groupadd_read_config(void);

// Variables
static long group_id   = PDB_NOGID;
static int allow_dup   = 0;
static int request_sys = 0;
static const char *action_before = NULL, *action_after = NULL,
    *database_name = "*", *group_name;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct vxpdb_state *db;
    int ret;

    if(groupadd_read_config() <= 0 || groupadd_get_options(&argc, &argv) <= 0)
        return E_OTHER;

    if((db = vxpdb_load(database_name)) == NULL) {
        fprintf(stderr, "Could not load database \"%s\": %s\n",
                database_name, strerror(errno));
        return E_OPEN;
    }

    group_name = argv[1];
    ret = groupadd_main2(db);
    vxpdb_unload(db);
    return ret;
}

static int groupadd_main2(struct vxpdb_state *db)
{
    int ret;
    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open PDB: %s\n", strerror(-ret));
        return E_OPEN;
    }

    ret = groupadd_main3(db);
    vxpdb_close(db);
    return ret;
}

static int groupadd_main3(struct vxpdb_state *db)
{
    struct vxpdb_group group_info;
    struct HXoption ext_catalog[] = {
        {.sh = 'G', .type = HXTYPE_STRING, .ptr = &group_name},
        {.sh = 'g', .type = HXTYPE_LONG,   .ptr = &group_id},
        HXOPT_TABLEEND,
    };
    int ret;

    if((ret = vxpdb_getgrnam(db, group_name, NULL)) < 0) {
        fprintf(stderr, "Error querying the PDB: %s\n", strerror(-ret));
        return E_OTHER;
    } else if(ret > 0) {
        fprintf(stderr, "Group \"%s\" already exists\n", group_name);
        return E_NAME_USED;
    }

    if(group_id != PDB_NOGID) {
        /* -g (explicit GID) was passed */
        if(!allow_dup && vxpdb_getgrgid(db, group_id, NULL) > 0) {
            /* The -o flag (allow creating group with duplicate GID)
            was not passed. */
            fprintf(stderr, "Group with GID %ld already exists."
                    " Use -o to override.\n", group_id);
            return E_GID_USED;
        }
    } else if(request_sys) { /* -r flag passed */
        group_id = vxpdb_modctl(db, PDB_NEXTGID_SYS, db);
    }

    if(action_before != NULL)
        vxutil_replace_run(action_before, ext_catalog);

    group_info.gr_name = const_cast(char *, group_name);
    group_info.gr_gid  = group_id;

    if((ret = vxpdb_groupadd(db, &group_info)) <= 0) {
        fprintf(stderr, "Warning: Group addition failed: %s\n", strerror(-ret));
        return E_UPDATE;
    } else if(action_after != NULL) {
        vxutil_replace_run(action_after, ext_catalog);
    }

    return E_SUCCESS;
}

//-----------------------------------------------------------------------------
static int groupadd_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-groupadd options
        {.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &action_after,
         .help = "Program to run after group addition", .htyp = "cmd"},
        {.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &action_before,
         .help = "Program to run before group addition", .htyp = "cmd"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &database_name,
         .help = "Use specified database", .htyp = "name"},

        // Default options
        {.sh = 'g', .type = HXTYPE_LONG, .ptr = &group_id,
         .help = "Numerical value of the group's ID", .htyp = "gid"},
        {.sh = 'o', .type = HXTYPE_NONE, .ptr = &allow_dup,
         .help = "Allow creating a group with non-unique GID"},
        {.sh = 'r', .type = HXTYPE_NONE, .ptr = &request_sys,
         .help = "System group (use groupid < GID_MIN)"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;
    if(argv[1] == NULL) { /* Group name is mandatory */
        fprintf(stderr, "Error: Need to specify a group name\n");
        return 0;
    }

    return 1;
}

static int groupadd_read_config(void) {
    static const struct HXoption config_table[] = {
        {.ln = "GROUP_PREADD",  .type = HXTYPE_STRING, .ptr = &action_before},
        {.ln = "GROUP_POSTADD", .type = HXTYPE_STRING, .ptr = &action_after},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(CONFIG_SYSCONFDIR "/groupadd.conf", config_table);
}

//=============================================================================
