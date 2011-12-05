/*
 *	libvxmdfmt/s_sb.c - Form letter formatter (Seriendruck)
 *	by Christoph Thiel <cthiel1 [at] linux01 gwdg de>, 2002
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2002 - 2010
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <libHX/option.h>
#include "libvxmdfmt/internal.h"
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include "libvxmdfmt/static-build.h"
#include <vitalnix/libvxmdfmt/vtable.h>
#include <vitalnix/libvxutil/libvxutil.h>

static int sb_construct(struct pwlfmt_workspace *state)
{
	/*
	 * Unlike SG_rtf, this template should only contain the
	 * THDR/ENTRY parts! (TFOOT is handled in sb_entry().)
	 */
	if (state->template_file == NULL) {
		fprintf(stderr, "sb style requires a template! (-t option)\n");
		return errno = 0;
	}

	state->template_data = vxutil_slurp_file(state->template_file);
	if (state->template_data == NULL) {
		fprintf(stderr, "%s: Could not open template file %s: %s\n",
		        __PRETTY_FUNCTION__, state->template_file, strerror(errno));
		return -errno;
	}
	return 1;
}

static void sb_destruct(struct pwlfmt_workspace *state)
{
	free(state->template_data);
}

static void sb_file_header(const struct pwlfmt_workspace *state)
{
	fprintf(state->output_fh,
		"\\documentclass[12pt]{article}\n\n"
		"\\usepackage{isolatin1}\n"
		"\\usepackage{graphics}\n"
		"\\usepackage{german}\n\n"
		"\\begin{document}\n\n"
	);
}

static void sb_tbl_entry(const struct pwlfmt_workspace *state,
    const struct pwl_data *data)
{
	struct HXformat_map *catalog = defcat_tbl_entry(state, data);
	HXformat_fprintf(catalog, state->output_fh, state->template_data);
	fprintf(state->output_fh, "\\newpage\n");
}

static void sb_file_footer(const struct pwlfmt_workspace *state)
{
	fprintf(state->output_fh, "\\end{document}\n");
}

static const struct pwlstyle_vtable THIS_STYLE = {
	.name             = "sb",
	.desc             = "per-user application/x-latex",
	.require_template = true,

	.init             = sb_construct,
	.exit             = sb_destruct,
	.file_header      = sb_file_header,
	.tbl_entry        = sb_tbl_entry,
	.file_footer      = sb_file_footer,
};

REGISTER_MODULE(sb, &THIS_STYLE);
