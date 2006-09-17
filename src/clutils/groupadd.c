/*=============================================================================
Vitalnix User Management Suite
clutils/groupadd.c - Create a new group
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
#include "vitalnix-config.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"
#include "libvxutil/libvxutil.h"

enum {
    E_SUCCESS = 0,
    E_OTHER,       // other error, see errno
    E_OPEN,        // unable to open back-end module or DB
    E_GID_USED,    // GID already used and -o was not specified
    E_NAME_USED,   // group already exists
    E_UPDATE,      // db->groupadd() did not return ok
    E_CLOSE,       // db->close() did not return ok
};

static struct {
    char *gname;
    long gid;

    // groupadd internal
    char *ac_after, *ac_before;
    int dup, sys;
} Opt = {
    .gid = PDB_NOGID,
};

// Functions
static int groupadd_get_options(int *, const char ***);
static int groupadd_read_config(void);

// Variables
static char *Module_path = "*";

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct vxpdb_state *db;
    struct vxpdb_group mmask = {}, smask = {};
    int ret, rv = E_SUCCESS;
    struct HXoption ext_catalog[] = {
        {.sh = 'G', .type = HXTYPE_STRING, .ptr = &mmask.gr_name},
        {.sh = 'g', .type = HXTYPE_LONG,   .ptr = &mmask.gr_gid},
        HXOPT_TABLEEND,
    };

    if(groupadd_read_config() <= 0 || groupadd_get_options(&argc, &argv) <= 0)
        return E_OTHER;

    // ----------------------------------------
    if((db = vxpdb_load(Module_path)) == NULL) {
        fprintf(stderr, "Could not load PDB back-end module \"%s\": %s\n",
                Module_path, strerror(errno));
        return E_OPEN;
    }

    Opt.gname = argv[1];

    // ----------------------------------------
    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open PDB back-end: %s\n", strerror(-ret));
        rv = E_OPEN;
        goto __main__close_pdb;
    }

    smask.gr_name = mmask.gr_name = Opt.gname;
    smask.gr_gid  = mmask.gr_gid  = PDB_NOGID;

    // ----------------------------------------
    if((ret = vxpdb_groupinfo(db, &smask, NULL, 0)) < 0) {
        fprintf(stderr, "Error querying the PDB: %s\n", strerror(-ret));
        rv = E_OTHER;
        goto __main__close_backend;
    } else if(ret > 0) {
        fprintf(stderr, "Group \"%s\" already exists\n", smask.gr_name);
        rv = E_NAME_USED;
        goto __main__close_backend;
    }

    // ----------------------------------------
    if(Opt.gid != PDB_NOGID) { // -g is provided
        smask.gr_name = NULL;
        smask.gr_gid  = Opt.gid;
        if(!Opt.dup && vxpdb_groupinfo(db, &smask, NULL, 0) > 0) {
            /* The -o flag (allow creating group with duplicate GID)
            was not passed. */
            fprintf(stderr, "Group with GID %ld already exists."
                    " Use -o to override.\n", smask.gr_gid);
            rv = E_GID_USED;
            goto __main__close_backend;
        }
        mmask.gr_gid = Opt.gid;
    } else if(Opt.sys) { // -r flag passed
        mmask.gr_gid = vxpdb_modctl(db, PDB_NEXTGID_SYS, db);
    }

    // ----------------------------------------
    if(Opt.ac_before != NULL)
        vxutil_replace_run(Opt.ac_before, ext_catalog);

    if((ret = vxpdb_groupadd(db, &mmask)) <= 0) {
        fprintf(stderr, "Warning: Group addition failed: %s\n", strerror(-ret));
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
static int groupadd_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-groupadd options
        {.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_after,
         .help = "Program to run after group addition", .htyp = "cmd"},
        {.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_before,
         .help = "Program to run before group addition", .htyp = "cmd"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &Module_path,
         .help = "Use a different module than \"*\" (the default)",
         .htyp = "name"},

        // Default options
        {.sh = 'g', .type = HXTYPE_LONG, .ptr = &Opt.gid,
         .help = "Numerical value of the group's ID", .htyp = "gid"},
        {.sh = 'o', .type = HXTYPE_NONE, .ptr = &Opt.dup,
         .help = "Allow creating a group with non-unique GID"},
        {.sh = 'r', .type = HXTYPE_NONE, .ptr = &Opt.sys,
         .help = "System group (use groupid < GID_MIN)"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;
    if(argv[1] == NULL) {
        // Group name is mandatory
        fprintf(stderr, "Error: Need to specify a group name\n");
        return 0;
    }

    return 1;
}

static int groupadd_read_config(void) {
    static const struct HXoption config_table[] = {
        {.ln = "GROUP_PREADD",  .type = HXTYPE_STRING, .ptr = &Opt.ac_before},
        {.ln = "GROUP_POSTADD", .type = HXTYPE_STRING, .ptr = &Opt.ac_after},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(CONFIG_ETC_VITALNIX "/groupadd.conf", config_table);
}

//=============================================================================
