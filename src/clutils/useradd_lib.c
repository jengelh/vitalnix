/*=============================================================================
Vitalnix User Management Suite
clutils/useradd_lib.c
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
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "compiler.h"
#include "vitalnix-config.h"
#include "clutils/useradd_lib.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"
#include "libvxpdb/xwfunc.h"
#include "libvxutil/defines.h"
#include "libvxutil/libvxutil.h"

// Functions
static void useradd_getopt_expire(const struct HXoptcb *);
static int useradd_read_config(struct useradd_state *);
static void useradd_umask(const struct HXoptcb *);

//-----------------------------------------------------------------------------
/*  useradd_fill_defautls
    @state:     pointer to useradd state

    Fills @state with the hardcoded defaults and with the defaults from
    configuration file.
*/
EXPORT_SYMBOL int useradd_fill_defaults(struct useradd_state *state)
{
    struct vxpdb_user *user = &state->user;
    int ret;

    memset(state, 0, sizeof(struct useradd_state));
    vxpdb_user_clean(user);
    user->pw_shell     = "/bin/bash";
    user->sp_lastchg   = vxutil_now_iday();
    state->db_module   = "*";
    state->create_home = 1;
    state->skeldir     = "/var/lib/empty";
    state->homebase    = "/home";
    state->umask       = S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    if((ret = useradd_read_config(state)) <= 0)
        return ret;

    return 1;
}

/*  useradd_genhome

    Generate a home directory path based upon the split level that was set in
    the configuration file.
*/
EXPORT_SYMBOL char *useradd_genhome(struct useradd_state *state)
{
    struct vxpdb_user *user = &state->user;
    char buf[MAXFNLEN];

    if(user->pw_name == NULL || state->homebase == NULL)
        return NULL;

    return HX_strdup(vxutil_propose_home(buf, sizeof(buf), state->homebase,
                     user->pw_name, state->split_lvl));
}

EXPORT_SYMBOL int useradd_get_options(int *argc, const char ***argv,
 struct useradd_state *state)
{
    struct vxpdb_user *user = &state->user;
    struct HXoption options_table[] = {
        // New, Vitalnix-useradd options
        {.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &state->ac_after,
         .help = "Program to run after user addition", .htyp = "cmd"},
        {.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &state->ac_before,
         .help = "Program to run before user addition", .htyp = "cmd"},
        {.sh = 'F', .type = HXTYPE_NONE, .ptr = &state->force,
         .help = "Force usage of dangerous umask"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &state->db_module,
         .help = "Use a different module than \"*\" (the default)", .htyp = "name"},
        {.sh = 'S', .type = HXTYPE_INT, .ptr = &state->split_lvl,
         .help = "Use split home feature (specify split level)"},

        // Default options
        {.sh = 'G', .type = HXTYPE_STRING, .ptr = &user->pw_sgrp,
         .help = "Supplementary groups", .htyp = "groups"},
        {.sh = 'c', .type = HXTYPE_STRING, .ptr = &user->pw_real,
         .help = "User's comment in the password file"},
        {.sh = 'd', .type = HXTYPE_STRING, .ptr = &user->pw_home,
         .help = "Home directory of the user (overrides -S)", .htyp = "dir"},
        {.sh = 'e', .type = HXTYPE_STRING, .cb = useradd_getopt_expire,
         .ptr = user, .help = "Date when the account expires", .htyp = "date"},
        {.sh = 'f', .type = HXTYPE_INT, .ptr = &user->sp_inact,
         .help = "Days until account becomes inactive", .ptr = "days"},
        {.sh = 'g', .type = HXTYPE_STRING, .ptr = &user->pw_igrp,
         .help = "Initial group of the user", .htyp = "group"},
        {.sh = 'k', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &state->skeldir,
         .help = "Skeleton directory", .htyp = "dir"},
        {.sh = 'm', .type = HXTYPE_NONE, .ptr = &state->create_home,
         .help = "Create home directory"},
        {.sh = 'o', .type = HXTYPE_NONE, .ptr = &state->allow_dup,
         .help = "Allow creating a user with non-unique UID"},
        {.sh = 'p', .type = HXTYPE_STRING, .ptr = &user->sp_passwd,
         .help = "Encrypted password to use"},
        {.sh = 'r', .type = HXTYPE_NONE, .ptr = &state->sys_uid,
         .help = "System account (use userid < UID_MIN)"},
        {.sh = 's', .type = HXTYPE_STRING, .ptr = &user->pw_shell,
         .help = "The user's default shell", .htyp = "prog"},
        {.sh = 'u', .type = HXTYPE_LONG, .ptr = &user->pw_uid,
         .help = "Numerical value of the user's ID", .htyp = "uid"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;

    if(state->split_lvl > 2)
        state->split_lvl = 2;

    return 1;
}

EXPORT_SYMBOL int useradd_run(struct useradd_state *state)
{
    struct vxpdb_user *user;
    struct vxpdb_state *db;
    int ret, ierr = 0;

    if((db = vxpdb_load(state->db_module)) == NULL)
        return errno | (UA_ELOAD << UA_SHIFT);

    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        ierr = UA_EOPEN;
        goto close_pdb;
    }

