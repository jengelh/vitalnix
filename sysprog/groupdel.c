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
    E_OTHER,      // other error, see errno
    E_OPEN,       // unable to open back-end module or DB
    E_NOEXIST,    // group does not exist
    E_UPDATE,     // db->groupdel() did not return ok
    E_CLOSE,      // db->close() did not return ok
    E_PRIMARY,    // will not delete a user's primary group, unless -F is given
};

static struct {
    char *gname;
    // groupdel internal
    char *ac_after, *ac_before;
    int force;
} Opt = {};

static int groupdel_check_pri_group(struct adb_module *, struct adb_group *);
static int groupdel_get_options(int *, const char ***);
static int groupdel_read_config(void);

static char *Module_path = "*";

//-----------------------------------------------------------------------------
int groupdel_main(int argc, const char **argv) {
    struct adb_module *db;
    struct adb_group mmask = {};
    int eax, rv = E_SUCCESS;
    struct HX_repmap sr_map[] = {
        {'G', "", &mmask.gname},
        {0},
    };

    if(groupdel_read_config() <= 0 || groupdel_get_options(&argc, &argv) <= 0) {
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

    mmask.gid   = -1;
    mmask.gname = Opt.gname = (char *)*argv;

    // ----------------------------------------
    if((eax = db->groupinfo(db, &mmask, NULL, 0)) < 0) {
        fprintf(stderr, "Error querying the ACCDB: %s (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        rv = E_OTHER;
        goto __main__close_backend;
    } else if(eax == 0) {
        fprintf(stderr, "Group \"%s\" does not exist\n", mmask.gname);
        rv = E_NOEXIST;
        goto __main__close_backend;
    }

    if(!Opt.force && groupdel_check_pri_group(db, &mmask)) {
        fprintf(stderr, "Will not remove a user's primary group\n");
        rv = E_PRIMARY;
        goto __main__close_backend;
    }

    // ----------------------------------------
    if(Opt.ac_before != NULL) {
        SH_runcmd(Opt.ac_before, sr_map);
    }

    if((eax = db->groupdel(db, &mmask)) <= 0) {
        fprintf(stderr, "Error: Deleting group failed: %s"
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
static int groupdel_check_pri_group(struct adb_module *db, struct adb_group *mm) {
    struct adb_user u;
    int pg = 0; // some user has this as primary group
    void *tp;

    db->usertrav(db, &tp, NULL); // init tp
    while(db->usertrav(db, &tp, &u) > 0) {
        if((u.gid != -1 && u.gid == mm->gid) || (u.igrp != NULL &&
         mm->gname != NULL && strcmp(u.igrp, mm->gname) == 0)) {
            pg = 1;
            break;
        }
    }

    return pg;
}

static int groupdel_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-userdel options
        {.sh = 'A', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_after,
         .help = "Program to run after group deletion", .htyp = "cmd"},
        {.sh = 'B', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_before,
         .help = "Program to run before group deletion", .htyp = "cmd"},
        {.sh = 'F', .type = HXOPT_NONE, .ptr = &Opt.force,
         .help = "Force deletion of group even if users have it as"
         " primary group"},
        {.sh = 'M', .type = HXOPT_STRING, .ptr = &Module_path,
         .help = "Use a different module than \"*\" (the default)",
         .htyp = "name"},
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

static int groupdel_read_config(void) {
    static const struct shconf_opt config_table[] = {
        {"AC_BEFORE", SHCONF_STRING, &Opt.ac_before},
        {"AC_AFTER",  SHCONF_STRING, &Opt.ac_after},
        {NULL},
    };
    return HX_shconfig("/etc/vitalnix/groupdel.conf", config_table);
}

//=============================================================================
