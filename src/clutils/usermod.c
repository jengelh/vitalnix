/*=============================================================================
Vitalnix User Management Suite
clutils/usermod.c - Modify a user account
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
#include <stdio.h>
#include <unistd.h>
#include <libHX.h>
#include "clutils/usermod_lib.h"
#include "libvxcli/libvxcli.h"
#include <vitalnix/libvxplex/libvxplex.h>
#include "libvxpdb/libvxpdb.h"
#include <vitalnix/libvxutil/libvxutil.h>

// Functions
static int usermod_nio(int, const char **, struct usermod_state *);
static int usermod_cli(int, const char **, struct usermod_state *);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct usermod_state state;
    int ui = vxplex_select_ui(&argc, &argv);

    usermod_fill_defaults(&state);
    if(usermod_get_options(&argc, &argv, &state) <= 0)
        return UM_EOTHER << UM_SHIFT;

    if(ui == PLEXUI_AUTO && argc > 1)
            return usermod_nio(argc, argv, &state);
    else if(ui == PLEXUI_CLI || (ui == PLEXUI_AUTO &&
     isatty(STDIN_FILENO) && argc == 1))
            return usermod_cli(argc, argv, &state);
    else
            return usermod_nio(argc, argv, &state);
}

static int usermod_nio(int argc, const char **argv,
  struct usermod_state *state)
{
    if(argc < 2) {
        fprintf(stderr, "You need to specify a username!\n");
        return UM_EOTHER << UM_SHIFT;
    }
    return usermod_run(state);
}

static int usermod_cli(int argc, const char **argv,
  struct usermod_state *state)
{
    struct vxpdb_user *user = &state->newstuff;
    struct vxcq_entry table_1[] = {
        {.prompt = "Login name", .type = HXTYPE_STRING,
         .defl = state->username, .ptr = &state->username},
        VXCQ_TABLE_END,
    };
    struct vxcq_entry table_2[] = {
        {.prompt = "User ID (-1 for next free)", .defl = "-1",
         .type = HXTYPE_LONG, .ptr = &user->pw_uid},
        {.prompt = "Initial group (or GID)",
         .type = HXTYPE_STRING, .ptr = &user->pw_igrp},
        {.prompt = "Supplemental groups (and/or GIDs), seperated by comma",
         .type = HXTYPE_STRING, .ptr = &user->pw_sgrp},
        {.prompt = "Real name",
         .type = HXTYPE_STRING, .ptr = &user->pw_real},
        {.prompt = "Home directory",
         .type = HXTYPE_STRING, .ptr = &user->pw_home},
        {.prompt = "Default shell",
         .type = HXTYPE_STRING, .ptr = &user->pw_shell},
        {.prompt = "Skeleton directory",
         .type = HXTYPE_STRING, .ptr = &state->skeldir},
        VXCQ_TABLE_END,
    };

    return usermod_run(state);
}

/*
enum {
    AOP_LOCK = 1,
    AOP_UNLOCK,
};

//-----------------------------------------------------------------------------
int usermod_main(int argc, const char **argv) {
    struct adb_module *db;
    struct vxp_user mmask = {}, smask = {}, result = {};
    int ret, rv = E_SUCCESS;
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
        fprintf(stderr, "Could not load PDB back-end module \"%s\": %s\n",
         Module_path, strerror(errno));
        return E_OPEN;
    }

    if((ret = db->open(db, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open PDB back-end: %s"
         " (ret=%d, errno=%d)\n",
         strerror(errno), ret, errno);
        rv = E_OPEN;
        goto __main__close_pdb;
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
        memcpy(&mmask, &result, sizeof(struct vxp_user));
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
        struct vxp_user quser = {.uid = mmask.uid, .gid = -1};
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
        struct vxp_user quser = {.lname = mmask.lname, .uid = -1, .gid = -1};
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

    if((ret = db->usermod(db, &smask, &mmask)) <= 0) {
        fprintf(stderr, "Error: User updating failed: %s (ret=%d, errno=%d)\n",
         strerror(errno), ret, errno);
        rv = E_UPDATE;
    } else if(Opt.ac_after != NULL) {
        SH_runcmd(Opt.ac_after, sr_map);
    }

    // ----------------------------------------
 __main__close_backend:
    if((ret = db->close(db)) <= 0) {
        fprintf(stderr, "Warning: DB was not cleanly closed:"
         " %s (ret=%d, errno=%d)\n", strerror(errno), ret, errno);
        // if there was some other error, keep that instead
        if(rv == E_SUCCESS) { rv = E_CLOSE; }
    }

 __main__close_pdb:
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
*/
//=============================================================================
