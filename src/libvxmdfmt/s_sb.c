/*=============================================================================
Vitalnix User Management Suite
libvxmdfmt/s_sb.c
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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libHX.h>
#include "libvxmdfmt/internal.h"
#include "libvxmdfmt/libvxmdfmt.h"
#include "libvxmdfmt/static-build.h"
#include "libvxmdfmt/vtable.h"
#include <vitalnix/libvxutil/libvxutil.h>

// Functions
static int sb_construct(struct pwlfmt_workspace *);
static void sb_destruct(struct pwlfmt_workspace *);

//-----------------------------------------------------------------------------
static int sb_construct(struct pwlfmt_workspace *state) {
    /* Unlike SG_rtf, this template should only contain the
    THDR/ENTRY parts! (TFOOT is handled in sb_entry().) */
    if(state->template_file == NULL) {
        fprintf(stderr, "sb style requires a template! (-t option)\n");
        return errno = 0;
    }

    state->template_data = vxutil_slurp_file(state->template_file);
    if(state->template_data == NULL) {
        fprintf(stderr, "%s: Could not open template file %s: %s\n",
                __PRETTY_FUNCTION__, state->template_file, strerror(errno));
        return -errno;
    }
    return 1;
}

static void sb_destruct(struct pwlfmt_workspace *state) {
    free(state->template_data);
    return;
}

static void sb_file_header(struct pwlfmt_workspace *state,
  const struct pwl_data *data)
{
    fprintf(state->output_fh,
        "\\documentclass[12pt]{article}\n\n"
        "\\usepackage{isolatin1}\n"
        "\\usepackage{graphics}\n"
        "\\usepackage{german}\n\n"
        "\\begin{document}\n\n"
    );
    return;
}

static void sb_tbl_entry(struct pwlfmt_workspace *state,
  const struct pwl_data *data)
{
    DEFCAT_TBL_ENTRY(state, data);
    HX_fstrrep(state->output_fh, state->template_data, catalog);
    fprintf(state->output_fh, "\\newpage\n");
    return;
}

static void sb_file_footer(struct pwlfmt_workspace *state,
  const struct pwl_data *data)
{
    fprintf(state->output_fh, "\\end{document}\n");
    return;
}

//-----------------------------------------------------------------------------
static const struct pwlstyle_vtable THIS_STYLE = {
    .name             = "sb",
    .desc             = "per-user application/x-latex",
    .author           = "Christoph Thiel <cthiel1 [at] linux01 gwdg de>, 2002",
    .require_template = 1,

    .init             = sb_construct,
    .exit             = sb_destruct,
    .file_header      = sb_file_header,
    .tbl_entry        = sb_tbl_entry,
    .file_footer      = sb_file_footer,
};

REGISTER_MODULE(sb, &THIS_STYLE);

//=============================================================================
