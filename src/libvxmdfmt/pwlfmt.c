/*
 *	libvxmdfmt/pwlfmt.c - Password list formatting
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2005 - 2009
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/map.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxcore/loader.h>
#include "libvxmdfmt/internal.h"
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include <vitalnix/libvxmdfmt/vtable.h>
#include "libvxmdsync/logversion.h"

/* Functions */
static int pwl_build_tree(FILE *, struct HXmap **);
static void pwlfmt_destruct(struct pwlfmt_workspace *);
static int pwlfmt_process2(struct pwlfmt_workspace *, const struct HXmap *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int pwlfmt_new(struct pwlfmt_workspace *w)
{
	int esave, ret;

	w->vtable = vxcore_module_lookup("libvxmdfmt", w->style_name);
	if (w->vtable == NULL)
		return -PWLFMT_ENOSTYLE;
	if (w->vtable->require_template && w->template_file == NULL)
		return -PWLFMT_EREQTPL;

	w->input_fh      = NULL;
	w->output_fh     = NULL;
	w->style_data    = NULL;
	w->template_data = NULL;

	if (w->vtable->init != NULL && (ret = w->vtable->init(w)) <= 0)
		return ret;
	if ((w->input_fh = fopen(w->input_file, "r")) == NULL) {
		esave = errno;
		ret   = -PWLFMT_EEINPUT;
		goto out;
	}
	if ((w->output_fh = fopen(w->output_file, "w")) == NULL) {
		esave = errno;
		ret   = -PWLFMT_EEOUTPUT;
		goto out;
	}

	return 1;

 out:
	pwlfmt_destruct(w);
	errno = esave;
	return ret;
}

EXPORT_SYMBOL int pwlfmt_process(struct pwlfmt_workspace *w)
{
	struct HXmap *group_list = NULL;
	int esave = 0, ret;

	if ((ret = pwl_build_tree(w->input_fh, &group_list)) == 0) {
		ret = -PWLFMT_ENOUSERS;
		goto out;
	}
	if (ret < 0) {
		esave = errno;
		goto out;
	}

	w->num_entries = ret;
	ret = pwlfmt_process2(w, group_list);

 out:
	pwlfmt_destruct(w);
	if (group_list != NULL)
		HXmap_free(group_list);
	errno = esave;
	return ret;
}

EXPORT_SYMBOL const char *pwlfmt_strerror(int err)
{
#define B PWLFMT_ESUCCESS
	static const char *const errlist[] = {
		[PWLFMT_ESUCCESS  - B] = "Success",
		[PWLFMT_ENOSTYLE  - B] = "Style not found",
		[PWLFMT_EREQTPL   - B] = "Style requires template",
		[PWLFMT_EEINPUT   - B] = "Could not open input file",
		[PWLFMT_EEOUTPUT  - B] = "Could not open output file",
		[PWLFMT_EINPUTVER - B] = "Version of input file not supported",
		[PWLFMT_ENOUSERS  - B] = "No users found in logfile. "
		                         "(Wrong file and/or format?)",
	};
#undef B
	if (err < 0)
		err = -err;
	err -= PWLFMT_ESUCCESS;
	if (err >= ARRAY_SIZE(errlist))
		return NULL;
	return errlist[err];
}

EXPORT_SYMBOL const struct pwlstyle_vtable *pwlstyles_trav(void **trav_pptr)
{
	return vxcore_section_trav(trav_pptr, "libvxmdfmt");
}

//-----------------------------------------------------------------------------
/**
 * pwl_build_tree -
 * @fh:		stdio filehandle to read from
 * @b_ret:	pointer to storage point for resulting B-tree
 *
 * Reads the MDSYNC log from @fh and puts new users into a two-level
 * pvgrp-keyed ordered map, sorting them as a wanted side-effect. The pointer
 * to the map is put into @*b_ret.
 *
 * Returns the number of entries put into the structures.
 */
static int pwl_build_tree(FILE *fh, struct HXmap **b_ret)
{
	static const struct HXmap_ops level1_ops = {
		.k_compare = reinterpret_cast(HXtypeof_member(
		             typeof(level1_ops), k_compare), compare_wbc),
		.d_free    = reinterpret_cast(HXtypeof_member(
		             typeof(level1_ops), d_free), HXmap_free),
	};
	static const struct HXmap_ops level2_ops = {
		.k_compare = reinterpret_cast(HXtypeof_member(
		             typeof(level2_ops), k_compare), strcoll),
	};
	unsigned int seen_format_id = 0;
	struct HXmap *group_map;
	hxmc_t *ln = NULL;
	int ret = 0;

	group_map = HXmap_init5(HXMAPT_ORDERED, HXMAP_SCKEY,
	            &level1_ops, 0, 0);
	if (group_map == NULL)
		return -errno;

	while (HX_getl(&ln, fh) != NULL) {
		char *bufp = ln, *c_type, *c_pvgrp;
		struct HXmap *user_list;

		HX_chomp(ln);
		if (*ln == '\0')
			continue;
		if (*ln == '#') {
			if (!seen_format_id &&
			    strncmp(ln, "# $logformat", 12) == 0) {
				if (strcmp(ln + 12, " " MDSYNC_LOG_VER) != 0) {
					fprintf(stderr, "Input file is not of version \"" MDSYNC_LOG_VER "\"\n");
					HXmap_free(group_map);
					return -PWLFMT_EINPUTVER;
				}
				++seen_format_id;
			}
			continue;
		}

		c_type  = HX_strsep(&bufp, ":");
		c_pvgrp = HX_strsep(&bufp, ":");
		if (*c_type != '+' || c_pvgrp == NULL || bufp == NULL)
			continue;

		/* Only print users which were added ('+') */
		if ((user_list = HXmap_get(group_map, c_pvgrp)) == NULL) {
			user_list = HXmap_init5(HXMAPT_ORDERED, HXMAP_SCKEY |
			            HXMAP_SINGULAR, &level2_ops, 0, 0);
			HXmap_add(group_map, c_pvgrp, user_list);
		}
		HXmap_add(user_list, bufp, NULL);
		++ret;
	}

	*b_ret = group_map;
	return ret;
}

/**
 * pwlfmt_destruct - Destroy the workspace
 * @w:	Workspace to free
 *
 * Calls the destructor function of the output style and frees any resources
 * used by the workspace.
 */
static void pwlfmt_destruct(struct pwlfmt_workspace *w)
{
	if (w->vtable->exit != NULL)
		w->vtable->exit(w);
	if (w->input_fh != NULL)
		fclose(w->input_fh);
	if (w->output_fh != NULL)
		fclose(w->output_fh);
	if (w->template_data != NULL)
		free(w->template_data);
}

/**
 * pwlfmt_process2 - Print tree
 * @w:		Workspace to operate on
 * @group_list:	Groups/users to print out
 *
 * Feeds the map's contents (previously generated by pwl_build_tree()) to the
 * style's functions.
 */
static int pwlfmt_process2(struct pwlfmt_workspace *w,
    const struct HXmap *tl_tree)
{
	const struct HXmap_node *tl_node;
	unsigned int proc = 0;
	struct pwl_data data;
	struct HXmap_trav *tl_trav;

	if ((tl_trav = HXmap_travinit(tl_tree, 0)) == NULL)
		return -errno;

	if (w->vtable->file_header != NULL)
		w->vtable->file_header(w);

	while ((tl_node = HXmap_traverse(tl_trav)) != NULL) {
		struct HXmap *pg_tree = tl_node->data;
		const char *pg_name = tl_node->skey;
		struct HXmap_trav *pg_trav = HXmap_travinit(pg_tree, 0);
		const struct HXmap_node *pg_node;

		if (w->vtable->tbl_header != NULL) {
			memset(&data, 0, sizeof(data));
			data.pvgrp = pg_name;
			w->vtable->tbl_header(w, &data);
		}

		while ((pg_node = HXmap_traverse(pg_trav)) != NULL) {
			/*
			 * Umm, we are modifying the key here. Not nice.
			 * It will be freed shortly in the caller anyway, tho.
			 */
			char *field[5], *sd = pg_node->key;

			if (HX_split5(sd, ":", ARRAY_SIZE(field), field) != 4) {
				fprintf(stderr, "%s: Error in logfile and/or "
				        "tree, during group \"%s\"\n",
				        __PRETTY_FUNCTION__, pg_name);
				continue;
			}

			memset(&data, 0, sizeof(data));
			data.pvgrp      = pg_name;
			data.first_name = field[1];
			data.surname    = field[0];
			data.username   = field[2];
			data.password   = field[3];
			w->vtable->tbl_entry(w, &data);

			if (w->report != NULL)
				w->report(w, ++proc, w->num_entries);
		}

		if (w->vtable->tbl_footer != NULL) {
			memset(&data, 0, sizeof(data));
			data.pvgrp = pg_name;
			w->vtable->tbl_footer(w, &data);
		}

		HXmap_travfree(pg_trav);
	}

	if (w->vtable->file_footer != NULL)
		w->vtable->file_footer(w);
	HXmap_travfree(tl_trav);
	return 1;
}
