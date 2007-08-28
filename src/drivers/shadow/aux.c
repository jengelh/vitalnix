/*
 *	shadow/aux.c
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2002 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/config.h>
#include "drivers/shadow/shadow.h"
#include <vitalnix/libvxpdb/libvxpdb.h>

long automatic_uid(struct shadow_state *state, long wanted)
{
	const struct HXdeque_node *travp = state->dq_user->first;
	long accept, high = PDB_NOUID, min, max;

	if (wanted == PDB_AUTOUID_SYS) {
		min = accept = 1;
		max = state->uid_min - 1;
	} else if (wanted == PDB_AUTOUID) {
		min = accept = state->uid_min;
		max = state->uid_max;
	} else {
		return wanted;
	}

	/* Find the highest ID */
	while (travp != NULL) {
		const struct vxpdb_user *user = travp->ptr;
		long uid = user->pw_uid;
		if (uid >= min && uid <= max && uid > high)
			high = uid;
		travp = travp->next;
	}

	/* If the successor id is not outside the boundaries, take it. */
	if (high >= 0 && high < max)
		return high + 1;

	/* No? Then just scan for the next */
	while (accept <= max) {
		bool used = false;
		travp = state->dq_user->first;
		while (travp != NULL) {
			const struct vxpdb_user *user = travp->ptr;
			if (user->pw_uid == accept) {
				used = 1;
				break;
			}
			travp = travp->next;
		}
		if (!used)
			return accept;
		++accept;
	}
	return -ENOENT;
}

long automatic_gid(struct shadow_state *state, long wanted)
{
	const struct HXdeque_node *travp = state->dq_group->first;
	long accept, high = PDB_NOGID, min, max;

	if (wanted == PDB_AUTOGID_SYS) {
		min = accept = 1;
		max = state->gid_min - 1;
	} else if (wanted == PDB_AUTOGID) {
		min = accept = state->gid_min;
		max = state->gid_max;
	} else {
		return wanted;
	}

	/* Find the highest GID */
	while (travp != NULL) {
		const struct vxpdb_group *group = travp->ptr;
		long gid = group->gr_gid;
		if (gid >= min && gid <= max && gid > high)
			high = gid;
		travp = travp->next;
	}

	/*
	 * If the highest id is below the max, there must be something free.
	 * Take the successor id.
	 */
	if (high >= 0 && high < max)
		return high + 1;

	/* No? Then just scan for the next */
	while (accept <= max) {
		bool used = false;
		travp = state->dq_group->first;
		while (travp != NULL) {
			const struct vxpdb_group *group = travp->ptr;
			if (group->gr_gid == accept) {
				used = true;
				break;
			}
			travp = travp->next;
		}
		if (!used)
			return accept;
		++accept;
	}
	return -ENOENT;
}

void free_data(struct shadow_state *state)
{
	struct HXdeque_node *travp;

	if (state->dq_user != NULL) {
		travp = state->dq_user->first;
		while (travp != NULL) {
			free_single_user(travp->ptr);
			travp = travp->next;
		}
		HXdeque_free(state->dq_user);
		state->dq_user = NULL;
	}

	if (state->dq_group != NULL) {
		travp = state->dq_group->first;
		while (travp != NULL) {
			free_single_group(travp->ptr);
			travp = travp->next;
		}
		HXdeque_free(state->dq_group);
		state->dq_group = NULL;
	}
	return;
}

void free_single_user(struct vxpdb_user *u)
{
	char **p = u->be_priv;

	free(u->pw_name);
	free(u->pw_real);
	free(u->pw_home);
	free(u->pw_shell);
	free(u->sp_passwd);

	if (p != NULL) {
		free(p[0]);
		free(p[1]);
		free(p);
	}

	free(u);
	return;
}

void free_single_group(struct vxpdb_group *g)
{
	char **p = g->be_priv;
	free(g->gr_name);
	if (p != NULL) {
		free(p[0]);
		free(p[1]);
		free(p);
	}
	free(g);
	return;
}

/*
 * lookup_group -
 * @dq:		list to search
 * @gname:	group name to search on, or %NULL if don't-care
 * @gid:	group id to search on, or %PDB_NOGID if don't-care
 *
 * Searches the group and returns a pointer to vxdb_shadow's internal data
 * structure if found, or %NULL if not found. If @gname is %NULL and @gid is
 * %PDB_NOGID, which would usually match any user, %NULL is returned too.
 */
