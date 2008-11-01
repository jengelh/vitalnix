#ifndef _VITALNIX_LIBVXMDFMT_INTERNAL_H
#define _VITALNIX_LIBVXMDFMT_INTERNAL_H 1

#ifndef __cplusplus
#	include <stdio.h>
#else
#	include <cstdio>
#endif
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include "libvxmdfmt/static-build.h"

#ifdef __cplusplus
extern "C" {
#endif

struct HXbtree;

struct pwl_data {
	const char *pvgrp, *first_name, *surname, *username, *password;
};

/*
 *	EXTRA.C
 */
extern int compare_wbc(const char *, const char *);
extern struct HXbtree *defcat_file_header(const struct pwlfmt_workspace *);
extern struct HXbtree *defcat_tbl_header(const struct pwlfmt_workspace *,
	const struct pwl_data *);
extern struct HXbtree *defcat_tbl_entry(const struct pwlfmt_workspace *,
	const struct pwl_data *);
extern int pwlfmt_extra_whitespace(const char *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXMDFMT_INTERNAL_H */
