/*=============================================================================
Vitalnix User Management Suite
libvxmdfmt/s_pgtxt.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2002 - 2007
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
#include <vitalnix/config.h>
#include "libvxmdfmt/internal.h"
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include "libvxmdfmt/static-build.h"
#include <vitalnix/libvxmdfmt/vtable.h>
#include <vitalnix/libvxutil/defines.h>

//-----------------------------------------------------------------------------
static void pgtxt_tbl_header(struct pwlfmt_workspace *state,
  const struct pwl_data *data)
{
    fprintf(state->output_fh,
        "\n\n"
        "                                  Formatted by Vitalnix (%s)\n\n\n"
        "  >>> Group %s\n\n"
        "  %-35s  %-10s  %s\n"
        "  =======================================================================\n",
        VITALNIX_VERSION, data->pvgrp, "Name", "Login", "Password"
    );
    return;
}

static void pgtxt_tbl_entry(struct pwlfmt_workspace *state,
  const struct pwl_data *data)
{
    char buf[MAXSNLEN];
    snprintf(buf, MAXSNLEN, "%s, %s", data->surname, data->first_name);
    fprintf(state->output_fh, "  %-*s  %-10s  %s\n",
            35 + pwlfmt_extra_whitespace(buf), buf, data->username,
            data->password);
    return;
}

static void pgtxt_tbl_footer(struct pwlfmt_workspace *state,
  const struct pwl_data *data)
{
    fputc('\f', state->output_fh);
    return;
}

//-----------------------------------------------------------------------------
static const struct pwlstyle_vtable THIS_STYLE = {
    .name       = "pg_txt",
    .desc       = "pvgrp-sorted text/plain",
    .author     = "Jan Engelhardt <jengelh [at] gmx de>, 2000 - 2006",

    .tbl_header = pgtxt_tbl_header,
    .tbl_entry  = pgtxt_tbl_entry,
    .tbl_footer = pgtxt_tbl_footer,
};

REGISTER_MODULE(pg_txt, &THIS_STYLE);

//=============================================================================
