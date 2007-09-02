/*
 *	shadow/fsgroup.c - group file handling
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2002 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include "drivers/shadow/shadow.h"
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/defines.h>

struct HXdeque *db_read_groups(FILE *fp)
{
	struct vxpdb_group *g;
	struct HXdeque *dq;
	char *ln = NULL;

	if ((dq = HXdeque_init()) == NULL)
		return NULL;

	while (HX_getl(&ln, fp) != NULL) {
		char *data[4];

		if (*ln == '#' || *ln == '+' || *ln == '-')
			continue;

		HX_chomp(ln);
		memset(data, 0, sizeof(data));
		if (HX_split5(ln, ":", ARRAY_SIZE(data), data) < 3) {
			fprintf(stderr, "vxdb_shadow: bogus line in "
			        "group file\n");
			continue;
		}

		if ((g = malloc(sizeof(struct vxpdb_group))) == NULL)
			return NULL;

		g->gr_name = HX_strdup(data[0]);
		g->gr_gid  = strtol(data[2], NULL, 0);
		g->be_priv = calloc(1, sizeof(void *) * 2);
		static_cast(char **, g->be_priv)[0] = HX_strdup(data[3]);
		HXdeque_push(dq, g);
	}

	hmc_free(ln);
	return dq;
}

void db_flush_groups(struct shadow_state *state)
{
	const struct HXdeque_node *travp = state->dq_group->first;
	FILE *fp = state->fgroup.fp;

	fseek(state->fgroup.fp, 0, SEEK_SET);

	while (travp != NULL) {
		const struct vxpdb_group *g = travp->ptr;
		const char **pr = g->be_priv;

		fprintf(fp, "%s:x:%u:", g->gr_name, g->gr_gid);
		if (pr != NULL && pr[0] != NULL)
			fprintf(fp, "%s", pr[0]);
		fprintf(fp, "\n");
		travp = travp->next;
	}

	truncate_here(state->fgroup.fp);
	TOUCH_GROUP_TAG(false);
	return;
}
