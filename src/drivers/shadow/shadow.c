/*
 *	shadow/shadow.c - Shadow back-end (local files)
 *	Copyright © CC Computer Consultants GmbH, 2002 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/file.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include "drivers/shadow/shadow.h"
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

#define RWLOCKED(z)     ((z)->flags & PDB_WRLOCK)
#define RWLOCK_CHK(z)   if (!RWLOCKED(z)) return -EPERM;

/* Definitions */
enum {
	IDF_NORMAL = 0,
	IDF_SYSTEM,
};

struct traverser_state {
	struct HXdeque_node *wp;
};

/* Functions */
static void vshadow_close(struct vxpdb_state *);

/* Variables */
static char *Path_passwd   = "/etc/passwd";
static char *Path_shadow   = "/etc/shadow";
static char *Path_group    = "/etc/group";
static char *Path_vxpasswd = "/etc/vxpasswd";
static char *Path_vxshadow = "/etc/vxshadow";

//-----------------------------------------------------------------------------
static int vshadow_init(struct vxpdb_state *vp, const char *config_file)
{
	struct shadow_state *state;

	state = vp->state = calloc(1, sizeof(struct shadow_state));
	if (state == NULL)
		return -errno;

	state->fpasswd.path   = HX_strdup(Path_passwd);
	state->fshadow.path   = HX_strdup(Path_shadow);
	state->fgroup.path    = HX_strdup(Path_group);
	state->fvxpasswd.path = HX_strdup(Path_vxpasswd);
	state->fvxshadow.path = HX_strdup(Path_vxshadow);
	state->fpasswd.fd     = -1;
	state->fshadow.fd     = -1;
	state->fgroup.fd      = -1;
	state->fvxpasswd.fd   = -1;
	state->fvxshadow.fd   = -1;
	read_config(state, CONFIG_READ, config_file);
	return 1;
}

static int vshadow_open(struct vxpdb_state *vp, unsigned int flags)
{
	struct shadow_state *state = vp->state;
	int ret;

	if ((ret = db_open(state, flags & PDB_WRLOCK)) <= 0) {
		vshadow_close(vp);
		return ret;
	}
	return 1;
}

static void vshadow_close(struct vxpdb_state *vp)
{
	struct shadow_state *state = vp->state;
	db_close(state);
	free_data(state);
	return;
}

static void vshadow_exit(struct vxpdb_state *vp)
{
	struct shadow_state *state = vp->state;
	read_config(state, CONFIG_FREE, NULL);
	free(state);
	return;
}

static long vshadow_modctl(struct vxpdb_state *vp, unsigned int command, ...)
{
	struct shadow_state *state = vp->state;
	errno = 0;

	switch (command) {
		case PDB_FLUSH:
			if (!(state->flags & PDB_WRLOCK))
				return -EPERM;
			db_flush(state, true);
			return 1;
		case PDB_COUNT_USERS:
			return state->dq_user->items;
		case PDB_COUNT_GROUPS:
			return state->dq_group->items;
		case PDB_NEXTUID_SYS:
			return automatic_uid(state, PDB_AUTOUID_SYS);
		case PDB_NEXTUID:
			return automatic_uid(state, PDB_AUTOUID);
		case PDB_NEXTGID_SYS:
			return automatic_gid(state, PDB_AUTOGID_SYS);
		case PDB_NEXTGID:
			return automatic_gid(state, PDB_AUTOGID);
	}

	return -ENOSYS;
}

static int vshadow_lock(struct vxpdb_state *vp)
{
	struct shadow_state *state = vp->state;
	if (RWLOCKED(state))
		return 1;
	db_close(state);
	free_data(state);
	return db_open(state, state->flags | PDB_WRLOCK);
}

static int vshadow_unlock(struct vxpdb_state *vp)
{
	struct shadow_state *state = vp->state;
	struct flock lockinfo = {
		.l_type   = F_UNLCK,
		.l_whence = SEEK_SET,
		.l_start  = 0,
		.l_len    = 0,
	};
	if (!RWLOCKED(state))
		return 1;
	if (fcntl(state->fpasswd.fd, F_SETLK, &lockinfo) != 0)
		return -errno;
	state->flags &= ~PDB_WRLOCK;
	return 1;
}

