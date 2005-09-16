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
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "include/accdb.h"
#include "sysprog/shared.h"

enum {
    E_SUCCESS = 0,
    E_OTHER,     // other error, see errno
    E_OPEN,      // unable to open back-end module or DB
    E_UID_USED,  // UID already used and -o was not specified
    E_NAME_USED, // user already exists
    E_UPDATE,    // db->useradd() did not return ok
    E_CREAT,     // post-stuff failed (home directory, skeleton copying)
    E_CLOSE,     // db->close() did not return ok
};

static struct {
    char *lname;
    long uid;
    char *igrp, *sgrp, *gecos, *homedir, *shell;

    char *passwd;
    long expire, inactive;

    // useradd internal
    char *ac_after, *ac_before, *skeldir;
    int creat, dup, force, inter, split_lvl, sys;
} Opt = {
    .uid        = -1,
    .gecos      = "(null)",
    .shell      = "/bin/bash",
    .passwd     = "!",
    .expire     = ADB_EXPIRE_NEVER,
    .inactive   = ADB_INACTIVE_NEVER,
    .skeldir    = "/etc/skel",
};

static struct {
    // Defaults
    char *homebase;       // default homebase
    long keep_min, keep_max, warn_age;
    unsigned long umask;
} Conf = {
    .homebase = "/home",
    .keep_min = 0,
    .keep_max = 366,
    .warn_age = 21,
    .umask    = 0066,
};

static void useradd_generate_homedir(void);
static int useradd_get_options(int *, const char ***);
static void useradd_getopt_expire(const struct HXoptcb *);
static int useradd_interactive_lname(const char *, char, void *);
static int useradd_read_config(void);
static int useradd_pconfig_umask(const struct shconf_opt *, const char *);

static char *Module_path = "*";

