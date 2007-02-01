/*=============================================================================
Vitalnix User Management Suite
clutils/useradd.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2007
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
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct useradd_state state;
    struct vxpdb_user *user = &state.config.defaults;

    useradd_fill_defaults(&state);
    if(useradd_get_options(&argc, &argv, &state) <= 0)
        return UA_EOTHER << UA_SHIFT;

    if(argc > 1) {
        HX_strclone(&user->pw_name, argv[1]);
        if(user->pw_home == NULL)
            user->pw_home = useradd_genhome(&state);
    }

    if(state.config.defaults.pw_name == NULL) {
        fprintf(stderr, "You have to specify a username!\n");
        return UA_EOTHER << UA_SHIFT;
    }

    return useradd_run(&state);
}

//=============================================================================
