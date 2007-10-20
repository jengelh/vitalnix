/*
 *	libvxmdfmt/s_a1.c - Simple text/plain formatter
 *	Copyright © CC Computer Consultants GmbH, 2002 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdio.h>
#include <vitalnix/config.h>
#include "libvxmdfmt/internal.h"
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include "libvxmdfmt/static-build.h"
#include <vitalnix/libvxmdfmt/vtable.h>
#include <vitalnix/libvxutil/defines.h>

static void a1_file_header(const struct pwlfmt_workspace *state)
{
	fprintf(state->output_fh,
		"\n\n"
		"                 Formatted by %s (%s)\n\n\n"
		"  %-8s  %-35s  %-10s  %s\n"
		"  =======================================================================\n",
		PACKAGE_NAME, PACKAGE_VERSION,
		"Group", "Name", "Login", "Password"
	);
	return;
}

static void a1_tbl_entry(const struct pwlfmt_workspace *state,
    const struct pwl_data *data)
{
	char buf[MAXSNLEN];
	snprintf(buf, MAXSNLEN, "%s, %s", data->surname, data->first_name);
	fprintf(state->output_fh, "  %-8s  %-*s  %-10s  %s\n",
	        data->pvgrp, 35 + pwlfmt_extra_whitespace(buf), buf,
	        data->username, data->password);
	return;
}

static const struct pwlstyle_vtable THIS_STYLE = {
	.name        = "a1",
	.desc        = "user-sorted text/plain",

	.file_header = a1_file_header,
	.tbl_entry   = a1_tbl_entry,
};

REGISTER_MODULE(a1, &THIS_STYLE);
