/*=============================================================================
Vitalnix User Management Suite
clutils/userdel.c
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
#include "clutils/userdel_lib.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxplex/libvxplex.h"
#include <vitalnix/libvxutil/libvxutil.h>

// Functions
static int userdel_nio(int, const char **, struct userdel_state *);
static int userdel_cli(int, const char **, struct userdel_state *);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct userdel_state state;
    int ui = vxplex_select_ui(&argc, &argv);

    userdel_fill_defaults(&state);
    if(userdel_get_options(&argc, &argv, &state) <= 0)
        return UD_EOTHER << UD_SHIFT;

    if(ui == PLEXUI_AUTO && argc > 1)
            return userdel_nio(argc, argv, &state);
    else if(ui == PLEXUI_CLI || (ui == PLEXUI_AUTO &&
     isatty(STDIN_FILENO) && argc == 1))
            return userdel_cli(argc, argv, &state);
    else
            return userdel_nio(argc, argv, &state);
}

static int userdel_nio(int argc, const char **argv,
  struct userdel_state *state)
{
    if(argc < 2) {
        fprintf(stderr, "You have to specify a username!\n");
        return UD_EOTHER << UD_SHIFT;
    }
    return userdel_run(state);
}

static int userdel_cli(int argc, const char **argv,
  struct userdel_state *state)
{
    return 0;
}

//=============================================================================
