/*
 *	nss.c - Example NSS back-end
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
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
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxdb/libvxdb.h>

/* Definitions */
enum {
	IDF_NORMAL = 0,
	IDF_SYSTEM,
};

struct nss_state {
	struct HXdeque *dq_user, *dq_group;
};

struct traverser_state {
	struct HXdeque_node *wp;
};

/* Functions */
static void vxnss_close(struct vxdb_state *);
static int db_open(struct nss_state *);
static struct HXdeque *db_read_groups(void);
static struct HXdeque *db_read_passwd(void);
static struct HXdeque *db_read_shadow(struct HXdeque *);
static void free_data(struct nss_state *);
static inline void free_single_group(struct vxdb_group *);
static inline void free_single_user(struct vxdb_user *);
static inline struct vxdb_user *get_user(const struct HXdeque *, const char *);
static inline struct vxdb_group *get_group(const struct HXdeque *, const char *);

//-----------------------------------------------------------------------------
static int vxnss_init(struct vxdb_state *vp, const char *config_file)
{
	if ((vp->state = calloc(1, sizeof(struct nss_state))) == NULL)
		return -errno;
	return 1;
}

static int vxnss_open(struct vxdb_state *vp, unsigned int flags)
{
	struct nss_state *state = vp->state;
	int ret;

	if ((ret = db_open(state)) <= 0) {
		vxnss_close(vp);
		return ret;
	}

	return 1;
}

static void vxnss_close(struct vxdb_state *vp)
{
	free_data(vp->state);
}

static void vxnss_exit(struct vxdb_state *vp)
{
	free(vp->state);
}

static long vxnss_modctl(struct vxdb_state *vp, unsigned int command, ...)
{
	struct nss_state *state = vp->state;

	errno = 0;
	switch (command) {
		case VXDB_COUNT_USERS:
			return state->dq_user->items;
		case VXDB_COUNT_GROUPS:
			return state->dq_group->items;
	}

	return -ENOSYS;
}

static int vxnss_getpwuid(struct vxdb_state *vp, unsigned int uid,
    struct vxdb_user *dest)
{
	const struct nss_state *state = vp->state;
	const struct HXdeque_node *trav;
	const struct vxdb_user *src;

	for (trav = state->dq_user->first; trav != NULL; trav = trav->next) {
		src = trav->ptr;
		if (src->pw_uid != uid)
			continue;
		if (dest != NULL)
			vxdb_user_copy(dest, src);
		return 1;
	}

	return 0;
}

static int vxnss_getpwnam(struct vxdb_state *vp, const char *name,
    struct vxdb_user *dest)
{
	const struct nss_state *state = vp->state;
	const struct vxdb_user *src;

	src = get_user(state->dq_user, name);
	if (src != NULL) {
		if (dest != NULL)
			vxdb_user_copy(dest, src);
		return 1;
	}

	return 0;
}

static void *vxnss_usertrav_init(struct vxdb_state *vp)
{
	struct nss_state *state = vp->state;
	struct traverser_state trav;

	trav.wp = state->dq_user->first;
	return HX_memdup(&trav, sizeof(trav));
}

static int vxnss_usertrav_walk(struct vxdb_state *vp, void *priv_data,
    struct vxdb_user *dest)
{
	struct traverser_state *trav = priv_data;

	if (trav->wp == NULL)
		return 0;
	vxdb_user_copy(dest, trav->wp->ptr);
	trav->wp = trav->wp->next;
	return 1;
}

static void vxnss_usertrav_free(struct vxdb_state *vp, void *priv_data)
{
	free(priv_data);
}

static int vxnss_getgrgid(struct vxdb_state *vp, unsigned int gid,
    struct vxdb_group *dest)
{
	const struct nss_state *state = vp->state;
	const struct HXdeque_node *trav;
	const struct vxdb_group *src;

	for (trav = state->dq_group->first; trav != NULL; trav = trav->next) {
		src = trav->ptr;
		if (src->gr_gid != gid)
			continue;
		if (dest != NULL)
			vxdb_group_copy(dest, src);
		return 1;
	}

	return 0;
}

