/*=============================================================================
Vitalnix User Management Suite
clutils/useradd.c
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
#include <vitalnix/compiler.h>
#include "clutils/useradd_lib.h"
#include "libvxcli/libvxcli.h"
#include "libvxpdb/libvxpdb.h"
#include <vitalnix/libvxplex/libvxplex.h>
#include <vitalnix/libvxutil/libvxutil.h>

// Functions
static int useradd_nio(int, const char **, struct useradd_state *);
static int useradd_cli(int, const char **, struct useradd_state *);
static int valid_user(const struct vxcq_entry *);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct useradd_state state;
    struct vxpdb_user *user = &state.user;
    int ui = vxplex_select_ui(&argc, &argv);

    useradd_fill_defaults(&state);
    if(useradd_get_options(&argc, &argv, &state) <= 0)
        return UA_EOTHER << UA_SHIFT;

    if(argc > 1) {
        HX_strclone(&user->pw_name, argv[1]);
        if(user->pw_home == NULL)
            user->pw_home = useradd_genhome(&state);
    }

    if(ui == PLEXUI_AUTO && argc > 1)
            return useradd_nio(argc, argv, &state);
    else if(ui == PLEXUI_CLI || (ui == PLEXUI_AUTO &&
     isatty(STDIN_FILENO) && argc == 1))
            return useradd_cli(argc, argv, &state);
    else
            return useradd_nio(argc, argv, &state);
}

static int useradd_nio(int argc, const char **argv,
  struct useradd_state *state)
{
    if(state->user.pw_name == NULL) {
        fprintf(stderr, "You have to specify a username!\n");
        return UA_EOTHER << UA_SHIFT;
    }
    return useradd_run(state);
}

static int useradd_cli(int argc, const char **argv,
  struct useradd_state *state)
{
    struct vxpdb_user *user = &state->user;
    struct vxcq_entry table1[] = {
        {.prompt = "Real name", .type = HXTYPE_STRING, .ptr = &user->pw_real},
        VXCQ_TABLE_END,
    };
    struct vxcq_entry table2[] = {
        {.prompt = "Login name", .type = HXTYPE_STRING,
         .ptr = &user->pw_name, .validate = valid_user},
        {.prompt = "User ID (-1 for next free)", .defl = "-1",
         .type = HXTYPE_LONG, .ptr = &user->pw_uid},
        {.prompt = "Initial group (or GID)",
         .type = HXTYPE_STRING, .ptr = &user->pw_igrp},
        {.prompt = "Supplemental groups (and/or GIDs), seperated by comma",
         .type = HXTYPE_STRING, .ptr = &user->pw_sgrp},
        {.prompt = "Home directory",
         .type = HXTYPE_STRING, .ptr = &user->pw_home},
        {.prompt = "Default shell",
         .type = HXTYPE_STRING, .ptr = &user->pw_shell},
        {.prompt = "Skeleton directory",
         .type = HXTYPE_STRING, .ptr = &state->skeldir},
        VXCQ_TABLE_END,
    };

    vxcli_query_v(table1);
    //vxutil_propose_lname(user);
    if(user->pw_home == NULL)
        user->pw_home = useradd_genhome(state);
    vxcli_query_v(table2);
    return useradd_run(state);
}

static int valid_user(const struct vxcq_entry *p) {
    return vxutil_valid_username(*static_cast(const char **, p->ptr));
}

//=============================================================================