static int vshadow_useradd(struct vxpdb_state *vp,
    const struct vxpdb_user *rq)
{
	struct shadow_state *state = vp->state;
	struct vxpdb_user *nu;
	unsigned int uid;

	if (rq->pw_name == NULL)
		return -EINVAL;
	if (lookup_user(state->dq_user, rq->pw_name, PDB_NOUID) != NULL)
		return -EEXIST;

	RWLOCK_CHK(state);

	uid = automatic_uid(state, rq->pw_uid);
	if (uid == -ENOSPC)
		return -ENOSPC;

	/*
	 * We have to be careful with strings here, as we need to duplicate
	 * them for us, leave the original ones intact and not have a single
	 * foreign pointer in our structs.
	 */
	if ((nu = calloc(1, sizeof(struct vxpdb_user))) == NULL)
		return -errno;

	nu->pw_name    = HX_strdup(rq->pw_name);
	nu->pw_uid     = uid;
	nu->pw_gid     = rq->pw_gid;
	nu->pw_real    = HX_strdup(rq->pw_real);
	nu->pw_home    = HX_strdup(rq->pw_home);
	nu->pw_shell   = HX_strdup(rq->pw_shell);

	nu->sp_passwd  = HX_strdup(rq->sp_passwd);
	nu->sp_lastchg = rq->sp_lastchg;
	nu->sp_min     = rq->sp_min;
	nu->sp_max     = rq->sp_max;
	nu->sp_warn    = rq->sp_warn;
	nu->sp_expire  = rq->sp_expire;
	nu->sp_inact   = rq->sp_inact;

	nu->vs_uuid    = HX_strdup(rq->vs_uuid);
	nu->vs_pvgrp   = HX_strdup(rq->vs_pvgrp);

	HXdeque_push(state->dq_user, nu);
	TOUCH_USER_TAG(true);
	db_flush(state, false);
	return 1;
}

static int vshadow_usermod(struct vxpdb_state *vp, const char *name,
    const struct vxpdb_user *mod_mask)
{
#define UP_INT(__field) \
	if (mod_mask->__field != PDB_NO_CHANGE) \
		user->__field = mod_mask->__field;
#define UP_STR(__field) \
	if (mod_mask->__field != NULL) \
		HX_strclone(&user->__field, mod_mask->__field);

	struct shadow_state *state = vp->state;
	struct vxpdb_user *user;

	RWLOCK_CHK(state);

	if (name == NULL)
		return -EINVAL;
	user = lookup_user(state->dq_user, name, PDB_NOUID);
	if (user == NULL)
		return -ENOENT;

	UP_STR(pw_name);
	UP_INT(pw_uid);
	UP_INT(pw_gid);
	UP_STR(pw_real);
	UP_STR(pw_home);
	UP_STR(pw_shell);

	UP_STR(sp_passwd);
	UP_INT(sp_lastchg);
	UP_INT(sp_min);
	UP_INT(sp_max);
	UP_INT(sp_warn);
	UP_INT(sp_expire);
	UP_INT(sp_inact);

	UP_STR(vs_uuid);
	UP_STR(vs_pvgrp);
	UP_INT(vs_defer);

	TOUCH_USER_TAG(true);
	db_flush(state, false);
	return 1;
#undef UP_INT
#undef UP_STR
}

static int vshadow_userdel(struct vxpdb_state *vp, const char *name)
{
	struct shadow_state *state = vp->state;
	struct vxpdb_user *user;

	RWLOCK_CHK(state);

	if (name == NULL)
		return -EINVAL;
	user = lookup_user(state->dq_user, name, PDB_NOUID);
	if (user == NULL)
		return -ENOENT;

	HXdeque_del(HXdeque_find(state->dq_user, user));
	free_single_user(user);
	TOUCH_USER_TAG(true);
	db_flush(state, false);
	return 1;
}

static int vshadow_getpwuid(struct vxpdb_state *vp, unsigned int uid,
    struct vxpdb_user *dest)
{
	const struct shadow_state *state = vp->state;
	const struct vxpdb_user *user;

	if ((user = lookup_user(state->dq_user, NULL, uid)) == NULL)
		return 0;
	if (dest != NULL)
		vxpdb_user_copy(dest, user);
	return 1;
}

static int vshadow_getpwnam(struct vxpdb_state *vp, const char *name,
    struct vxpdb_user *dest)
{
	const struct shadow_state *state = vp->state;
	const struct vxpdb_user *user;

	if ((user = lookup_user(state->dq_user, name, PDB_NOUID)) == NULL)
		return 0;
	if (dest != NULL)
		vxpdb_user_copy(dest, user);
	return 1;
}

static void *vshadow_usertrav_init(struct vxpdb_state *vp)
{
	struct shadow_state *state = vp->state;
	struct traverser_state trav;

	trav.wp = skip_nis_users(state->dq_user->first);
	return HX_memdup(&trav, sizeof(trav));
}

static int vshadow_usertrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_user *dest)
{
	struct traverser_state *trav = ptr;
	if (trav->wp == NULL)
		return 0;
	vxpdb_user_copy(dest, trav->wp->ptr);
	trav->wp = skip_nis_users(trav->wp->next);
	return 1;
}

static void vshadow_usertrav_free(struct vxpdb_state *vp, void *ptr)
{
	free(ptr);
	return;
}

