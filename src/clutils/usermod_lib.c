/*=============================================================================
Vitalnix User Management Suite
clutils/usermod_lib.c
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
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include "clutils/usermod_lib.h"
#include <vitalnix/libvxpdb/xafunc.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

// Functions
static void usermod_getopt_expire(const struct HXoptcb *);
static int usermod_read_config(struct usermod_state *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int usermod_fill_defaults(struct usermod_state *sp)
{
    struct vxpdb_user *u = &sp->newstuff;
    int ret;

    vxpdb_user_nomodify(u);
    sp->db_module = "*";
    if((ret = usermod_read_config(sp)) <= 0)
        return ret;

    return 1;
}

EXPORT_SYMBOL int usermod_get_options(int *argc, const char ***argv,
  struct usermod_state *sp)
{
    struct vxpdb_user *nu = &sp->newstuff;
    struct HXoption options_table[] = {
        // New, Vitalnix-usermod options
        {.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &sp->ac_after,
         .help = "Program to run after user modification", .htyp = "cmd"},
        {.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &sp->ac_before,
         .help = "Program to run before user modification", .htyp = "cmd"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &sp->db_module,
         .help = "Use a different module than \"*\" (the default)",
         .htyp = "name"},

        // Default options
        {.sh = 'G', .type = HXTYPE_STRING, .ptr = &nu->pw_sgrp,
         .help = "The new supplementary groups for the user", .htyp = "groups"},
        {.sh = 'L', .type = HXTYPE_VAL, .ptr = &sp->lock_account, .val = 1,
         .help = "Lock user account"},
        {.sh = 'U', .type = HXTYPE_VAL, .ptr = &sp->lock_account, .val = 2,
         .help = "Unlock user account"},
        {.sh = 'c', .type = HXTYPE_STRING, .ptr = &nu->pw_real,
         .help = "New real name", .htyp = "name"},
        {.sh = 'd', .type = HXTYPE_STRING, .ptr = &nu->pw_home,
         .help = "New home directory (implies home automove)", .htyp = "dir"},
        {.sh = 'e', .type = HXTYPE_STRING, .cb = usermod_getopt_expire,
         .help = "Date when the account expires", .htyp = "date"},
        {.sh = 'f', .type = HXTYPE_INT, .ptr = &nu->sp_inact,
         .help = "Days until account becomes inactive", .htyp = "days"},
        {.sh = 'g', .type = HXTYPE_STRING, .ptr = &nu->pw_igrp,
         .help = "The new primary group for the user", .htyp = "group"},
        {.sh = 'l', .type = HXTYPE_STRING, .ptr = &nu->pw_name,
         .help = "New login name of the user"},
        {.sh = 'm', .type = HXTYPE_NONE, .ptr = &sp->move_home,
         .help = "Move home directory if -d is given"},
        {.sh = 'o', .type = HXTYPE_NONE, .ptr = &sp->allow_dup,
         .help = "Allow creating a user with non-unique UID"},
        {.sh = 'p', .type = HXTYPE_STRING, .ptr = &nu->sp_passwd,
         .help = "New password to use (encrypted form)"},
        {.sh = 's', .type = HXTYPE_STRING, .ptr = &nu->pw_shell,
         .help = "New default shell for user", .htyp = "file"},
        {.sh = 'u', .type = HXTYPE_LONG, .ptr = &nu->pw_uid,
         .help = "New numerical value of the user's ID", .htyp = "uid"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;

    if(argv[1] == NULL) {
        // loginname is mandatory
        fprintf(stderr, "You need to specify a username\n");
        return 0;
    }

    return 1;
}

EXPORT_SYMBOL int usermod_run(struct usermod_state *state)
{
    struct vxpdb_user search;
    struct vxpdb_state *db;
    int ierr = 0, ret;

    if((db = vxpdb_load(state->db_module)) == NULL)
        return errno | (UM_ELOAD << UM_SHIFT);

    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        ierr = UM_EOPEN;
        goto close_adb;
    }

    search.pw_name = state->username;
    search.pw_uid  = PDB_NOUID;
    search.pw_gid  = PDB_NOGID;

    if((ret = vxpdb_userinfo(db, &search, NULL, 0)) < 0) {
        ierr = UM_EQUERY;
        goto close_backend;
    } else if(ret == 0) {
        errno = 0;
        ierr = UM_ENOEXIST;
        goto close_backend;
    }
/*
    if(state->ac_before != NULL)
        replace_and_runcmd(state->ac_before, sr_map);

    if((ret = db->usermod(db, &search, &modify)) <= 0) {
        ierr = UM_EUPDATE;
        goto close_backend;
    }

    if(state->ac_after != NULL)
        replace_and_runcmd(state->ac_after, sr_map);
*/
 close_backend:
    vxpdb_close(db);

 close_adb:
    vxpdb_unload(db);
    return ret | (ierr << UM_SHIFT);
}

EXPORT_SYMBOL const char *usermod_strerror(int e)
{
    switch(e >> UM_SHIFT) {
        case UM_ELOAD:
            return "Could not load PDB back-end";
        case UM_EOPEN:
            return "Could not open PDB back-end";
        case UM_EQUERY:
            return "Error querying the PDB";
        case UM_ENAMEUSED:
            return "User already exists";
        case UM_EUIDUSED:
            return "GID already exists";
        case UM_EUPDATE:
            return "Error adding user";
    }
    return "(unknown usermod error)";
}         

//-----------------------------------------------------------------------------
static void usermod_getopt_expire(const struct HXoptcb *cbi) {
    struct vxpdb_user *user = cbi->current->ptr;
    user->sp_expire = vxutil_string_iday(cbi->s);
    return;
}

static int usermod_read_config(struct usermod_state *sp) {
    struct HXoption config_table[] = {
        {.ln = "AC_BEFORE", .type = HXTYPE_STRING, .ptr = &sp->ac_before},
        {.ln = "AC_AFTER",  .type = HXTYPE_STRING, .ptr = &sp->ac_after},
        {.ln = "MOVE_HOME", .type = HXTYPE_BOOL,   .ptr = &sp->move_home},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(CONFIG_SYSCONFDIR "/usermod.conf", config_table);
}

//=============================================================================
