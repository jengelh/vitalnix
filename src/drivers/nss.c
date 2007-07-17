/*
 *	nss.c - Example NSS back-end
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2005 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#define _GNU_SOURCE 1
#define _POSIX_SOURCE 1
#define _XOPEN_SOURCE 1
#include <features.h>

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <grp.h>
#include <pwd.h>
#include <shadow.h>
#include <vitalnix/config.h>
#include "drivers/proto.h"
#include "drivers/static-build.h"
#include <vitalnix/libvxpdb/libvxpdb.h>

/* Definitions */
enum {
	IDF_NORMAL = 0,
	IDF_SYSTEM,
};

struct nss_state {
	struct HXdeque *dq_user, *dq_group;
	long uid_min, uid_max, gid_min, gid_max;
};

struct traverser_state {
	struct HXdeque_node *wp;
};

/* Functions */
static void vnss_close(struct vxpdb_state *);
static int db_open(struct nss_state *);
static struct HXdeque *db_read_groups(void);
static struct HXdeque *db_read_passwd(void);
static struct HXdeque *db_read_shadow(struct HXdeque *);
static void free_data(struct nss_state *);
static inline void free_single_group(struct vxpdb_group *);
static inline void free_single_user(struct vxpdb_user *);
static inline struct vxpdb_user *get_user(struct HXdeque *, const char *);
static inline struct vxpdb_group *get_group(struct HXdeque *, const char *);

//-----------------------------------------------------------------------------
static int vnss_init(struct vxpdb_state *vp, const char *config_file)
{
	if ((vp->state = calloc(1, sizeof(struct nss_state))) == NULL)
		return -errno;
	return 1;
}

static int vnss_open(struct vxpdb_state *vp, long flags)
{
	struct nss_state *state = vp->state;
	int ret;

	if ((ret = db_open(state)) <= 0) {
		vnss_close(vp);
		return ret;
	}

	return 1;
}

static void vnss_close(struct vxpdb_state *vp)
{
	free_data(vp->state);
	return;
}

static void vnss_exit(struct vxpdb_state *vp)
{
	free(vp->state);
	return;
}

static long vnss_modctl(struct vxpdb_state *vp, long command, ...)
{
	struct nss_state *state = vp->state;

	errno = 0;
	switch (command) {
		case PDB_COUNT_USERS:
			return state->dq_user->items;
		case PDB_COUNT_GROUPS:
			return state->dq_group->items;
	}

	return -ENOSYS;
}

//-----------------------------------------------------------------------------
static int vnss_userinfo(struct vxpdb_state *vp,
    const struct vxpdb_user *sr_mask, struct vxpdb_user *dest, size_t size)
{
	struct nss_state *state = vp->state;
	const struct HXdeque_node *travp;
	struct vxpdb_user temp_mask;
	int found = 0;

	if (sr_mask == dest) {
		memcpy(&temp_mask, sr_mask, sizeof(struct vxpdb_user));
		sr_mask = &temp_mask;
	}

	for (travp = state->dq_user->first;
	    travp != NULL && (dest == NULL || size > 0); travp = travp->next)
	{
		const struct vxpdb_user *src = travp->ptr;
		if (!vxpdb_user_match(src, sr_mask))
			continue;
		if (dest != NULL) {
			vxpdb_user_copy(dest, src);
			++dest;
			++found;
			--size;
		} else {
			if (size == 0)
				return 1;
			++found;
		}
	}

	return found;
}

static void *vnss_usertrav_init(struct vxpdb_state *vp)
{
	struct nss_state *state = vp->state;
	struct traverser_state trav;

	trav.wp = state->dq_user->first;
	return HX_memdup(&trav, sizeof(trav));
}

static int vnss_usertrav_walk(struct vxpdb_state *vp, void *priv_data,
    struct vxpdb_user *dest)
{
	struct traverser_state *trav = priv_data;

	if (trav->wp == NULL)
		return 0;
	vxpdb_user_copy(dest, trav->wp->ptr);
	trav->wp = trav->wp->next;
	return 1;
}

