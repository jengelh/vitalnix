/*
 *	shadow/shadow.c - Shadow back-end (local files)
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2002 - 2008
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
#include <libHX/deque.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include "drivers/shadow/shadow.h"
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

#define RWLOCKED(z)     ((z)->flags & VXDB_WRLOCK)
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
static void vxshadow_close(struct vxdb_state *);

/* Variables */
static char *Path_passwd   = "/etc/passwd";
static char *Path_shadow   = "/etc/shadow";
static char *Path_group    = "/etc/group";
static char *Path_vxpasswd = "/etc/vxpasswd";
static char *Path_vxshadow = "/etc/vxshadow";

//-----------------------------------------------------------------------------
static int vxshadow_init(struct vxdb_state *vp, const char *config_file)
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

static int vxshadow_open(struct vxdb_state *vp, unsigned int flags)
{
	struct shadow_state *state = vp->state;
	int ret;

	if ((ret = db_open(state, flags & (VXDB_ADMIN | VXDB_WRLOCK))) <= 0) {
		vxshadow_close(vp);
		return ret;
	}
	return 1;
}

static void vxshadow_close(struct vxdb_state *vp)
{
	struct shadow_state *state = vp->state;
	db_close(state);
	free_data(state);
}

static void vxshadow_exit(struct vxdb_state *vp)
{
	struct shadow_state *state = vp->state;
	read_config(state, CONFIG_FREE, NULL);
	free(state);
}

static long vxshadow_modctl(struct vxdb_state *vp, unsigned int command, ...)
{
	struct shadow_state *state = vp->state;
	errno = 0;

	switch (command) {
		case VXDB_FLUSH:
			if (!(state->flags & VXDB_WRLOCK))
				return -EPERM;
			db_flush(state, true);
			return 1;
		case VXDB_COUNT_USERS:
			return state->dq_user->items;
		case VXDB_COUNT_GROUPS:
			return state->dq_group->items;
		case VXDB_NEXTUID_SYS:
			return automatic_uid(state, VXDB_AUTOUID_SYS);
		case VXDB_NEXTUID:
			return automatic_uid(state, VXDB_AUTOUID);
		case VXDB_NEXTGID_SYS:
			return automatic_gid(state, VXDB_AUTOGID_SYS);
		case VXDB_NEXTGID:
			return automatic_gid(state, VXDB_AUTOGID);
	}

	return -ENOSYS;
}

static int vxshadow_lock(struct vxdb_state *vp)
{
	struct shadow_state *state = vp->state;
	if (RWLOCKED(state))
		return 1;
	db_close(state);
	free_data(state);
	return db_open(state, state->flags | VXDB_WRLOCK);
}

static int vxshadow_unlock(struct vxdb_state *vp)
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
	state->flags &= ~VXDB_WRLOCK;
	return 1;
}

