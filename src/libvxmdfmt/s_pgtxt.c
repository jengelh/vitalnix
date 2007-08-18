/*
 *	libvxmdfmt/s_pgtxt.c - text/plain formatter
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2002 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */

/* Please do not remove the "Formatted by" messages as they are the only
external advertisements for this software. */

#include <stdio.h>
#include <vitalnix/config.h>
#include "libvxmdfmt/internal.h"
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include "libvxmdfmt/static-build.h"
#include <vitalnix/libvxmdfmt/vtable.h>
#include <vitalnix/libvxutil/defines.h>

static void pgtxt_tbl_header(const struct pwlfmt_workspace *state,
    const struct pwl_data *data)
{
	fprintf(state->output_fh,
		"\n\n"
		"                 Formatted by %s (%s)\n\n\n"
		"  >>> Group %s\n\n"
		"  %-35s  %-10s  %s\n"
		"  =======================================================================\n",
		PACKAGE_NAME, PACKAGE_VERSION, data->pvgrp,
		"Name", "Login", "Password"
	);
	return;
}

static void pgtxt_tbl_entry(const struct pwlfmt_workspace *state,
    const struct pwl_data *data)
{
	char buf[MAXSNLEN];
	snprintf(buf, MAXSNLEN, "%s, %s", data->surname, data->first_name);
	fprintf(state->output_fh, "  %-*s  %-10s  %s\n",
	        35 + pwlfmt_extra_whitespace(buf), buf, data->username,
	        data->password);
	return;
}

static void pgtxt_tbl_footer(const struct pwlfmt_workspace *state,
    const struct pwl_data *data)
{
	fputc('\f', state->output_fh);
	return;
}

static const struct pwlstyle_vtable THIS_STYLE = {
	.name       = "pg_txt",
	.desc       = "pvgrp-sorted text/plain",

	.tbl_header = pgtxt_tbl_header,
	.tbl_entry  = pgtxt_tbl_entry,
	.tbl_footer = pgtxt_tbl_footer,
};

REGISTER_MODULE(pg_txt, &THIS_STYLE);