static void vnss_usertrav_free(struct vxpdb_state *vp, void *priv_data)
{
	free(priv_data);
	return;
}

//-----------------------------------------------------------------------------
static int vnss_groupinfo(struct vxpdb_state *vp,
    const struct vxpdb_group *sr_mask, struct vxpdb_group *dest, size_t size)
{
	struct nss_state *state = vp->state;
	const struct HXdeque_node *travp;
	struct vxpdb_group temp_mask;
	int found = 0;

	if (sr_mask == dest) {
		memcpy(&temp_mask, sr_mask, sizeof(struct vxpdb_group));
		sr_mask = &temp_mask;
	}

	for (travp = state->dq_group->first;
	    travp != NULL && (dest == NULL || size > 0); travp = travp->next)
	{
		const struct vxpdb_group *src = travp->ptr;
		if (!vxpdb_group_match(src, sr_mask))
			continue;
		if (dest != NULL) {
			vxpdb_group_copy(dest, src);
			++dest;
			++found;
			--size;
		} else {
			if (size == 0)
				return 1;
			++found;
		}
	}

	return found;
}

static void *vnss_grouptrav_init(struct vxpdb_state *vp)
{
	struct nss_state *state = vp->state;
	struct traverser_state trav;

	trav.wp = state->dq_group->first;
	return HX_memdup(&trav, sizeof(trav));
}

static int vnss_grouptrav_walk(struct vxpdb_state *vp, void *priv_data,
    struct vxpdb_group *dest)
{
	struct traverser_state *trav = priv_data;

	if (trav->wp == NULL)
		return 0;
	vxpdb_group_copy(dest, trav->wp->ptr);
	trav->wp = trav->wp->next;
	return 1;
}

static void vnss_grouptrav_free(struct vxpdb_state *vp, void *priv_data)
{
	free(priv_data);
	return;
}

//-----------------------------------------------------------------------------
static int db_open(struct nss_state *state)
{
	state->dq_user  = db_read_passwd();
	db_read_shadow(state->dq_user);
	state->dq_group = db_read_groups();
	return 1;
}

static struct HXdeque *db_read_passwd(void)
{
	struct passwd nss_passwd, *nss_ptr;
	char nss_buffer[1024];
	struct HXdeque *dq;
	struct vxpdb_user *u;
	int ret;

	if ((dq = HXdeque_init()) == NULL)
		return NULL;

	setpwent();
	while ((ret = getpwent_r(&nss_passwd, nss_buffer,
	    sizeof(nss_buffer), &nss_ptr)) == 0 && nss_ptr != NULL)
	{
		if (*nss_ptr->pw_name == '+' || *nss_ptr->pw_name == '-')
			++nss_ptr->pw_name;
		if ((u = malloc(sizeof(struct vxpdb_user))) == NULL)
			break;

		vxpdb_user_clean(u);
		u->pw_name  = HX_strdup(nss_ptr->pw_name);
		u->pw_uid   = nss_ptr->pw_uid;
		u->pw_gid   = nss_ptr->pw_gid;
		u->pw_real  = HX_strdup(nss_ptr->pw_gecos);
		u->pw_home  = HX_strdup(nss_ptr->pw_dir);
		u->pw_shell = HX_strdup(nss_ptr->pw_shell);
		HXdeque_push(dq, u);
	}

	endpwent();
	return dq;
}

static struct HXdeque *db_read_shadow(struct HXdeque *dq)
{
	struct spwd nss_shadow, *nss_sptr;
	char nss_buffer[1024];
	struct vxpdb_user *u;

	if (dq->items == 0)
		return dq;

