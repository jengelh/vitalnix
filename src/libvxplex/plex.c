/*=============================================================================
Vitalnix User Management Suite
libvxplex/plex.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2007
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
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxplex/libvxplex.h>

enum {
    COMMAND_NOT_FOUND = 127, // see bash(1)
};

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int vxplex_enter(const char *library, const char *function,
  int argc, const char **argv, void *ptr)
{
    int (*entrypoint)(int, const char **, void *);
    void *handle;
    int ret;

    if((handle = HX_dlopen(library)) == NULL) {
        fprintf(stderr, "Could not load %s: %s\n", library, HX_dlerror());
        return COMMAND_NOT_FOUND;
    }

    if((entrypoint = HX_dlsym(handle, function)) == NULL) {
        fprintf(stderr, "Could not retrieve symbol %s: %s\n",
         function, HX_dlerror());
        HX_dlclose(handle);
        return COMMAND_NOT_FOUND;
    }

    ret = entrypoint(argc, argv, ptr);
    HX_dlclose(handle);
    return ret;
}

EXPORT_SYMBOL unsigned int vxplex_select_ui(int *argc, const char ***argv)
{
    int ui = PLEXUI_AUTO;
    struct HXoption options_table[] = {
        {.ln = "nio", .type = HXTYPE_VAL, .ptr = &ui, .val = PLEXUI_NIO},
        {.ln = "cli", .type = HXTYPE_VAL, .ptr = &ui, .val = PLEXUI_CLI},
        {.ln = "gui", .type = HXTYPE_VAL, .ptr = &ui, .val = PLEXUI_GUI},
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_PTHRU) <= 0) {
        struct HXoptcb cbi = {.arg0 = **argv, .table = options_table};
        HX_getopt_help(&cbi, stderr);
        return -1;
    }

    return ui;
}

//=============================================================================
