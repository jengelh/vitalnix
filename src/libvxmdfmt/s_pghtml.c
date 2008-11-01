/*
 *	libvxmdfmt/s_pghtml.c - text/html formatter
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2002 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */

/* Please do not remove the "Formatted by" messages as they are the only
external advertisements for this software. */

#include <stdio.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include "libvxmdfmt/internal.h"
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include "libvxmdfmt/static-build.h"
#include <vitalnix/libvxmdfmt/vtable.h>

static void pghtml_file_header(const struct pwlfmt_workspace *state)
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
}

static void pghtml_tbl_header(const struct pwlfmt_workspace *state,
    const struct pwl_data *data)
{
	fprintf(state->output_fh,
		"<p>&nbsp;</p><table width=\"100%%\"><tr><td><h1>Group %s</h1></td>"
		"<td align=\"right\"><i>Formatted by %s (%s)</i></td></tr>"
		"</table><table cellpadding=\"5\" width=\"100%%\"><tr>"
		"<th width=\"50%%\">Name</th><th>Login</th><th>Password</th></tr>\n",
		data->pvgrp, PACKAGE_NAME, PACKAGE_VERSION
	);
}

static void pghtml_tbl_entry(const struct pwlfmt_workspace *state,
    const struct pwl_data *data)
{
	fprintf(state->output_fh,
		"<tr><td>%s, %s</td>"
		"<td class=\"sserif\">%s</td>"
		"<td class=\"sserif\">%s</td></tr>\n",
		   data->surname, data->first_name,
		   data->username,
		   data->password);
}

static void pghtml_tbl_footer(const struct pwlfmt_workspace *state,
    const struct pwl_data *data)
{
	fprintf(state->output_fh, "</table>");
}

static void pghtml_file_footer(const struct pwlfmt_workspace *state)
{
	fprintf(state->output_fh, "</body></html>");
}

static const struct pwlstyle_vtable THIS_STYLE = {
	.name        = "pg_html",
	.desc        = "pvgrp-sorted text/html",

	.file_header = pghtml_file_header,
	.tbl_header  = pghtml_tbl_header,
	.tbl_entry   = pghtml_tbl_entry,
	.tbl_footer  = pghtml_tbl_footer,
	.file_footer = pghtml_file_footer,
};

REGISTER_MODULE(pg_html, &THIS_STYLE);