	setspent();
	while (getspent_r(&nss_shadow, nss_buffer,
	    sizeof(nss_buffer), &nss_sptr) == 0 && nss_sptr != NULL)
	{
		if (*nss_sptr->sp_namp == '+' || *nss_sptr->sp_namp == '-')
			++nss_sptr->sp_namp;
		if ((u = get_user(dq, nss_sptr->sp_namp)) == NULL)
			continue;
		u->sp_passwd  = HX_strdup(nss_sptr->sp_pwdp);
		u->sp_lastchg = nss_sptr->sp_lstchg;
		u->sp_min     = nss_sptr->sp_min;
		u->sp_max     = nss_sptr->sp_max;
		u->sp_warn    = nss_sptr->sp_warn;
		u->sp_expire  = nss_sptr->sp_expire;
		u->sp_inact   = nss_sptr->sp_inact;
	}

	endspent();
	return dq;
}

static struct HXdeque *db_read_groups(void)
{
	struct group nss_group, *nss_gptr;
	char nss_buffer[1024];
	struct vxpdb_group *g;
	struct HXdeque *dq;

	if ((dq = HXdeque_init()) == NULL)
		return NULL;

	setgrent();
	while (getgrent_r(&nss_group, nss_buffer,
	    sizeof(nss_buffer), &nss_gptr) == 0 && nss_gptr != NULL)
	{
		if (*nss_gptr->gr_name == '+' || *nss_gptr->gr_name == '-')
			++nss_gptr->gr_name;
		if ((g = malloc(sizeof(struct vxpdb_group))) == NULL)
			break;
		g->gr_name = HX_strdup(nss_gptr->gr_name);
		g->gr_gid  = nss_gptr->gr_gid;
		HXdeque_push(dq, g);
	}

	endgrent();
	return dq;
}

static void free_data(struct nss_state *state)
{
	struct HXdeque_node *travp;

	if (state->dq_user != NULL) {
		for (travp = state->dq_user->first;
		    travp != NULL; travp = travp->next)
			free_single_user(travp->ptr);

		HXdeque_free(state->dq_user);
		state->dq_user = NULL;
	}

	if (state->dq_group != NULL) {
		for (travp = state->dq_group->first;
		    travp != NULL; travp = travp->next)
			free_single_group(travp->ptr);

		HXdeque_free(state->dq_group);
		state->dq_group = NULL;
	}
	return;
}

static inline void free_single_user(struct vxpdb_user *u)
{
	free(u->pw_name);
	free(u->pw_real);
	free(u->pw_home);
	free(u->pw_shell);
	free(u->sp_passwd);
	free(u);
	return;
}

static inline void free_single_group(struct vxpdb_group *g)
{
	free(g->gr_name);
	free(g);
	return;
}

static inline struct vxpdb_user *get_user(struct HXdeque *dq,
    const char *lname)
{
	const struct HXdeque_node *travp;

	for (travp = dq->first; travp != NULL; travp = travp->next) {
		struct vxpdb_user *u = travp->ptr;
		if (strcmp(u->pw_name, lname) == 0)
			return u;
	}

	return NULL;
}

static inline struct vxpdb_group *get_group(struct HXdeque *dq,
    const char *name)
{
	const struct HXdeque_node *travp;

	for (travp = dq->first; travp != NULL; travp = travp->next) {
		struct vxpdb_group *g = travp->ptr;
		if (strcmp(g->gr_name, name) == 0)
			return g;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
static struct vxpdb_driver THIS_MODULE = {
	.name           = "NSS back-end module (not MU/MT-safe)",
	.desc           = "API demonstration",

	DRIVER_CB_BASE1(vnss),
	.modctl         = vnss_modctl,

	.userinfo       = vnss_userinfo,
	.usertrav_init  = vnss_usertrav_init,
	.usertrav_walk  = vnss_usertrav_walk,
	.usertrav_free  = vnss_usertrav_free,

	.groupinfo      = vnss_groupinfo,
	.grouptrav_init = vnss_grouptrav_init,
	.grouptrav_walk = vnss_grouptrav_walk,
	.grouptrav_free = vnss_grouptrav_free,
};

REGISTER_MODULE(nss, &THIS_MODULE);

//=============================================================================