static int vxnss_getgrnam(struct vxdb_state *vp, const char *name,
    struct vxdb_group *dest)
{
	const struct nss_state *state = vp->state;
	const struct vxdb_group *src;

	src = get_group(state->dq_group, name);
	if (src != NULL) {
		if (dest != NULL)
			vxdb_group_copy(dest, src);
		return 1;
	}

	return 0;
}

static void *vxnss_grouptrav_init(struct vxdb_state *vp)
{
	const struct nss_state *state = vp->state;
	struct traverser_state trav;

	trav.wp = state->dq_group->first;
	return HX_memdup(&trav, sizeof(trav));
}

static int vxnss_grouptrav_walk(struct vxdb_state *vp, void *priv_data,
    struct vxdb_group *dest)
{
	struct traverser_state *trav = priv_data;

	if (trav->wp == NULL)
		return 0;
	vxdb_group_copy(dest, trav->wp->ptr);
	trav->wp = trav->wp->next;
	return 1;
}

static void vxnss_grouptrav_free(struct vxdb_state *vp, void *priv_data)
{
	free(priv_data);
}

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
	struct vxdb_user *u;
	int ret;

	if ((dq = HXdeque_init()) == NULL)
		return NULL;

	setpwent();
	while ((ret = getpwent_r(&nss_passwd, nss_buffer,
	    sizeof(nss_buffer), &nss_ptr)) == 0 && nss_ptr != NULL)
	{
		if (*nss_ptr->pw_name == '+' || *nss_ptr->pw_name == '-')
			++nss_ptr->pw_name;
		if ((u = malloc(sizeof(struct vxdb_user))) == NULL)
			break;

		vxdb_user_clean(u);
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
	struct vxdb_user *u;

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
	struct vxdb_group *g;
	struct HXdeque *dq;

	if ((dq = HXdeque_init()) == NULL)
		return NULL;

	setgrent();
	while (getgrent_r(&nss_group, nss_buffer,
	    sizeof(nss_buffer), &nss_gptr) == 0 && nss_gptr != NULL)
	{
		if (*nss_gptr->gr_name == '+' || *nss_gptr->gr_name == '-')
			++nss_gptr->gr_name;
		if ((g = malloc(sizeof(struct vxdb_group))) == NULL)
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
}

static inline void free_single_user(struct vxdb_user *u)
{
	free(u->pw_name);
	free(u->pw_real);
	free(u->pw_home);
	free(u->pw_shell);
	free(u->sp_passwd);
	free(u);
}

static inline void free_single_group(struct vxdb_group *g)
{
	free(g->gr_name);
	free(g);
}

static inline struct vxdb_user *get_user(const struct HXdeque *dq,
    const char *lname)
{
	const struct HXdeque_node *travp;

	for (travp = dq->first; travp != NULL; travp = travp->next) {
		struct vxdb_user *u = travp->ptr;
		if (strcmp(u->pw_name, lname) == 0)
			return u;
	}

	return NULL;
}

static inline struct vxdb_group *get_group(const struct HXdeque *dq,
    const char *name)
{
	const struct HXdeque_node *travp;

	for (travp = dq->first; travp != NULL; travp = travp->next) {
		struct vxdb_group *g = travp->ptr;
		if (strcmp(g->gr_name, name) == 0)
			return g;
	}

	return NULL;
}

EXPORT_SYMBOL struct vxdb_driver THIS_MODULE = {
	.name           = "NSS back-end module (not MU/MT-safe)",
	.desc           = "API demonstration",
	.init           = vxnss_init,
	.open           = vxnss_open,
	.close          = vxnss_close,
	.exit           = vxnss_exit,
	.modctl         = vxnss_modctl,
	.getpwuid       = vxnss_getpwuid,
	.getpwnam       = vxnss_getpwnam,
	.usertrav_init  = vxnss_usertrav_init,
	.usertrav_walk  = vxnss_usertrav_walk,
	.usertrav_free  = vxnss_usertrav_free,
	.getgrgid       = vxnss_getgrgid,
	.getgrnam       = vxnss_getgrnam,
	.grouptrav_init = vxnss_grouptrav_init,
	.grouptrav_walk = vxnss_grouptrav_walk,
	.grouptrav_free = vxnss_grouptrav_free,
};
