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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sysprog/shared.h"

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
#define TARGET(s) \
    if(strcmp(*args, #s) == 0 || strcmp(*args, "v"#s) == 0) { \
        return s##_main(argk, args); \
    }

    const char **args = argv; // have a copy so that printf() at the end works
    const char *base;
    int argk = argc;
    int i = 2;

    if((base = strrchr(*args, '/')) == NULL) {
        base = *args;
    } else {
        ++base;
        *args = base;
    }

    while(i--) {
        TARGET(useradd)
        else TARGET(usermod)
        else TARGET(userdel)
        else TARGET(groupadd)
        else TARGET(groupmod)
        else TARGET(groupdel)
        --argk;
        ++args;
    }
    fprintf(stderr, "%s: Can not find component \"%s\"\n", argv[0], argv[1]);
    return EXIT_FAILURE;
#undef TARGET
}

//=============================================================================
