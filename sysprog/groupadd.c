/*=============================================================================
Vitalnix User Management Suite
  Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2003 - 2005
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

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
#include "include/accdb.h"
#include "sysprog/shared.h"

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
    int dup, inter, sys;
} Opt = {
    .gid       = -1,
};

static int groupadd_get_options(int *, const char ***);
static int groupadd_read_config(void);

static char *Module_path = "*";

//-----------------------------------------------------------------------------
int groupadd_main(int argc, const char **argv) {
    struct adb_module *db;
    struct adb_group mmask = {}, smask = {};
    int eax, rv = E_SUCCESS;
    struct HX_repmap sr_map[] = {
        {'G', "", &mmask.gname},
        {'g', "%ld", &mmask.gid},
        {0},
    };

    if(groupadd_read_config() <= 0 || groupadd_get_options(&argc, &argv) <= 0) {
        return E_OTHER;
    }

    // ----------------------------------------
    if((db = adb_load(Module_path, NULL)) == NULL) {
        fprintf(stderr, "Could not load ACCDB back-end module \"%s\": %s\n",
         Module_path, strerror(errno));
        return E_OPEN;
    }

    Opt.gname = (char *)*argv;
    if(Opt.inter) {
        struct itab t[] = {
            {"Group name\n[%s] > ", 's', &Opt.gname, NULL},
            {"Group ID (-1 for next free)\n[%d] > ", 'l', &Opt.gid, NULL},
            {NULL},
        };
        SH_interactive(t);
    }

    // ----------------------------------------
    if((eax = db->open(db, ADB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open ACCDB back-end: %s"
         " (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        rv = E_OPEN;
        goto __main__close_accdb;
    }

    smask.gname = mmask.gname = Opt.gname;
    smask.gid   = mmask.gid   = -1;

    // ----------------------------------------
    if((eax = db->groupinfo(db, &smask, NULL, 0)) < 0) {
        fprintf(stderr, "Error querying the ACCDB: %s (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        rv = E_OTHER;
        goto __main__close_backend;
    } else if(eax > 0) {
        fprintf(stderr, "Group \"%s\" already exists\n", smask.gname);
        rv = E_NAME_USED;
        goto __main__close_backend;
    }

    // ----------------------------------------
    if(Opt.gid != -1) { // -g is provided
        smask.gname = NULL;
        smask.gid = Opt.gid;
        if(!Opt.dup && db->groupinfo(db, &smask, NULL, 0) > 0) {
            /* The -o flag (allow creating group with duplicate GID)
            was not passed. */
            fprintf(stderr, "Group with GID %ld already exists."
             " Use -o to override.\n", smask.gid);
            rv = E_GID_USED;
            goto __main__close_backend;
        }
        mmask.gid = Opt.gid;
    } else if(Opt.sys) { // -r flag passed
        mmask.gid = db->modctl(db, ADB_NEXTGID_SYS, db);
    }

    // ----------------------------------------
    if(Opt.ac_before != NULL) {
        SH_runcmd(Opt.ac_before, sr_map);
    }

    if((eax = db->groupadd(db, &mmask)) <= 0) {
        fprintf(stderr, "Warning: Group addition failed: %s"
         " (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        rv = E_UPDATE;
    } else if(Opt.ac_after != NULL) {
        SH_runcmd(Opt.ac_after, sr_map);
    }

    // ----------------------------------------
 __main__close_backend:
    if((eax = db->close(db)) <= 0) {
        fprintf(stderr, "Warning: DB was not cleanly closed: %s"
         " (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        // if there was some other error, keep that instead
        if(rv == E_SUCCESS) { rv = E_CLOSE; }
    }

 __main__close_accdb:
    adb_unload(db);
    return rv;
}

//-----------------------------------------------------------------------------
static int groupadd_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-groupadd options
        {.sh = 'A', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_after,
         .help = "Program to run after group addition", .htyp = "cmd"},
        {.sh = 'B', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_before,
         .help = "Program to run before group addition", .htyp = "cmd"},
        {.sh = 'I', .type = HXOPT_NONE, .ptr = &Opt.inter,
         .help = "Interactively prompt for parameters"},
        {.sh = 'M', .type = HXOPT_STRING, .ptr = &Module_path,
         .help = "Use a different module than \"*\" (the default)",
         .htyp = "name"},

        // Default options
        {.sh = 'g', .type = HXOPT_LONG, .ptr = &Opt.gid,
         .help = "Numerical value of the group's ID", .htyp = "gid"},
        {.sh = 'o', .type = HXOPT_NONE, .ptr = &Opt.dup,
         .help = "Allow creating a group with non-unique GID"},
        {.sh = 'r', .type = HXOPT_NONE, .ptr = &Opt.sys,
         .help = "System group (use groupid < GID_MIN)"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0) {
        return 0;
    }

    if(!Opt.inter && argv[1] == NULL) {
        // gname is mandatory
        fprintf(stderr, "Error: Need to specify a group name\n");
        return 0;
    }

    return 1;
}

static int groupadd_read_config(void) {
    static const struct shconf_opt config_table[] = {
        {"AC_BEFORE", SHCONF_STRING, &Opt.ac_before},
        {"AC_AFTER",  SHCONF_STRING, &Opt.ac_after},
        {NULL},
    };
    return HX_shconfig("/etc/vitalnix/groupadd.conf", config_table);
}

//=============================================================================
