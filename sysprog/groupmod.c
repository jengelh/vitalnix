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
    .new_gid   = -1,
};

static int groupmod_get_options(int *, const char ***);
static int groupmod_read_config(void);

static char *Module_path = "*";

//-----------------------------------------------------------------------------
int groupmod_main(int argc, const char **argv) {
    struct adb_module *db;
    struct adb_group mmask = {}, smask = {}, result = {};
    int eax, rv = E_SUCCESS;
    struct HX_repmap sr_map[] = {
        {'G', "", &mmask.gname},
        {'g', "%ld", &mmask.gid},
        {0},
    };

    if(groupmod_read_config() <= 0 || groupmod_get_options(&argc, &argv) <= 0) {
        return E_OTHER;
    }

    // ----------------------------------------
    if((db = adb_load(Module_path, NULL)) == NULL) {
        fprintf(stderr, "Could not load ACCDB back-end module \"%s\": %s\n",
         Module_path, strerror(errno));
        return E_OPEN;
    }

    if((eax = db->open(db, ADB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open ACCDB back-end: %s"
         " (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        rv = E_OPEN;
        goto __main__close_accdb;
    }

    // ----------------------------------------
    smask.gname = Opt.gname = (char *)*argv;
    smask.gid   = -1;
    mmask.gid   = -1;

    if((eax = db->groupinfo(db, &smask, &result, 1)) < 0) {
        fprintf(stderr, "Error querying the ACCDB: %s (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        rv = E_OTHER;
        goto __main__close_backend;
    } else if(eax == 0) {
        fprintf(stderr, "Group \"%s\" does not exist\n", smask.gname);
        rv = E_NOEXIST;
        goto __main__close_backend;
    }

    // ----------------------------------------
    if(Opt.inter) {
        struct itab t[] = {
          {"Group name\n[%s] > ", 's', &Opt.new_gname, NULL},
          {"Group ID (-1 for next free)\n[%d] > ", 'l', &Opt.new_gid, NULL},
          {NULL},
        };
        memcpy(&mmask, &result, sizeof(struct adb_group));
        SH_interactive(t);
    }

    if(Opt.new_gname != NULL) { mmask.gname = Opt.new_gname; }
    if(Opt.new_gid   != -1)   { mmask.gid   = Opt.new_gid;   }

    if(mmask.gid != -1 && mmask.gid != result.gid && !Opt.dup) {
        // If GID has changed...
        struct adb_group qgrp = {gid: mmask.gid, gname: NULL};
        if(db->groupinfo(db, &qgrp, NULL, 0) > 0) {
            fprintf(stderr, "A group with GID %ld already exists."
             " Use -o to override.\n", mmask.gid);
            rv = E_GID_USED;
            goto __main__close_backend;
        }
    }

    if(mmask.gname != NULL && result.gname != NULL &&
     strcmp(mmask.gname, result.gname) != 0) {
        // ... if name changed
        struct adb_group qgrp = {gid: -1, gname: mmask.gname};
        if(db->groupinfo(db, &qgrp, NULL, 0) > 0) {
            fprintf(stderr, "A group with that name (\"%s\") already"
             " exists.\n", mmask.gname);
            rv = E_NAME_USED;
            goto __main__close_backend;
        }
    }

    // ----------------------------------------
    if(Opt.ac_before != NULL) {
        SH_runcmd(Opt.ac_before, sr_map);
    }

    if((eax = db->groupmod(db, &smask, &mmask)) <= 0) {
        fprintf(stderr, "Error: Group updating failed: %s"
         " (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        rv = E_UPDATE;
    } else if(Opt.ac_after != NULL) {
        SH_runcmd(Opt.ac_after, sr_map);
    }

    // ----------------------------------------
 __main__close_backend:
    if((eax = db->close(db)) <= 0) {
        fprintf(stderr, "Warning: DB was not cleanly closed:"
         " %s (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        if(rv == E_SUCCESS) { rv = E_CLOSE; }
    }

 __main__close_accdb:
    adb_unload(db);
    return rv;
}

//-----------------------------------------------------------------------------
static int groupmod_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-groupmod options
        {.sh = 'A', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_after,
         .help = "Program to run after group modification", .htyp = "cmd"},
        {.sh = 'B', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_before,
         .help = "Program to run before group modification", .htyp = "cmd"},
        {.sh = 'I', .type = HXOPT_NONE, .ptr = &Opt.inter,
         .help = "Interactively prompt for parameters"},
        {.sh = 'M', .type = HXOPT_STRING, .ptr = &Module_path,
         .help = "Use a different module than \"*\" (the default)",
         .htyp = "name"},

        // Default options
        {.sh = 'g', .type = HXOPT_LONG, .ptr = &Opt.new_gid,
         .help = "Numerical value of the group's ID (UDB module might ignore this)",
         .htyp = "gid"},
        {.sh = 'n', .type = HXOPT_STRING, .ptr = &Opt.new_gname,
         .help = "New name of the group"},
        {.sh = 'o', .type = HXOPT_NONE, .ptr = &Opt.dup,
         .help = "Allow creating a group with non-unique GID"
         " (might be disabled by back-end module)"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0) {
        return 0;
    }

    if(argv[1] == NULL) {
        // gname is mandatory
        fprintf(stderr, "You need to specify a group name\n");
        return 0;
    }

    return 1;
}

static int groupmod_read_config(void) {
    static const struct shconf_opt config_table[] = {
        {"AC_BEFORE", SHCONF_STRING, &Opt.ac_before},
        {"AC_AFTER",  SHCONF_STRING, &Opt.ac_after},
        {NULL},
    };
    return HX_shconfig("/etc/vitalnix/groupmod.conf", config_table);
}

//=============================================================================
