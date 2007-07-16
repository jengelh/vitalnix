/*
 *	libvxmdfmt/internal.h
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2003 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef _VITALNIX_LIBVXMDFMT_INTERNAL_H
#define _VITALNIX_LIBVXMDFMT_INTERNAL_H 1

#ifndef __cplusplus
#    include <stdio.h>
#else
#    include <cstdio>
#endif
#include <libHX.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include "libvxmdfmt/static-build.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pwl_data {
    const char *pvgrp, *first_name, *surname, *username, *password;
};

/*
 *      EXTRA.C
 */
extern int compare_wbc(const char *, const char *);
extern struct HXbtree *defcat_file_header(const struct pwlfmt_workspace *);
extern struct HXbtree *defcat_tbl_header(const struct pwlfmt_workspace *,
	const struct pwl_data *);
extern struct HXbtree *defcat_tbl_entry(const struct pwlfmt_workspace *,
	const struct pwl_data *);
extern int pwlfmt_extra_whitespace(const char *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXMDFMT_INTERNAL_H

//=============================================================================