static int vshadow_groupadd(struct vxpdb_state *vp,
    const struct vxpdb_group *rq)
{
	struct shadow_state *state = vp->state;
	struct vxpdb_group *ng;
	unsigned int gid;

	if (rq->gr_name == NULL)
		return -EINVAL;
	if (lookup_group(state->dq_group, rq->gr_name, PDB_NOGID) == NULL)
		return -ENOENT;

	RWLOCK_CHK(state);

	gid = automatic_gid(state, rq->gr_gid);
	if (gid == -ENOSPC)
		return -ENOSPC;

	if ((ng = calloc(1, sizeof(struct vxpdb_group))) == NULL)
		return -errno;

	ng->gr_name = HX_strdup(rq->gr_name);
	ng->gr_gid  = gid;
	HXdeque_push(state->dq_group, ng);
	TOUCH_GROUP_TAG(true);
	db_flush(state, false);
	return 1;
}

static int vshadow_groupmod(struct vxpdb_state *vp, const char *name,
    const struct vxpdb_group *mod_mask)
{
	struct shadow_state *state = vp->state;
	struct vxpdb_group *group;

	RWLOCK_CHK(state);
	group = lookup_group(state->dq_group, name, PDB_NOGID);
	if (group == NULL)
		return -ENOENT;

	if (mod_mask->gr_name != NULL)
		HX_strclone(&group->gr_name, mod_mask->gr_name);
	if (mod_mask->gr_gid != PDB_NOGID)
		group->gr_gid = mod_mask->gr_gid;

	TOUCH_GROUP_TAG(true);
	db_flush(state, false);
	return 1;
}

static int vshadow_groupdel(struct vxpdb_state *vp, const char *name)
{
	struct shadow_state *state = vp->state;
	struct vxpdb_group *grp;

	RWLOCK_CHK(state);
	grp = lookup_group(state->dq_group, name, PDB_NOGID);
	if (grp == NULL)
		return -ENOENT;

	HXdeque_del(HXdeque_find(state->dq_group, grp));
	free_single_group(grp);
	TOUCH_GROUP_TAG(true);
	db_flush(state, false);
	return 1;
}

static int vshadow_getgrgid(struct vxpdb_state *vp, unsigned int gid,
    struct vxpdb_group *dest)
{
	const struct shadow_state *state = vp->state;
	const struct vxpdb_group *group;

	if ((group = lookup_group(state->dq_group, NULL, gid)) == NULL)
		return 0;
	if (dest != NULL)
		vxpdb_group_copy(dest, group);
	return 1;
}

static int vshadow_getgrnam(struct vxpdb_state *vp, const char *name,
    struct vxpdb_group *dest)
{
	const struct shadow_state *state = vp->state;
	const struct vxpdb_group *group;

	if ((group = lookup_group(state->dq_group, name, PDB_NOGID)) == NULL)
		return 0;
	if (dest != NULL)
		vxpdb_group_copy(dest, group);
	return 1;
}

static void *vshadow_grouptrav_init(struct vxpdb_state *vp)
{
	struct shadow_state *state = vp->state;
	struct traverser_state trav;

	trav.wp = skip_nis_groups(state->dq_group->first);
	return HX_memdup(&trav, sizeof(trav));
}

static int vshadow_grouptrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_group *dest)
{
	struct traverser_state *trav = ptr;
	if (trav->wp == NULL)
		return 0;
	vxpdb_group_copy(dest, trav->wp->ptr);
	trav->wp = skip_nis_groups(trav->wp->next);
	return 1;
}

static void vshadow_grouptrav_free(struct vxpdb_state *vp, void *ptr)
{
	free(ptr);
	return;
}

EXPORT_SYMBOL struct vxpdb_driver THIS_MODULE = {
	.name           = "vxShadow back-end module",
	.desc           = "for shadow suite (and vxshadow extension)",
	.init           = vshadow_init,
	.open           = vshadow_open,
	.close          = vshadow_close,
	.exit           = vshadow_exit,
	.modctl         = vshadow_modctl,
	.lock           = vshadow_lock,
	.unlock         = vshadow_unlock,
	.useradd        = vshadow_useradd,
	.usermod        = vshadow_usermod,
	.userdel        = vshadow_userdel,
	.getpwuid       = vshadow_getpwuid,
	.getpwnam       = vshadow_getpwnam,
	.usertrav_init  = vshadow_usertrav_init,
	.usertrav_walk  = vshadow_usertrav_walk,
	.usertrav_free  = vshadow_usertrav_free,
	.groupadd       = vshadow_groupadd,
	.groupmod       = vshadow_groupmod,
	.groupdel       = vshadow_groupdel,
	.getgrgid       = vshadow_getgrgid,
	.getgrnam       = vshadow_getgrnam,
	.grouptrav_init = vshadow_grouptrav_init,
	.grouptrav_walk = vshadow_grouptrav_walk,
	.grouptrav_free = vshadow_grouptrav_free,
};
