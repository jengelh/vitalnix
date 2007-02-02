/*=============================================================================
Vitalnix User Management Suite
clutils/groupmod.c - Modify a group
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
    E_OPEN,        // unable to open database or driver
    E_NOEXIST,     // group does not exist
    E_GID_USED,    // GID already used and -o was not specified
    E_NAME_USED,   // new group name (-n) already exists
    E_UPDATE,      // db->groupmod() did not return ok
    E_CLOSE,       // db->close() did not return ok
};

// Functions
static int groupmod_main2(struct vxpdb_state *);
static int groupmod_main3(struct vxpdb_state *);
static int groupmod_get_options(int *, const char ***);
static int groupmod_read_config(void);

// Variables
static int allow_dup              = 0;
static const char *new_group_name = NULL;
static long new_group_id          = PDB_NOGID;
static const char *action_before  = NULL,
                  *action_after   = NULL,
                  *database_name  = "*",
                  *group_name;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct vxpdb_state *db;
    int ret;

    if(groupmod_read_config() <= 0 || groupmod_get_options(&argc, &argv) <= 0)
        return E_OTHER;

    if(new_group_name != NULL && !vxutil_valid_username(new_group_name)) {
        fprintf(stderr, "\"%s\" is not a valid group name\n", new_group_name);
        return E_OTHER;
    }

    if((db = vxpdb_load(database_name)) == NULL) {
        fprintf(stderr, "Could not load database \"%s\": %s\n",
                database_name, strerror(errno));
        return E_OPEN;
    }

    ret = groupmod_main2(db);
    vxpdb_unload(db);
    return ret;
}

static int groupmod_main2(struct vxpdb_state *db)
{
    int ret;
    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open database: %s\n", strerror(-ret));
        return E_OPEN;
    }

    ret = groupmod_main3(db);
    vxpdb_close(db);
    return ret;
}

static int groupmod_main3(struct vxpdb_state *db)
{
    struct vxpdb_group current = {}, mod_request;
    struct HXoption ext_catalog[] = {
        {.sh = 'G', .type = HXTYPE_STRING, .ptr = &new_group_name},
        {.sh = 'N', .type = HXTYPE_STRING, .ptr = &group_name},
        {.sh = 'g', .type = HXTYPE_LONG,   .ptr = &new_group_id},
        HXOPT_TABLEEND,
    };
    int ret;

    if((ret = vxpdb_getgrnam(db, group_name, &current)) < 0) {
        fprintf(stderr, "Error querying database: %s\n", strerror(-ret));
        return E_OTHER;
    } else if(ret == 0) {
        fprintf(stderr, "Group \"%s\" does not exist\n", group_name);
        return E_NOEXIST;
    }

    if(new_group_id != PDB_NOGID && new_group_id != current.gr_gid &&
      !allow_dup && vxpdb_getgrgid(db, new_group_id, NULL) > 0)
    {
        /* If GID has changed */
        fprintf(stderr, "A group with GID %ld already exists."
                " Use -o to override.\n", new_group_id);
        return E_GID_USED;
    }

    if(new_group_name != NULL && strcmp(current.gr_name,
      current.gr_name) != 0 && vxpdb_getgrnam(db, new_group_name, NULL) > 0)
    {
        /* If name has changed */
        fprintf(stderr, "A group with that name (\"%s\") already "
                "exists.\n", new_group_name);
        return E_NAME_USED;
    }

    if(action_before != NULL)
        vxutil_replace_run(action_before, ext_catalog);

    mod_request.gr_name = static_cast(char *, new_group_name);
    mod_request.gr_gid  = new_group_id;

    if((ret = vxpdb_groupmod(db, &current, &mod_request)) <= 0) {
        fprintf(stderr, "Error: Group updating failed: %s\n", strerror(-ret));
        return E_UPDATE;
    } else if(action_after != NULL) {
        vxutil_replace_run(action_after, ext_catalog);
    }

    return E_SUCCESS;
}

//-----------------------------------------------------------------------------
static int groupmod_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-groupmod options
        {.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &action_after,
         .help = "Program to run after group modification", .htyp = "cmd"},
        {.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &action_before,
         .help = "Program to run before group modification", .htyp = "cmd"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &database_name,
         .help = "Use specified database", .htyp = "name"},

        // Default options
        {.sh = 'g', .type = HXTYPE_LONG, .ptr = &new_group_id, .htyp = "gid",
         .help = "Numerical value of the group's ID (UDB module might ignore this)"},
        {.sh = 'n', .type = HXTYPE_STRING, .ptr = &new_group_name,
         .help = "New name of the group"},
        {.sh = 'o', .type = HXTYPE_NONE, .ptr = &allow_dup,
         .help = "Allow creating a group with non-unique GID"
         " (might be disabled by the db driver)"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;
    if(argv[1] == NULL) { /* Group name is mandatory */
        fprintf(stderr, "You need to specify a group name\n");
        return 0;
    }

    return 1;
}

static int groupmod_read_config(void) {
    static const struct HXoption config_table[] = {
        {.ln = "AC_BEFORE", .type = HXTYPE_STRING, .ptr = &action_before},
        {.ln = "AC_AFTER",  .type = HXTYPE_STRING, .ptr = &action_after},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(CONFIG_SYSCONFDIR "/groupmod.conf", config_table);
}

//=============================================================================
