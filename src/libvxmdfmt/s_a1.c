/*
    libvxmdfmt/s_a1.c - Simple text/plain formatter
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2002 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
*/
#include <stdio.h>
#include <vitalnix/config.h>
#include "libvxmdfmt/internal.h"
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include "libvxmdfmt/static-build.h"
#include <vitalnix/libvxmdfmt/vtable.h>
#include <vitalnix/libvxutil/defines.h>

//-----------------------------------------------------------------------------
static void a1_file_header(struct pwlfmt_workspace *state,
  const struct pwl_data *data)
{
    fprintf(state->output_fh,
        "\n\n"
        "                                  Formatted by Vitalnix (%s)\n\n\n"
        "  %-8s  %-35s  %-10s  %s\n"
        "  =======================================================================\n",
        VITALNIX_VERSION, "Group", "Name", "Login", "Password"
    );
    return;
}

static void a1_tbl_entry(struct pwlfmt_workspace *state,
  const struct pwl_data *data)
{
    char buf[MAXSNLEN];
    snprintf(buf, MAXSNLEN, "%s, %s", data->surname, data->first_name);
    fprintf(state->output_fh, "  %-8s  %-*s  %-10s  %s\n",
            data->pvgrp, 35 + pwlfmt_extra_whitespace(buf), buf,
            data->username, data->password);
    return;
}

//-----------------------------------------------------------------------------
static const struct pwlstyle_vtable THIS_STYLE = {
    .name        = "a1",
    .desc        = "user-sorted text/plain",

    .file_header = a1_file_header,
    .tbl_entry   = a1_tbl_entry,
};

REGISTER_MODULE(a1, &THIS_STYLE);

//=============================================================================