struct vxpdb_group *lookup_group(struct HXdeque *dq,
    const char *gname, long gid)
{
	const struct HXdeque_node *travp = dq->first;

	if (gname == NULL && gid == PDB_NOGID)
		return NULL;

	while (travp != NULL) {
		struct vxpdb_group *g = travp->ptr;
		if ((gname == NULL || strcmp(g->gr_name, gname) == 0) &&
		    (gid == PDB_NOGID || g->gr_gid == gid))
			return g;
		travp = travp->next;
	}

	return NULL;
}

/*
 * lookup_user -
 * @dq:	list to search on
 * @lname:	user login name to match on, or %NULL if don't-care
 * @uid:	user uid to match o, or %PDB_NOUID if don't-care
 *
 * Searches the user and returns a pointer to vxdb_shadow's internal data
 * structure if found, or %NULL if not found. If @lname is %NULL and @uid is
 * %PDB_NOUID, which would usually match any user, %NULL is returned too.
 */
struct vxpdb_user *lookup_user(struct HXdeque *dq,
    const char *lname, long uid)
{
	const struct HXdeque_node *travp = dq->first;

	if (lname == NULL && uid == PDB_NOUID)
		return NULL;

	while (travp != NULL) {
		struct vxpdb_user *u = travp->ptr;
		if ((lname == NULL || strcmp(u->pw_name, lname) == 0) &&
		    (uid == PDB_NOUID || uid == u->pw_uid))
			return u;
		travp = travp->next;
	}

	return NULL;
}

void read_config(struct shadow_state *state, unsigned int action,
    const char *file)
{
	struct HXoption autouid_table[] = {
		{.ln = "UID_MIN", .type = HXTYPE_LONG, .ptr = &state->uid_min},
		{.ln = "UID_MAX", .type = HXTYPE_LONG, .ptr = &state->uid_max},
		{.ln = "GID_MIN", .type = HXTYPE_LONG, .ptr = &state->gid_min},
		{.ln = "GID_MAX", .type = HXTYPE_LONG, .ptr = &state->gid_max},
		HXOPT_TABLEEND,
	};
	struct HXoption options_table[] = {
		{.ln = "PASSWD_DB",   .type = HXTYPE_STRING, .ptr = &state->fpasswd.path},
		{.ln = "SHADOW_DB",   .type = HXTYPE_STRING, .ptr = &state->fshadow.path},
		{.ln = "VXPASSWD_DB", .type = HXTYPE_STRING, .ptr = &state->fvxpasswd.path},
		{.ln = "VXSHADOW_DB", .type = HXTYPE_STRING, .ptr = &state->fvxshadow.path},
		{.ln = "GROUP_DB",	.type = HXTYPE_STRING, .ptr = &state->fgroup.path},
		HXOPT_TABLEEND,
	};

	if (action == CONFIG_READ) {
		state->uid_min = state->gid_min = 1000;
		state->uid_max = state->gid_max = 60000;
		HX_shconfig(CONFIG_SYSCONFDIR "/autouid.conf", autouid_table);
		HX_shconfig(file, options_table);
	} else if (action == CONFIG_FREE) {
		HX_shconfig_free(options_table);
	}
	return;
}

struct HXdeque_node *skip_nis_users(struct HXdeque_node *node)
{
	const struct vxpdb_user *user;
	while (node != NULL) {
		user = node->ptr;
		if (*user->pw_name != '+' && *user->pw_name != '-')
			return node;
		node = node->next;
	}
	return node;
}

struct HXdeque_node *skip_nis_groups(struct HXdeque_node *node)
{
	const struct vxpdb_group *group;
	while (node != NULL) {
		group = node->ptr;
		if (*group->gr_name != '+' && *group->gr_name != '-')
			return node;
		node = node->next;
	}
	return node;
}

void truncate_here(FILE *fp)
{
	long p = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fflush(fp);
	ftruncate(fileno(fp), p);
	fflush(fp);
	fseek(fp, p, SEEK_SET);
	return;
}
