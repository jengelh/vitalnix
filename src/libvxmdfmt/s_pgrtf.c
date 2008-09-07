/*
 *	libvxmdfmt/s_pgrtf.c - text/rtf formatter
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2005 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */

/* Please do not remove the "Formatted by" messages (from neither pg_*.rtf
nor the final .rtf) as they are the only external advertisements for this
software. */

#include <errno.h>
#include <iconv.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include "libvxmdfmt/internal.h"
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include "libvxmdfmt/static-build.h"
#include <vitalnix/libvxmdfmt/vtable.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
struct pgrtf_data {
	char *template_data;
	const char *tps_file_header, *tps_tbl_header, *tps_tbl_entry;
	const char *tps_tbl_footer, *tps_file_footer;
};

/* Functions */
static int pgrtf_read_template(const char *, struct pgrtf_data *);
static hxmc_t *utf8_to_rtfuni(const char *);

//-----------------------------------------------------------------------------
static int pgrtf_construct(struct pwlfmt_workspace *w)
{
	struct pgrtf_data priv;
	int ret;

	if ((ret = pgrtf_read_template(w->template_file, &priv)) < 0)
		return ret;
	if ((w->style_data = HX_memdup(&priv, sizeof(priv))) == NULL) {
		free(priv.template_data);
		return -errno;
	}

	return 1;
}

static void pgrtf_destruct(struct pwlfmt_workspace *w)
{
	free(w->style_data);
}

static void pgrtf_file_header(const struct pwlfmt_workspace *ws)
{
	const struct pgrtf_data *priv = ws->style_data;
	struct HXbtree *catalog       = defcat_file_header(ws);

	HXformat_fprintf(catalog, ws->output_fh, priv->tps_file_header);
	HXformat_free(catalog);
}

static void pgrtf_tbl_header(const struct pwlfmt_workspace *ws,
    const struct pwl_data *data)
{
	const struct pgrtf_data *priv = ws->style_data;
	struct HXbtree *catalog       = defcat_tbl_header(ws, data);

	HXformat_fprintf(catalog, ws->output_fh, priv->tps_tbl_header);
	HXformat_free(catalog);
}

static void pgrtf_tbl_entry(const struct pwlfmt_workspace *ws,
    const struct pwl_data *data)
{
	const struct pgrtf_data *priv = ws->style_data;
	struct HXbtree *catalog       = defcat_tbl_entry(ws, data);
	hxmc_t *uni_firstname         = utf8_to_rtfuni(data->first_name);
	hxmc_t *uni_surname           = utf8_to_rtfuni(data->surname);

	HXformat_add(catalog, "SURNAME", uni_surname,
	             HXTYPE_STRING | HXFORMAT_IMMED);
	HXformat_add(catalog, "FIRSTNAME", uni_firstname,
	             HXTYPE_STRING | HXFORMAT_IMMED);
	HXformat_fprintf(catalog, ws->output_fh, priv->tps_tbl_entry);
}

static void pgrtf_tbl_footer(const struct pwlfmt_workspace *ws,
    const struct pwl_data *data)
{
	const struct pgrtf_data *priv = ws->style_data;
	fprintf(ws->output_fh, priv->tps_tbl_footer);
}

static void pgrtf_file_footer(const struct pwlfmt_workspace *ws)
{
	const struct pgrtf_data *priv = ws->style_data;
	fprintf(ws->output_fh, priv->tps_file_footer);
}

//-----------------------------------------------------------------------------
static int pgrtf_read_template(const char *template_file, struct pgrtf_data *p)
{
	char *ptr;

	if (template_file == NULL)
		return -EFAULT;
	if ((p->template_data = vxutil_slurp_file(template_file)) == NULL)
		return -errno;

	/*
	 * The following fixup is so that the RTF file can be edited more
	 * easily. And of course, we keep it in one file, which is essential.
	 * Microsoft Office successfully ignores our private tags, so that we
	 * can check out what our template looks like without having to do
	 * guesses.
	 */
	ptr                = p->template_data;
	p->tps_file_header = HX_strsep2(&ptr, "{\\THDR}");
	p->tps_tbl_header  = HX_strsep2(&ptr, "{\\ENTRY}");
	p->tps_tbl_entry   = HX_strsep2(&ptr, "{\\TFOOT}");
	p->tps_tbl_footer  = HX_strsep2(&ptr, "{\\FOOTER}");
	p->tps_file_footer = ptr;
	if (p->tps_file_header == NULL) p->tps_file_header = "";
	if (p->tps_tbl_header  == NULL) p->tps_tbl_header  = "";
	if (p->tps_tbl_entry   == NULL) p->tps_tbl_entry   = "";
	if (p->tps_tbl_footer  == NULL) p->tps_tbl_footer  = "";
	if (p->tps_file_footer == NULL) p->tps_file_footer = "";
	return 1;
}

static hxmc_t *utf8_to_rtfuni(const char *ip)
{
	const char *cfh = ip;
	iconv_t cd      = iconv_open("wchar_t", "UTF-8");
	hxmc_t *dest    = HXmc_meminit(NULL, 0);
	char buf[16];
	size_t is, os;
	wchar_t oc, *op = &oc;

	while (*ip != '\0') {
		if (*signed_cast(const unsigned char *, ip) < 0x80) {
			++ip;
			continue;
		}

		is = strlen(ip);
		os = sizeof(wchar_t);
		HXmc_memcat(&dest, cfh, ip - cfh);
		iconv(cd, reinterpret_cast(char **, &ip), &is,
		          reinterpret_cast(char **,
		          	reinterpret_cast(void *, &op)), &os);
		snprintf(buf, sizeof(buf), "\\uc0\\u%ld", static_cast(long, oc));
		HXmc_strcat(&dest, buf);
		cfh = ip;
	}

	if (*cfh != '\0')
		HXmc_strcat(&dest, cfh);
	return dest;
}

//-----------------------------------------------------------------------------
static const struct pwlstyle_vtable THIS_STYLE = {
	.name             = "pg_rtf",
	.desc             = "pvgrp-sorted text/rtf",
	.require_template = true,

	.init             = pgrtf_construct,
	.exit             = pgrtf_destruct,
	.file_header      = pgrtf_file_header,
	.tbl_header       = pgrtf_tbl_header,
	.tbl_entry        = pgrtf_tbl_entry,
	.tbl_footer       = pgrtf_tbl_footer,
	.file_footer      = pgrtf_file_footer,
};

REGISTER_MODULE(pg_rtf, &THIS_STYLE);
