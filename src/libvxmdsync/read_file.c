/*
 *	libvxmdsync/read_file.c
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/arbtree.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include "libvxmdsync/internal.h" /* MAX_LNAME */
#include <vitalnix/libvxmdsync/libvxmdsync.h>
#include <vitalnix/libvxutil/libvxutil.h>

EXPORT_SYMBOL int mdsync_read_file(struct mdsync_workspace *w,
    const char *input_dsc, const char *input_fmt)
{
	struct vxeds_entry *entry;
	char username[MAX_LNAME+1];
	void *state;
	int ret;

	if ((ret = vxeds_open(input_dsc, input_fmt, &state)) <= 0)
		return ret;

	while (true) {
		entry = malloc(sizeof(struct vxeds_entry));

		if ((ret = vxeds_read(state, entry)) < 0) {
			if (ret == -EINVAL)
				fprintf(stderr, "%s: Warning: Stumbled upon a "
				        "bogus entry in Data Source, "
				        "stopping.\n", __func__);
			else
				fprintf(stderr, "%s+eds_read(): %s\n",
				        __func__, strerror(-ret));
			vxeds_free_entry(entry);
			free(entry);
			continue;
		} else if (ret == 0) {
			/* End of list */
			free(entry);
			break;
		}

		entry->username = HX_strdup(vxutil_propose_lname(username,
		                  sizeof(username), entry->surname,
		                  entry->first_name));
		HXbtree_add(w->add_req, entry->uuid, entry);
	}

	vxeds_close(state);
	return 1;
}