    user = HX_memdup(&state->user, sizeof(state->user));

    if((ret = vxpdb_getpwnam(db, user->pw_name, NULL)) < 0) {
        ierr = UA_EQUERY;
        goto close_backend;
    } else if(ret > 0) {
        errno = 0;
        ierr = UA_ENAMEUSED;
        goto close_backend;
    }

    if(user->pw_uid != PDB_NOUID) { // -u is provided
        if(!state->allow_dup && vxpdb_getpwuid(db, user->pw_uid, NULL) > 0) {
            /* The -o flag (allow creating user with duplicate UID)
            was not passed. */
            ierr = UA_EUIDUSED;
            goto close_backend;
        }
    } else if(state->sys_uid) { // -r flag passed
        if((user->pw_uid = vxpdb_modctl(db, PDB_NEXTUID_SYS, db)) == -ENOSYS) {
            ierr = UA_ENOSYS;
            goto close_backend;
        }
    }

    user->sp_lastchg = vxutil_now_iday();

    if(vxutil_only_digits(user->pw_igrp)) {
        user->pw_gid  = strtoul(user->pw_igrp, NULL, 0);
        user->pw_igrp = NULL;
    } else {
        user->pw_gid  = PDB_NOGID;
    }

    struct HXoption sr_map[] = {
        {.sh = 'l', .type = HXTYPE_STRING, .ptr = &user->pw_name},
        {.sh = 'u', .type = HXTYPE_LONG,   .ptr = &user->pw_uid},
        HXOPT_TABLEEND,
    };

    if(state->ac_before != NULL)
        vxutil_replace_run(state->ac_before, sr_map);

    if((ret = vxpdb_useradd(db, user)) <= 0) {
        ierr = UA_EUPDATE;
        goto close_backend;
    }

    if(state->create_home) {
        if(HX_mkdir(user->pw_home) <= 0) {
            ierr = UA_EPOST;
            goto close_backend;
/*          fprintf(stderr, "Warning: Could not create home directory %s\n",
             strerror(errno));*/
        }

        lchown(user->pw_home, user->pw_uid, user->pw_gid);
        chmod(user->pw_home, 0755 & ~state->umask);

        if(state->skeldir != NULL && *state->skeldir != '\0')
            HX_copy_dir(state->skeldir, user->pw_home, HXF_UID | HXF_GID | HXF_KEEP,
             user->pw_uid, user->pw_gid);
    }

    if(state->ac_after != NULL)
        vxutil_replace_run(state->ac_after, sr_map);

 close_backend:
    free(user);
    vxpdb_close(db);