static int vxshadow_useradd(struct vxdb_state *vp,
    const struct vxdb_user *rq)
{
	struct shadow_state *state = vp->state;
	struct vxdb_user *nu;
	unsigned int uid;

	if (rq->pw_name == NULL)
		return -EINVAL;
	if (lookup_user(state->dq_user, rq->pw_name, VXDB_NOUID) != NULL)
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
	if ((nu = calloc(1, sizeof(struct vxdb_user))) == NULL)
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

static int vxshadow_usermod(struct vxdb_state *vp, const char *name,
    const struct vxdb_user *mod_mask)
{
#define UP_INT(__field) \
	if (mod_mask->__field != VXDB_NO_CHANGE) \
		user->__field = mod_mask->__field;
#define UP_STR(__field) \
	if (mod_mask->__field != NULL) \
		HX_strclone(&user->__field, mod_mask->__field);

	struct shadow_state *state = vp->state;
	struct vxdb_user *user;

	RWLOCK_CHK(state);

	if (name == NULL)
		return -EINVAL;
	user = lookup_user(state->dq_user, name, VXDB_NOUID);
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

static int vxshadow_userdel(struct vxdb_state *vp, const char *name)
{
	struct shadow_state *state = vp->state;
	struct vxdb_user *user;

	RWLOCK_CHK(state);

	if (name == NULL)
		return -EINVAL;
	user = lookup_user(state->dq_user, name, VXDB_NOUID);
	if (user == NULL)
		return -ENOENT;

	HXdeque_del(HXdeque_find(state->dq_user, user));
	free_single_user(user);
	TOUCH_USER_TAG(true);
	db_flush(state, false);
	return 1;
}

static int vxshadow_getpwuid(struct vxdb_state *vp, unsigned int uid,
    struct vxdb_user *dest)
{
	const struct shadow_state *state = vp->state;
	const struct vxdb_user *user;

	if ((user = lookup_user(state->dq_user, NULL, uid)) == NULL)
		return 0;
	if (dest != NULL)
		vxdb_user_copy(dest, user);
	return 1;
}

static int vxshadow_getpwnam(struct vxdb_state *vp, const char *name,
    struct vxdb_user *dest)
{
	const struct shadow_state *state = vp->state;
	const struct vxdb_user *user;

	if ((user = lookup_user(state->dq_user, name, VXDB_NOUID)) == NULL)
		return 0;
	if (dest != NULL)
		vxdb_user_copy(dest, user);
	return 1;
}

static void *vxshadow_usertrav_init(struct vxdb_state *vp)
{
	struct shadow_state *state = vp->state;
	struct traverser_state trav;

	trav.wp = skip_nis_users(state->dq_user->first);
	return HX_memdup(&trav, sizeof(trav));
}

static int vxshadow_usertrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_user *dest)
{
	struct traverser_state *trav = ptr;
	if (trav->wp == NULL)
		return 0;
	vxdb_user_copy(dest, trav->wp->ptr);
	trav->wp = skip_nis_users(trav->wp->next);
	return 1;
}

static void vxshadow_usertrav_free(struct vxdb_state *vp, void *ptr)
{
	free(ptr);
}

static int vxshadow_groupadd(struct vxdb_state *vp,
    const struct vxdb_group *rq)
{
	struct shadow_state *state = vp->state;
	struct vxdb_group *ng;
	unsigned int gid;

	if (rq->gr_name == NULL)
		return -EINVAL;
	if (lookup_group(state->dq_group, rq->gr_name, VXDB_NOGID) == NULL)
		return -ENOENT;

	RWLOCK_CHK(state);

	gid = automatic_gid(state, rq->gr_gid);
	if (gid == -ENOSPC)
		return -ENOSPC;

	if ((ng = calloc(1, sizeof(struct vxdb_group))) == NULL)
		return -errno;

	ng->gr_name = HX_strdup(rq->gr_name);
	ng->gr_gid  = gid;
	HXdeque_push(state->dq_group, ng);
	TOUCH_GROUP_TAG(true);
	db_flush(state, false);
	return 1;
}

static int vxshadow_groupmod(struct vxdb_state *vp, const char *name,
    const struct vxdb_group *mod_mask)
{
	struct shadow_state *state = vp->state;
	struct vxdb_group *group;

	RWLOCK_CHK(state);
	group = lookup_group(state->dq_group, name, VXDB_NOGID);
	if (group == NULL)
		return -ENOENT;

	if (mod_mask->gr_name != NULL)
		HX_strclone(&group->gr_name, mod_mask->gr_name);
	if (mod_mask->gr_gid != VXDB_NOGID)
		group->gr_gid = mod_mask->gr_gid;

	TOUCH_GROUP_TAG(true);
	db_flush(state, false);
	return 1;
}

static int vxshadow_groupdel(struct vxdb_state *vp, const char *name)
{
	struct shadow_state *state = vp->state;
	struct vxdb_group *grp;

	RWLOCK_CHK(state);
	grp = lookup_group(state->dq_group, name, VXDB_NOGID);
	if (grp == NULL)
		return -ENOENT;

	HXdeque_del(HXdeque_find(state->dq_group, grp));
	free_single_group(grp);
	TOUCH_GROUP_TAG(true);
	db_flush(state, false);
	return 1;
}

static int vxshadow_getgrgid(struct vxdb_state *vp, unsigned int gid,
    struct vxdb_group *dest)
{
	const struct shadow_state *state = vp->state;
	const struct vxdb_group *group;

	if ((group = lookup_group(state->dq_group, NULL, gid)) == NULL)
		return 0;
	if (dest != NULL)
		vxdb_group_copy(dest, group);
	return 1;
}

static int vxshadow_getgrnam(struct vxdb_state *vp, const char *name,
    struct vxdb_group *dest)
{
	const struct shadow_state *state = vp->state;
	const struct vxdb_group *group;

	if ((group = lookup_group(state->dq_group, name, VXDB_NOGID)) == NULL)
		return 0;
	if (dest != NULL)
		vxdb_group_copy(dest, group);
	return 1;
}

static void *vxshadow_grouptrav_init(struct vxdb_state *vp)
{
	struct shadow_state *state = vp->state;
	struct traverser_state trav;

	trav.wp = skip_nis_groups(state->dq_group->first);
	return HX_memdup(&trav, sizeof(trav));
}

static int vxshadow_grouptrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_group *dest)
{
	struct traverser_state *trav = ptr;
	if (trav->wp == NULL)
		return 0;
	vxdb_group_copy(dest, trav->wp->ptr);
	trav->wp = skip_nis_groups(trav->wp->next);
	return 1;
}

static void vxshadow_grouptrav_free(struct vxdb_state *vp, void *ptr)
{
	free(ptr);
}

EXPORT_SYMBOL struct vxdb_driver THIS_MODULE = {
	.name           = "vxShadow back-end module",
	.desc           = "for shadow suite (and vxshadow extension)",
	.init           = vxshadow_init,
	.open           = vxshadow_open,
	.close          = vxshadow_close,
	.exit           = vxshadow_exit,
	.modctl         = vxshadow_modctl,
	.lock           = vxshadow_lock,
	.unlock         = vxshadow_unlock,
	.useradd        = vxshadow_useradd,
	.usermod        = vxshadow_usermod,
	.userdel        = vxshadow_userdel,
	.getpwuid       = vxshadow_getpwuid,
	.getpwnam       = vxshadow_getpwnam,
	.usertrav_init  = vxshadow_usertrav_init,
	.usertrav_walk  = vxshadow_usertrav_walk,
	.usertrav_free  = vxshadow_usertrav_free,
	.groupadd       = vxshadow_groupadd,
	.groupmod       = vxshadow_groupmod,
	.groupdel       = vxshadow_groupdel,
	.getgrgid       = vxshadow_getgrgid,
	.getgrnam       = vxshadow_getgrnam,
	.grouptrav_init = vxshadow_grouptrav_init,
	.grouptrav_walk = vxshadow_grouptrav_walk,
	.grouptrav_free = vxshadow_grouptrav_free,
};
