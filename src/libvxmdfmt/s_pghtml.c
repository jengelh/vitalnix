/*=============================================================================
Vitalnix User Management Suite
libvxmdfmt/s_pghtml.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2002 - 2006
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
/* Please do not remove the "Formatted by" messages as they are the only
external advertisements for this software. */

#include <stdio.h>
#include <libHX.h>
#include "compiler.h"
#include "vitalnix-config.h"
#include "libvxmdfmt/internal.h"
#include "libvxmdfmt/libvxmdfmt.h"
#include "libvxmdfmt/static-build.h"
#include "libvxmdfmt/vtable.h"
#include "libvxutil/defines.h"

// Functions
static printfunc_t pghtml_file_header, pghtml_tbl_header, pghtml_tbl_entry,
                   pghtml_tbl_footer, pghtml_file_footer;

// Variables
static const struct pwlstyle_vtable THIS_STYLE = {
    .name        = "pg_html",
    .desc        = "pvgrp-sorted text/html",

    .file_header = pghtml_file_header,
    .tbl_header  = pghtml_tbl_header,
    .tbl_entry   = pghtml_tbl_entry,
    .tbl_footer  = pghtml_tbl_footer,
    .file_footer = pghtml_file_footer,
};

//-----------------------------------------------------------------------------
REGISTER_MODULE(pg_html, &THIS_STYLE);

static void pghtml_file_header(struct pwlfmt_workspace *state,
 const struct pwl_data *data)
{
    fprintf(state->output_fh,
        "<html><head>"
        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">"
        "<head><title>%s</title><style type=\"text/css\">"
        "h1 { font: bold 16pt \"Arial\", sans-serif; }\n"
        "th { font: bold 12pt \"Arial\", sans-serif; text-align: left;"
        " border-bottom: 1.5pt solid #000000; }\n"
        ".serif { font: 12pt \"Times New Roman\", \"Times\", serif; }\n"
        ".sserif { font: 12pt \"Verdana\", \"Tahoma\", \"Arial\", sans-serif; }\n"
        "</style><body>",
        state->input_file
    );
    return;
}

static void pghtml_tbl_header(struct pwlfmt_workspace *state,
 const struct pwl_data *data)
{
    fprintf(state->output_fh,
        "<p>&nbsp;</p><table width=\"100%%\"><tr><td><h1>Group %s</h1></td>"
        "<td align=\"right\"><i>Formatted by Vitalnix (%s)</i></td></tr>"
        "</table><table cellpadding=\"5\" width=\"100%%\"><tr>"
        "<th width=\"50%%\">Name</th><th>Login</th><th>Password</th></tr>\n",
        data->pvgrp, VITALNIX_VERSION
    );
    return;
}

static void pghtml_tbl_entry(struct pwlfmt_workspace *state,
 const struct pwl_data *data)
{
    char buf[MAXSNLEN];
    struct HXoption catalog[] = {
        {.sh = 'n', .type = HXTYPE_STRING, .ptr = buf},
        {.sh = 'p', .type = HXTYPE_STRING, .ptr = static_cast(void *, data->password)},
        {.sh = 'u', .type = HXTYPE_STRING, .ptr = static_cast(void *, data->username)},
        HXOPT_TABLEEND,
    };
    snprintf(buf, MAXSNLEN, "%s, %s", data->surname, data->first_name);
    HX_fstrrep(state->output_fh, "<tr><td>%n</td><td class=\"sserif\">%u</td>"
               "<td class=\"sserif\">%p</td></tr>\n", catalog);
    return;
}

static void pghtml_tbl_footer(struct pwlfmt_workspace *state,
 const struct pwl_data *data)
{
    fprintf(state->output_fh, "</table>");
    return;
}

static void pghtml_file_footer(struct pwlfmt_workspace *state,
 const struct pwl_data *data)
{
    fprintf(state->output_fh, "</body></html>");
    return;
}

//=============================================================================
