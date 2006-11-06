/*=============================================================================
Vitalnix User Management Suite
clutils/groupmod.c - Modify a group
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/config.h>
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"
#include "libvxutil/libvxutil.h"

enum {
    E_SUCCESS = 0,
    E_OTHER,       // other error, see errno
    E_OPEN,        // unable to open back-end module or DB
    E_NOEXIST,     // group does not exist
    E_GID_USED,    // GID already used and -o was not specified
    E_NAME_USED,   // new group name (-n) already exists
    E_UPDATE,      // db->groupmod() did not return ok
    E_CLOSE,       // db->close() did not return ok
};

static struct {
    char *gname, *new_gname;
    long new_gid;

    // groupmod internal
    char *ac_after, *ac_before;
    int dup, inter;
} Opt = {
    .new_gid   = PDB_NOGID,
};

// Functions
static int groupmod_get_options(int *, const char ***);
static int groupmod_read_config(void);

// Variables
static char *Module_path = "*";

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct vxpdb_state *db;
    struct vxpdb_group mmask = {}, smask = {}, result = {};
    int ret, rv = E_SUCCESS;
    struct HXoption ext_catalog[] = {
        {.sh = 'G', .type = HXTYPE_STRING, .ptr = &mmask.gr_name},
        {.sh = 'g', .type = HXTYPE_LONG,   .ptr = &mmask.gr_gid},
        HXOPT_TABLEEND,
    };

    if(groupmod_read_config() <= 0 || groupmod_get_options(&argc, &argv) <= 0)
        return E_OTHER;

    // ----------------------------------------
    if((db = vxpdb_load(Module_path)) == NULL) {
        fprintf(stderr, "Could not load PDB back-end module \"%s\": %s\n",
                Module_path, strerror(errno));
        return E_OPEN;
    }

    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open PDB back-end: %s\n", strerror(-ret));
        rv = E_OPEN;
        goto __main__close_pdb;
    }

    // ----------------------------------------
    smask.gr_name = Opt.gname = argv[1];
    smask.gr_gid  = PDB_NOGID;
    mmask.gr_gid  = PDB_NOGID;

    if((ret = vxpdb_groupinfo(db, &smask, &result, 1)) < 0) {
        fprintf(stderr, "Error querying the PDB: %s\n", strerror(-ret));
        rv = E_OTHER;
        goto __main__close_backend;
    } else if(ret == 0) {
        fprintf(stderr, "Group \"%s\" does not exist\n", smask.gr_name);
        rv = E_NOEXIST;
        goto __main__close_backend;
    }

    // ----------------------------------------
    if(mmask.gr_gid != PDB_NOGID && mmask.gr_gid != result.gr_gid && !Opt.dup) {
        // If GID has changed...
        struct vxpdb_group qgrp = {.gr_gid = mmask.gr_gid, .gr_name = NULL};
        if(vxpdb_groupinfo(db, &qgrp, NULL, 0) > 0) {
            fprintf(stderr, "A group with GID %ld already exists."
             " Use -o to override.\n", mmask.gr_gid);
            rv = E_GID_USED;
            goto __main__close_backend;
        }
    }

    if(mmask.gr_name != NULL && result.gr_name != NULL &&
     strcmp(mmask.gr_name, result.gr_name) != 0) {
        // ... if name changed
        struct vxpdb_group qgrp = {
            .gr_gid  = PDB_NOGID,
            .gr_name = mmask.gr_name,
        };
        if(vxpdb_groupinfo(db, &qgrp, NULL, 0) > 0) {
            fprintf(stderr, "A group with that name (\"%s\") already "
                    "exists.\n", mmask.gr_name);
            rv = E_NAME_USED;
            goto __main__close_backend;
        }
    }

    // ----------------------------------------
    if(Opt.ac_before != NULL)
        vxutil_replace_run(Opt.ac_before, ext_catalog);

    if((ret = vxpdb_groupmod(db, &smask, &mmask)) <= 0) {
        fprintf(stderr, "Error: Group updating failed: %s\n", strerror(-ret));
        rv = E_UPDATE;
    } else if(Opt.ac_after != NULL) {
        vxutil_replace_run(Opt.ac_after, ext_catalog);
    }

    // ----------------------------------------
 __main__close_backend:
    vxpdb_close(db);
 __main__close_pdb:
    vxpdb_unload(db);
    return rv;
}

//-----------------------------------------------------------------------------
static int groupmod_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-groupmod options
        {.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_after,
         .help = "Program to run after group modification", .htyp = "cmd"},
        {.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_before,
         .help = "Program to run before group modification", .htyp = "cmd"},
        {.sh = 'I', .type = HXTYPE_NONE, .ptr = &Opt.inter,
         .help = "Interactively prompt for parameters"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &Module_path,
         .help = "Use a different module than \"*\" (the default)",
         .htyp = "name"},

        // Default options
        {.sh = 'g', .type = HXTYPE_LONG, .ptr = &Opt.new_gid,
         .help = "Numerical value of the group's ID (UDB module might ignore this)",
         .htyp = "gid"},
        {.sh = 'n', .type = HXTYPE_STRING, .ptr = &Opt.new_gname,
         .help = "New name of the group"},
        {.sh = 'o', .type = HXTYPE_NONE, .ptr = &Opt.dup,
         .help = "Allow creating a group with non-unique GID"
         " (might be disabled by back-end module)"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;

    if(argv[1] == NULL) {
        // Group name is mandatory
        fprintf(stderr, "You need to specify a group name\n");
        return 0;
    }

    return 1;
}

static int groupmod_read_config(void) {
    static const struct HXoption config_table[] = {
        {.ln = "AC_BEFORE", .type = HXTYPE_STRING, .ptr = &Opt.ac_before},
        {.ln = "AC_AFTER",  .type = HXTYPE_STRING, .ptr = &Opt.ac_after},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(CONFIG_SYSCONFDIR "/groupmod.conf", config_table);
}

//=============================================================================
