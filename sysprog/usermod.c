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
    E_UID_USED,    // UID already used and -o was not specified
    E_NAME_USED,   // new user name (-n) already exists
    E_UPDATE,      // db->usermod() did not return ok
    E_CLOSE,       // db->close() did not return ok
};

enum {
    AOP_LOCK = 1,
    AOP_UNLOCK,
};

static struct {
    char *lname, *new_lname;
    long new_uid;
    char *new_igrp, *new_sgrp, *new_gecos, *new_homedir, *new_shell,
     *new_passwd;
    long new_expire, new_inactive;

    // usermod internal
    char *ac_after, *ac_before;
    int dup, inter, lock, mov;
} Opt = {
    .new_uid      = -1,
    .new_expire   = -1,
    .new_inactive = -1,
};

static int usermod_check_login(const char *);
static int usermod_get_options(int *, const char ***);
static void usermod_getopt_expire(const struct HXoptcb *);
static int usermod_pconfig_lname(const char *, char, void *);
static int usermod_read_config(void);

static char *Module_path = "*";

//-----------------------------------------------------------------------------
int usermod_main(int argc, const char **argv) {
    struct adb_module *db;
    struct adb_user mmask = {}, smask = {}, result = {};
    int eax, rv = E_SUCCESS;
    struct HX_repmap sr_map[] = {
        {'l', "", &mmask.lname},
        {'u', "%ld", &mmask.uid},
        {0},
    };

    mmask.uid         = mmask.gid = -1;
    mmask.last_change = \
    mmask.keep_min    = \
    mmask.keep_max    = \
    mmask.warn_age    = \
    mmask.expire      = \
    mmask.inactive    = -1;

    if(usermod_read_config() <= 0 || usermod_get_options(&argc, &argv) <= 0) {
        return E_OTHER;
    }

    if((db = adb_load(Module_path, NULL)) == NULL) {
        fprintf(stderr, "Could not load ACCDB back-end module \"%s\": %s\n",
         Module_path, strerror(errno));
        return E_OPEN;
    }

    if((eax = db->open(db, ADB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open ACCDB back-end: %s"
         " (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        rv = E_OPEN;
        goto __main__close_accdb;
    }

    // ----------------------------------------
    smask.lname = Opt.lname = (char *)*argv;
    smask.uid = smask.gid = -1;

    if((eax = db->userinfo(db, &smask, &result, 1)) < 0) {
        fprintf(stderr, "Error querying the ACCDB: %s (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        rv = E_OTHER;
        goto __main__close_backend;
    } else if(eax == 0) {
        fprintf(stderr, "User \"%s\" does not exist\n", smask.lname);
        rv = E_NOEXIST;
        goto __main__close_backend;
    }

    // ----------------------------------------
    if(Opt.inter) {
        struct itab t[] = {
            {"Login name\n[%s] > ", 's', &mmask.lname, usermod_pconfig_lname},
            {"User ID (-1 for next free)\n[%d] > ", 'l', &mmask.uid, NULL},
            {"Initial group (or GID)\n[%s] > ", 's', &Opt.new_igrp, NULL},
            {"Supplemental groups (and/or GIDs), seperated by comma\n[%s] > ",
             's', &Opt.new_sgrp, NULL},
            {"Comment field (real name, etc.)\n[%s] > ",
             's', &mmask.gecos, NULL},
            {"Home directory\n[%s] > ", 's', &mmask.home, NULL},
            {"Shell\n[%s] > ", 's', &mmask.shell, NULL},
            {NULL},
        };
        memcpy(&mmask, &result, sizeof(struct adb_user));
        if(Opt.new_igrp == NULL) {
            char buf[16];
            snprintf(buf, 16, "%ld", result.gid);
            Opt.new_igrp = (result.igrp != NULL) ?
             result.igrp : HX_strdup(buf);
        }
        SH_interactive(t);
    }

    if(Opt.new_igrp != NULL) {
        if(SH_only_digits(Opt.new_igrp)) {
            mmask.gid  = strtoul(Opt.new_igrp, NULL, 0);
            mmask.igrp = NULL;
        } else {
            mmask.gid  = -1;
            mmask.igrp = Opt.new_igrp;
        }
    }

    if(Opt.new_lname    != NULL) { mmask.lname    = Opt.new_lname;    }
    if(Opt.new_uid      != -1)   { mmask.uid      = Opt.new_uid;      }
    if(Opt.new_gecos    != NULL) { mmask.gecos    = Opt.new_gecos;    }
    if(Opt.new_homedir  != NULL) { mmask.home     = Opt.new_homedir;  }
    if(Opt.new_shell    != NULL) { mmask.shell    = Opt.new_shell;    }
    if(Opt.new_passwd   != NULL) { mmask.passwd   = Opt.new_passwd;   }
    if(Opt.new_expire   != -1)   { mmask.expire   = Opt.new_expire;   }
    if(Opt.new_inactive != -1)   { mmask.inactive = Opt.new_inactive; }

    if(mmask.uid != -1 && mmask.uid != result.uid && !Opt.dup) {
        // If UID has changed...
        struct adb_user quser = {.uid = mmask.uid, .gid = -1};
        if(db->userinfo(db, &quser, NULL, 0) > 0) {
            fprintf(stderr, "A user with UID %ld already exists."
             " Use -o to override.\n", mmask.uid);
            rv = E_UID_USED;
            goto __main__close_backend;
        }
    }

    if(mmask.lname != NULL && result.lname != NULL &&
     strcmp(mmask.lname, result.lname) != 0) {
        // ... if name changed
        struct adb_user quser = {.lname = mmask.lname, .uid = -1, .gid = -1};
        if(db->userinfo(db, &quser, NULL, 0) > 0) {
            fprintf(stderr, "A user with that name (\"%s\") already"
             " exists.\n", mmask.lname);
            rv = E_NAME_USED;
            goto __main__close_backend;
        }
    }

    // ----------------------------------------
    if(Opt.ac_before != NULL) {
        SH_runcmd(Opt.ac_before, sr_map);
    }

    if((eax = db->usermod(db, &smask, &mmask)) <= 0) {
        fprintf(stderr, "Error: User updating failed: %s (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        rv = E_UPDATE;
    } else if(Opt.ac_after != NULL) {
        SH_runcmd(Opt.ac_after, sr_map);
    }

    // ----------------------------------------
 __main__close_backend:
    if((eax = db->close(db)) <= 0) {
        fprintf(stderr, "Warning: DB was not cleanly closed:"
         " %s (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        // if there was some other error, keep that instead
        if(rv == E_SUCCESS) { rv = E_CLOSE; }
    }

 __main__close_accdb:
    adb_unload(db);
    return rv;
}

//-----------------------------------------------------------------------------
static int usermod_check_login(const char *ln) {
    if(*ln == '\0') {
        fprintf(stderr, "You need to provide a username\n");
        return 0;
    }
    return 1;
}

static int usermod_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-usermod options
        {.sh = 'A', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_after,
         .help = "Program to run after user modification", .htyp = "cmd"},
        {.sh = 'B', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_before,
         .help = "Program to run before user modification", .htyp = "cmd"},
        {.sh = 'I', .type = HXOPT_NONE, .ptr = &Opt.inter,
         .help = "Interactively prompt for parameters"},
        {.sh = 'M', .type = HXOPT_STRING, .ptr = &Module_path,
         .help = "Use a different module than \"*\" (the default)",
         .htyp = "name"},

        // Default options
        {.sh = 'G', .type = HXOPT_STRING, .ptr = &Opt.new_sgrp,
         .help = "The new supplementary groups for the user", .htyp = "groups"},
        {.sh = 'L', .type = HXOPT_VAL, .ptr = &Opt.lock, .val = AOP_LOCK,
         .help = "Lock user account"},
        {.sh = 'U', .type = HXOPT_VAL, .ptr = &Opt.lock, .val = AOP_UNLOCK,
         .help = "Unlock user account"},
        {.sh = 'c', .type = HXOPT_STRING, .ptr = &Opt.new_gecos,
         .help = "New comment for the user", .htyp = "comment"},
        {.sh = 'd', .type = HXOPT_STRING, .ptr = &Opt.new_homedir,
         .help = "New home directory (home automatically moved)", .htyp = "dir"},
        {.sh = 'e', .type = HXOPT_STRING, .cb = usermod_getopt_expire,
         .help = "Date when the account expires", .htyp = "date"},
        {.sh = 'f', .type = HXOPT_INT, .ptr = &Opt.new_inactive,
         .help = "Days until account becomes inactive", .htyp = "days"},
        {.sh = 'g', .type = HXOPT_LONG, .ptr = &Opt.new_igrp,
         .help = "The new primary group for the user", .htyp = "group"},
        {.sh = 'l', .type = HXOPT_STRING, .ptr = &Opt.new_lname,
         .help = "New login name of the user"},
        {.sh = 'm', .type = HXOPT_NONE, .ptr = &Opt.mov,
         .help = "Move home directory if -d is given"},
        {.sh = 'o', .type = HXOPT_NONE, .ptr = &Opt.dup,
         .help = "Allow creating a user with non-unique UID"},
        {.sh = 'p', .type = HXOPT_STRING, .ptr = &Opt.new_passwd,
         .help = "New password to use (encrypted form)"},
        {.sh = 's', .type = HXOPT_STRING, .ptr = &Opt.new_shell,
         .help = "New default shell for user", .htyp = "file"},
        {.sh = 'u', .type = HXOPT_LONG, .ptr = &Opt.new_uid,
         .help = "New numerical value of the user's ID", .htyp = "uid"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0) {
        return 0;
    }

    if(argv[1] == NULL) {
        // loginname is mandatory
        fprintf(stderr, "You need to specify a username\n");
        return 0;
    }

    return 1;
}

static void usermod_getopt_expire(const struct HXoptcb *cbi) {
    Opt.new_expire = SH_string_to_iday(cbi->s);
    return;
}

static int usermod_pconfig_lname(const char *key, char type, void *ptr) {
    char **lnamep = ptr;
    if(!usermod_check_login(*lnamep)) { return 0; }
//    if(Opt.new_homedir == NULL) { usermod_generate_homedir(*lnamep); }
    return 1;
}

static int usermod_read_config(void) {
    static const struct shconf_opt config_table[] = {
        {"AC_BEFORE", SHCONF_STRING, &Opt.ac_before},
        {"AC_AFTER",  SHCONF_STRING, &Opt.ac_after},
        {NULL},
    };
    return HX_shconfig("/etc/vitalnix/usermod.conf", config_table);
}

//=============================================================================