//-----------------------------------------------------------------------------
int useradd_main(int argc, const char **argv) {
    struct adb_module *db;
    struct adb_user mmask = {}, smask = {};
    int eax, rv = E_SUCCESS;
    struct HX_repmap sr_map[] = {
        {'l', "", &mmask.lname},
        {'u', "%ld", &mmask.uid},
        {0},
    };

    if(useradd_read_config() <= 0 || useradd_get_options(&argc, &argv) <= 0) {
        return E_OTHER;
    }
    Opt.lname = (char *)*argv;

    // ----------------------------------------
    if((db = adb_load(Module_path, NULL)) == NULL) {
        fprintf(stderr, "Could not load ACCDB back-end module \"%s\": %s\n",
         Module_path, strerror(errno));
        return E_OPEN;
    }

    if(Opt.inter) {
        struct itab t[] = {
            {"Login name\n[%s] > ", 's', &Opt.lname, useradd_interactive_lname},
            {"User ID (-1 for next free)\n[%d] > ", 'l', &Opt.uid, NULL},
            {"Initial group (or GID)\n[%s] > ", 's', &Opt.igrp, NULL},
            {"Supplemental groups (and/or GIDs), seperated by comma\n[%s] > ",
             's', &Opt.sgrp, NULL},
            {"Comment field (real name, etc.)\n[%s] > ",
             's', &Opt.gecos, NULL},
            {"Home directory\n[%s] > ", 's', &Opt.homedir, NULL},
            {"Shell\n[%s] > ", 's', &Opt.shell, NULL},
            {"Skeleton dir\n[%s] > ", 's', &Opt.skeldir, NULL},
            {NULL},
        };
        SH_interactive(t);
    } else if(Opt.homedir == NULL) {
        useradd_generate_homedir();
    }

    mmask.lname    = Opt.lname;
    mmask.uid      = Opt.uid;
    mmask.gecos    = Opt.gecos;
    mmask.home     = Opt.homedir;
    mmask.shell    = Opt.shell;

    mmask.passwd      = Opt.passwd;
    mmask.last_change = 0;
    mmask.keep_min    = Conf.keep_min;
    mmask.keep_max    = Conf.keep_max;
    mmask.warn_age    = Conf.warn_age;
    mmask.expire      = Opt.expire;
    mmask.inactive    = Opt.inactive;

    // ----------------------------------------
    if((eax = db->open(db, ADB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open ACCDB back-end: %s"
         " (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        rv = E_OPEN;
        goto __main__close_accdb;
    }

    smask.lname = Opt.lname;
    smask.uid   = smask.gid = -1;

    if((eax = db->userinfo(db, &smask, NULL, 0)) < 0) {
        fprintf(stderr, "Error querying the ACCDB: %s (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        rv = E_OTHER;
        goto __main__close_backend;
    } else if(eax > 0) {
        fprintf(stderr, "User \"%s\" already exists\n", smask.lname);
        rv = E_NAME_USED;
        goto __main__close_backend;
    }

    // ----------------------------------------
    if(Opt.uid != -1) { // -u is provided
        smask.lname = NULL;
        smask.uid   = Opt.uid;
        smask.gid   = -1;
        if(!Opt.dup && db->userinfo(db, &smask, NULL, 0) > 0) {
            /* The -o flag (allow creating user with duplicate UID)
            was not passed. */
            fprintf(stderr, "User with GID %ld already exists."
             " Use -o to override.\n", smask.gid);
            rv = E_UID_USED;
            goto __main__close_backend;
        }
        mmask.uid = Opt.uid;
    } else if(Opt.sys) { // -r flag passed
        mmask.uid = db->modctl(db, ADB_NEXTUID_SYS, db);
    }

    if(SH_only_digits(Opt.igrp)) {
        mmask.gid  = strtoul(Opt.igrp, NULL, 0);
        mmask.igrp = NULL;
    } else {
        mmask.gid  = -1;
        mmask.igrp = Opt.igrp;
    }

    // ----------------------------------------
    if(Opt.ac_before != NULL) {
        SH_runcmd(Opt.ac_before, sr_map);
    }

    if((eax = db->useradd(db, &mmask)) <= 0) {
        fprintf(stderr, "Warning: User addition failed: %s"
         " (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        rv = E_UPDATE;
        goto __main__close_backend;
    }

    if(Opt.creat) {
        if(HX_mkdir(Opt.homedir) <= 0) {
            fprintf(stderr, "Warning: Could not create home directory %s\n",
             strerror(errno));
            rv = E_CREAT;
            goto __main__close_backend;
        }

        lchown(Opt.homedir, mmask.uid, mmask.gid);
        chmod(Opt.homedir, 0755 & ~Conf.umask);

        if(Opt.skeldir != NULL && *Opt.skeldir != '\0' &&
         *Opt.skeldir != ' ') {
            HX_copy_dir(Opt.skeldir, Opt.homedir, HXF_UID | HXF_GID | HXF_KEEP,
             mmask.uid, mmask.gid);
        }
    }

    if(eax > 0 && Opt.ac_after != NULL) {
        SH_runcmd(Opt.ac_after, sr_map);
    }

    // ----------------------------------------
 __main__close_backend:
    if((eax = db->close(db)) <= 0) {
        fprintf(stderr, "Warning: DB was not cleanly closed: %s"
         " (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        if(rv == E_SUCCESS) { rv = E_CLOSE; }
    }

 __main__close_accdb:
    adb_unload(db);
    return rv;
}

//-----------------------------------------------------------------------------
static void useradd_generate_homedir(void) {
    char buf[MAXFNLEN];
    if(Opt.lname == NULL || Conf.homebase == NULL) { return; }

    if(Opt.split_lvl == 1 || strlen(Opt.lname) == 1) {
        snprintf(buf, MAXFNLEN, "%s/%c/%s", Conf.homebase,
         *Opt.lname, Opt.lname);
    } else if(Opt.split_lvl == 2) {
        snprintf(buf, MAXFNLEN, "%s/%c/%c%c/%s", Conf.homebase,
         *Opt.lname, Opt.lname[0], Opt.lname[1], Opt.lname);
    } else {
        snprintf(buf, MAXFNLEN, "%s/%s", Conf.homebase, Opt.lname);
    }

    Opt.homedir = HX_strdup(buf);
    return;
}

static int useradd_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-useradd options
        {.sh = 'A', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_after,
         .help = "Program to run after user addition", .htyp = "cmd"},
        {.sh = 'B', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_before,
         .help = "Program to run before user addition", .htyp = "cmd"},
        {.sh = 'F', .type = HXOPT_NONE, .ptr = &Opt.force,
         .help = "Force usage of dangerous umask"},
        {.sh = 'I', .type = HXOPT_NONE, .ptr = &Opt.inter,
         .help = "Interactively prompt for parameters"},
        {.sh = 'M', .type = HXOPT_STRING, .ptr = &Module_path,
         .help = "Use a different module than \"*\" (the default)",
         .htyp = "name"},
        {.sh = 'S', .type = HXOPT_INT, .ptr = &Opt.split_lvl,
         .help = "Use split home feature (specify split level)"},

        // Default options
        {.sh = 'G', .type = HXOPT_STRING, .ptr = &Opt.sgrp,
         .help = "Supplementary groups", .htyp = "groups"},
        {.sh = 'c', .type = HXOPT_STRING, .ptr = &Opt.gecos,
         .help = "User's comment in the password file"},
        {.sh = 'd', .type = HXOPT_STRING, .ptr = &Opt.homedir,
         .help = "Home directory of the user (overrides -S)", .htyp = "dir"},
        {.sh = 'e', .type = HXOPT_STRING, .cb = useradd_getopt_expire,
         .help = "Date when the account expires", .htyp = "date"},
        {.sh = 'f', .type = HXOPT_INT, .ptr = &Opt.inactive,
         .help = "Days until account becomes inactive", .ptr = "days"},
        {.sh = 'g', .type = HXOPT_STRING, .ptr = &Opt.igrp,
         .help = "Initial group of the user", .htyp = "group"},
        {.sh = 'k', .type = HXOPT_STRING | HXOPT_OPTIONAL, .ptr = &Opt.skeldir,
         .help = "Skeleton directory", .htyp = "dir"},
        {.sh = 'm', .type = HXOPT_NONE, .ptr = &Opt.creat,
         .help = "Create home directory"},
        {.sh = 'o', .type = HXOPT_NONE, .ptr = &Opt.dup,
         .help = "Allow creating a user with non-unique UID"},
        {.sh = 'p', .type = HXOPT_STRING, .ptr = &Opt.passwd,
         .help = "Encrypted password to use"},
        {.sh = 'r', .type = HXOPT_NONE, .ptr = &Opt.sys,
         .help = "System account (use userid < UID_MIN)"},
        {.sh = 's', .type = HXOPT_STRING, .ptr = &Opt.shell,
         .help = "The user's default shell", .htyp = "prog"},
        {.sh = 'u', .type = HXOPT_LONG, .ptr = &Opt.uid,
         .help = "Numerical value of the user's ID", .htyp = "uid"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0) {
        return 0;
    }

    if(Opt.split_lvl > 2) { Opt.split_lvl = 2; }

    if(!Opt.inter && argv[1] == NULL) {
        fprintf(stderr, "You need to specify a username\n");
        return 0;
    }

    return 1;
}

static void useradd_getopt_expire(const struct HXoptcb *cbi) {
    Opt.expire = SH_string_to_iday(cbi->s);
    return;
}

static int useradd_interactive_lname(const char *key, char type, void *ptr) {
    char **lname = ptr;
    if(**lname == '\0') { return 0; }
    if(Opt.homedir == NULL) { useradd_generate_homedir(); }
    return 1;
}

static int useradd_read_config(void) {
    static const struct shconf_opt config_table[] = {
        {"AC_BEFORE", SHCONF_STRING, &Opt.ac_before},
        {"AC_AFTER",  SHCONF_STRING, &Opt.ac_after},

        {"GROUP",       SHCONF_STRING, &Opt.igrp},
        {"HOME",        SHCONF_STRING, &Conf.homebase},
        {"CREATE_HOME", SHCONF_IBOOL,  &Opt.creat},
        {"SPLIT_LVL",   SHCONF_INT,    &Opt.split_lvl},
        {"UMASK",       SHCONF_ULONG,  &Conf.umask, useradd_pconfig_umask},
        {"SKEL",        SHCONF_STRING, &Opt.skeldir},
        {"SHELL",       SHCONF_STRING, &Opt.shell},

        {"PASS_KEEP_MIN", SHCONF_ULONG, &Conf.keep_min},
        {"PASS_KEEP_MAX", SHCONF_ULONG, &Conf.keep_max},
        {"PASS_WARN_AGE", SHCONF_ULONG, &Conf.warn_age},
        {"PASS_EXPIRE",   SHCONF_LONG,  &Opt.expire},
        {"PASS_INACTIVE", SHCONF_LONG,  &Opt.inactive},
        {NULL},
    };
    return HX_shconfig("/etc/vitalnix/useradd.conf", config_table);
}

static int useradd_pconfig_umask(const struct shconf_opt *t, const char *val) {
    unsigned long *mask = t->ptr;
    if((*mask & (S_IWGRP | S_IWOTH)) != (S_IWGRP | S_IWOTH) && !Opt.force) {
        // complain if not all write permission bits are cleared
        fprintf(stderr, "Error: will refuse to allow foreign write-access for"
         " user directory,\n" "use -F to override.\n");
    } else if((*mask & (S_IRGRP | S_IROTH)) != (S_IRGRP | S_IROTH) &&
     !Opt.force) {
        // warn if not all read permission bits are cleared
        fprintf(stderr, "Warning: other users will possibly be able to spy on"
         " the home directory\n");
    }
    return 1;
}

//=============================================================================