 close_pdb:
    vxpdb_unload(db);
    return ret | (ierr << UA_SHIFT);
}

/*
UA_UPDATE:
        fprintf(stderr, "Warning: User addition failed: %s"
         " (ret=%d, errno=%d)\n", strerror(errno), ret, errno);

UA_HOME:
            fprintf(stderr, "Warning: Could not create home directory %s\n",
             strerror(errno));

UA_CLOSE:
        fprintf(stderr, "Warning: DB was not cleanly closed: %s"
         " (ret=%d, errno=%d)\n", strerror(errno), ret, errno);
*/
EXPORT_SYMBOL const char *useradd_strerror(int e)
{
    switch(e >> UA_SHIFT) {
        case UA_ELOAD:
            return "Could not load PDB back-end";
        case UA_EOPEN:
            return "Could not open PDB back-end";
        case UA_EQUERY:
            return "Error querying the PDB";
        case UA_EUIDUSED:
            return "UID already exists";
        case UA_ENOSYS:
            return "Backend does not support PDB_NEXTUID_SYS modctl";
        case UA_ENAMEUSED:
            return "User already exists";
        case UA_EUPDATE:
            return "Error adding user";
        case UA_EPOST:
            return "Error during post setup";
        case UA_ECLOSE:
            return "Warning during close";
    }
    return "(unknown useradd error)";
}

//-----------------------------------------------------------------------------
static void useradd_getopt_expire(const struct HXoptcb *cbi) {
    struct vxpdb_user *user = cbi->current->ptr;
    user->sp_expire = vxutil_string_iday(cbi->s);
    return;
}

static int useradd_read_config(struct useradd_state *state) {
    struct vxpdb_user *user = &state->user;
    struct HXoption config_table[] = {
        {.ln = "AC_AFTER",      .type = HXTYPE_STRING, .ptr = &state->ac_after},
        {.ln = "AC_BEFORE",     .type = HXTYPE_STRING, .ptr = &state->ac_before},
        {.ln = "CREATE_HOME",   .type = HXTYPE_BOOL,   .ptr = &state->create_home},
        {.ln = "GROUP",         .type = HXTYPE_STRING, .ptr = &user->pw_igrp},
        {.ln = "HOME",          .type = HXTYPE_STRING, .ptr = &state->homebase},
        {.ln = "PASS_EXPIRE",   .type = HXTYPE_LONG,   .ptr = &user->sp_expire},
        {.ln = "PASS_INACTIVE", .type = HXTYPE_LONG,   .ptr = &user->sp_inact},
        {.ln = "PASS_KEEP_MAX", .type = HXTYPE_ULONG,  .ptr = &user->sp_max},
        {.ln = "PASS_KEEP_MIN", .type = HXTYPE_ULONG,  .ptr = &user->sp_min},
        {.ln = "PASS_WARN_AGE", .type = HXTYPE_ULONG,  .ptr = &user->sp_warn},
        {.ln = "SHELL",         .type = HXTYPE_STRING, .ptr = &user->pw_shell},
        {.ln = "SKEL",          .type = HXTYPE_STRING, .ptr = &state->skeldir},
        {.ln = "SPLIT_LVL",     .type = HXTYPE_INT,    .ptr = &state->split_lvl},
        {.ln = "UMASK",         .type = HXTYPE_ULONG,  .ptr = &state->umask,
         .cb = useradd_umask, .uptr = state},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(CONFIG_ETC_VITALNIX "/useradd.conf", config_table);
}

static void useradd_umask(const struct HXoptcb *cbi) {
    const struct useradd_state *state = cbi->current->uptr;
    unsigned long mask = cbi->l;

    if((mask & (S_IWGRP | S_IWOTH)) != (S_IWGRP | S_IWOTH) && !state->force) {
        // complain if not all write permission bits are cleared
        fprintf(stderr, "Error: will refuse to allow foreign write-access for"
         " user directory,\n" "use -F to override.\n");
        exit(UA_EOTHER);
    } else if((mask & (S_IRGRP | S_IROTH)) != (S_IRGRP | S_IROTH) &&
     !state->force) {
        // warn if not all read permission bits are cleared
        fprintf(stderr, "Warning: other users will possibly be able to spy on"
         " the home directory\n");
    }

    return;
}

//=============================================================================
