/*
 *	shadow/aux.c
 *	Copyright © CC Computer Consultants GmbH, 2002 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
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
#include <vitalnix/libvxdb/libvxdb.h>

unsigned int automatic_uid(struct shadow_state *state, unsigned int wanted)
{
	const struct HXdeque_node *travp = state->dq_user->first;
	unsigned int accept, min, max;
	int high = -1;

	if (wanted == VXDB_AUTOUID_SYS) {
		min = accept = 1;
		max = state->uid_min - 1;
	} else if (wanted == VXDB_AUTOUID) {
		min = accept = state->uid_min;
		max = state->uid_max;
	} else {
		return wanted;
	}

	/* Find the highest ID */
	while (travp != NULL) {
		const struct vxdb_user *user = travp->ptr;
		unsigned int uid = user->pw_uid;
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
		for (travp = state->dq_user->first; travp != NULL;
		    travp = travp->next) {
			const struct vxdb_user *user = travp->ptr;
			if (user->pw_uid == accept) {
				used = true;
				break;
			}
		}
		if (!used)
			return accept;
		++accept;
	}
	return -ENOSPC;
}

unsigned int automatic_gid(struct shadow_state *state, unsigned int wanted)
{
	const struct HXdeque_node *travp = state->dq_group->first;
	unsigned int accept, min, max;
	int high = -1;

	if (wanted == VXDB_AUTOGID_SYS) {
		min = accept = 1;
		max = state->gid_min - 1;
	} else if (wanted == VXDB_AUTOGID) {
		min = accept = state->gid_min;
		max = state->gid_max;
	} else {
		return wanted;
	}

	/* Find the highest GID */
	while (travp != NULL) {
		const struct vxdb_group *group = travp->ptr;
		unsigned int gid = group->gr_gid;
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
		for (travp = state->dq_group->first; travp != NULL;
		    travp = travp->next) {
			const struct vxdb_group *group = travp->ptr;
			if (group->gr_gid == accept) {
				used = true;
				break;
			}
		}
		if (!used)
			return accept;
		++accept;
	}
	return -ENOSPC;
}

void free_data(struct shadow_state *state)
{
	struct HXdeque_node *travp;

	if (state->dq_user != NULL) {
		for (travp = state->dq_user->first; travp != NULL;
		    travp = travp->next)
			free_single_user(travp->ptr);
		HXdeque_free(state->dq_user);
		state->dq_user = NULL;
	}

	if (state->dq_group != NULL) {
		for (travp = state->dq_group->first; travp != NULL;
		    travp = travp->next)
			free_single_group(travp->ptr);
		HXdeque_free(state->dq_group);
		state->dq_group = NULL;
	}
	return;
}

void free_single_user(struct vxdb_user *u)
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

void free_single_group(struct vxdb_group *g)
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
 * @gid:	group id to search on, or %VXDB_NOGID if don't-care
 *
 * Searches the group and returns a pointer to vxdb_shadow's internal data
 * structure if found, or %NULL if not found. If @gname is %NULL and @gid is
 * %VXDB_NOGID, which would usually match any user, %NULL is returned too.
 */
struct vxdb_group *lookup_group(const struct HXdeque *dq,
    const char *gname, unsigned int gid)
{
	const struct HXdeque_node *travp;

	if (gname == NULL && gid == VXDB_NOGID)
		return NULL;

	for (travp = dq->first; travp != NULL; travp = travp->next) {
		struct vxdb_group *g = travp->ptr;
		if ((gname == NULL || strcmp(g->gr_name, gname) == 0) &&
		    (gid == VXDB_NOGID || g->gr_gid == gid))
			return g;
	}

	return NULL;
}

/*
 * lookup_user -
 * @dq:	list to search on
 * @lname:	user login name to match on, or %NULL if don't-care
 * @uid:	user uid to match o, or %VXDB_NOUID if don't-care
 *
 * Searches the user and returns a pointer to vxdb_shadow's internal data
 * structure if found, or %NULL if not found. If @lname is %NULL and @uid is
 * %VXDB_NOUID, which would usually match any user, %NULL is returned too.
 */
struct vxdb_user *lookup_user(const struct HXdeque *dq,
    const char *lname, unsigned int uid)
{
	const struct HXdeque_node *travp;

	if (lname == NULL && uid == VXDB_NOUID)
		return NULL;

	for (travp = dq->first; travp != NULL; travp = travp->next) {
		struct vxdb_user *u = travp->ptr;
		if ((lname == NULL || strcmp(u->pw_name, lname) == 0) &&
		    (uid == VXDB_NOUID || uid == u->pw_uid))
			return u;
	}

	return NULL;
}

void read_config(struct shadow_state *state, unsigned int action,
    const char *file)
{
	struct HXoption autouid_table[] = {
		{.ln = "UID_MIN", .type = HXTYPE_UINT, .ptr = &state->uid_min},
		{.ln = "UID_MAX", .type = HXTYPE_UINT, .ptr = &state->uid_max},
		{.ln = "GID_MIN", .type = HXTYPE_UINT, .ptr = &state->gid_min},
		{.ln = "GID_MAX", .type = HXTYPE_UINT, .ptr = &state->gid_max},
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
	const struct vxdb_user *user;
	for (; node != NULL; node = node->next) {
		user = node->ptr;
		if (*user->pw_name != '+' && *user->pw_name != '-')
			return node;
	}
	return node;
}

struct HXdeque_node *skip_nis_groups(struct HXdeque_node *node)
{
	const struct vxdb_group *group;
	for (; node != NULL; node = node->next) {
		group = node->ptr;
		if (*group->gr_name != '+' && *group->gr_name != '-')
			return node;
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
