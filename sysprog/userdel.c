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
    E_NOEXIST,     // user does not exist
    E_DENY,        // will not remove 'root' or UID 0 without -F (force)
    E_UPDATE,      // db->userdel() did not return ok
    E_CLOSE,       // db->close() did not return ok
};

static struct {
    char *lname;
    // userdel internal
    char *ac_after, *ac_before;
    int force, rm_home;
} Opt = {};

static int userdel_count_oc(const char *, char);
static int userdel_get_options(int *, const char ***);
static int userdel_read_config(void);

static char *Module_path = "*";

//-----------------------------------------------------------------------------
int userdel_main(int argc, const char **argv) {
    struct adb_module *db;
    struct adb_user result = {}, mmask = {};
    int eax, rv = E_SUCCESS;
    char *homedir = NULL;
    struct HX_repmap sr_map[] = {
        {'l', "", &mmask.lname},
        {0},
    };

    if(userdel_read_config() <= 0 || userdel_get_options(&argc, &argv) <= 0) {
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

    mmask.uid   = mmask.gid = -1;
    mmask.lname = Opt.lname = (char *)*argv;

    // ----------------------------------------
    if((eax = db->userinfo(db, &mmask, &result, 1)) < 0) {
        fprintf(stderr, "Error querying the ACCDB: %s (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        rv = E_OTHER;
        goto __main__close_backend;
    } else if(eax == 0) {
        fprintf(stderr, "User \"%s\" does not exist\n", mmask.lname);
        rv = E_NOEXIST;
        goto __main__close_backend;
    }

    if(!Opt.force && (strcmp(result.lname, "root") == 0 || result.uid == 0)) {
        fprintf(stderr, "Will not remove user \"root\" or user with UID 0,"
         " unless -F is given.");
        rv = E_DENY;
        goto __main__close_backend;
    }

    // ----------------------------------------
    if(Opt.ac_before != NULL) {
        SH_runcmd(Opt.ac_before, sr_map);
    }

    if(Opt.rm_home) {
        char buf[MAXFNLEN];
        snprintf(buf, MAXFNLEN, "/var/spool/mail/%s", mmask.lname);
        unlink(buf);
        homedir = HX_strdup(result.home);
    }

    if((eax = db->userdel(db, &mmask)) <= 0) {
        fprintf(stderr, "Error: Deleting user failed: %s (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        rv = E_UPDATE;
    } else if(Opt.ac_after != NULL) {
        SH_runcmd(Opt.ac_after, sr_map);
    }

    if(Opt.rm_home) {
        if(homedir == NULL || *homedir == '\0') {
            fprintf(stderr, "Warning: User had no home directory. Not removing"
             " anything.\n");
        } else if(strcmp(homedir, "/") == 0) {
            fprintf(stderr, "Warning: Will refuse to delete home directory"
             " \"/\"\n");
        } else if(userdel_count_oc(homedir, '/') <= 1 && !Opt.force) {
            fprintf(stderr, "Warning: Will not remove home directory \"%s\""
             " which has less than two slashes and thus looks like a system"
             " or a malformed directory. (Remove it manually.)\n", homedir);
        } else {
            HX_rrmdir(homedir);
        }

        free(homedir);
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
static int userdel_count_oc(const char *fn, char s) {
    const char *ptr = fn;
    int n = 0;
    if(fn == NULL) { return 0; }

    while((ptr = strchr(ptr, '/')) != NULL) {
        ++n;
        ++ptr;
    }

    return n;
}

static int userdel_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, accdb-userdel options
        {.sh = 'A', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_after,
         .help = "Program to run after user modification", .htyp = "cmd"},
        {.sh = 'B', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_before,
         .help = "Program to run before user modification", .htyp = "cmd"},
        {.sh = 'F', .type = HXOPT_NONE, .ptr = &Opt.force,
         .help = "Force deletion even if UID is 0 or name is 'root'"},
        {.sh = 'M', .type = HXOPT_STRING, .ptr = &Module_path,
         .help = "Use a different module than \"*\" (the default)", .htyp = "name"},

        // Default options
        {.sh = 'r', .type = HXOPT_NONE, .ptr = &Opt.rm_home,
         .help = "Remove home directory and mail spool"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0) {
        return 0;
    }

    if(argv[1] == NULL) {
        fprintf(stderr, "You need to specify a username\n");
        return 0;
    }

    return 1;
}

static int userdel_read_config(void) {
    static const struct shconf_opt config_table[] = {
        {"AC_BEFORE", SHCONF_STRING, &Opt.ac_before},
        {"AC_AFTER",  SHCONF_STRING, &Opt.ac_after},
        {NULL},
    };
    return HX_shconfig("/etc/vitalnix/userdel.conf", config_table);
}

//=============================================================================
