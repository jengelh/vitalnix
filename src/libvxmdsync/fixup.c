/*
 *	libvxmdsync/fixup.c
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdio.h>
#include <string.h>
#include <libHX/arbtree.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include "libvxmdsync/internal.h" /* get MAX_LNAME */
#include <vitalnix/libvxmdsync/libvxmdsync.h>

/* Functions */
static inline char *format_name(const char *, unsigned int, char *, size_t);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL void mdsync_fixup(struct mdsync_workspace *w)
{
	unsigned int users_max, users_proc;
	struct HXbtree_node *node;
	char tmp[MAX_LNAME+1];
	void *travp;

	if (w->add_req->items == 0)
		return;

	users_proc = 0;
	users_max  = w->add_req->items;

	travp = HXbtrav_init(w->add_req);
	while ((node = HXbtraverse(travp)) != NULL) {
		struct vxeds_entry *entry = node->data;
		unsigned int i = 0;

		HX_strlcpy(tmp, entry->username, sizeof(tmp));
		while (HXbtree_find(w->lnlist, tmp) != NULL)
			format_name(entry->username, ++i, tmp, sizeof(tmp));
		if (strcmp(entry->username, tmp) != 0)
			HX_strclone(&entry->username, tmp);
		HXbtree_add(w->lnlist, tmp);
		if (w->report != NULL)
			w->report(MDREP_FIXUP, w, ++users_proc, users_max);
	}

	HXbtrav_free(travp);
}

//-----------------------------------------------------------------------------
/*
 * format_name -
 * @name:	Login name
 * @idx:	Index number
 * @dest:	Destination buffer
 * @dsize:	Buffer size
 *
 * Constructs a new username from the original one (@name) and an index number,
 * possibly truncating @name to make @idx fit into eight characters (the
 * preferred UNIX username length).
 */
static inline char *format_name(const char *name, unsigned int idx,
    char *dest, size_t dsize)
{
	char istr[24];
	int ret;

	ret = snprintf(istr, sizeof(istr), "%u", idx);
	if (ret >= MAX_LNAME - 3) {
		fprintf(stderr, "%s: FATAL ERROR: Index number is too large "
		        "to fit\n", __func__);
		abort();
	}

	snprintf(dest, dsize, "%.*s%s", MAX_LNAME - ret, name, istr);
	return dest;
}
