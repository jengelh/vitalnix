/*
 *	mmd.c - Multiple module back-end
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2007
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
#include <libHX/deque.h>
#include <libHX/misc.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>

#define WR_OPEN(state)  ((state)->wr_mod.mh_state == STATE_OPEN)
#define WR_MOD(state)   ((state)->wr_mod.mh_instance)

/* Definitions */
enum open_state {
	STATE_OUT,
	STATE_LOADED,
	STATE_OPEN,
};

struct module_handle {
	char *mh_name;
	enum open_state mh_state;
	struct vxdb_state *mh_instance;
};

struct multi_state {
	char *rdmod_str, *wrmod_str;
	struct HXdeque *rd_mod;
	struct module_handle wr_mod;
	unsigned int uid_min, uid_max, gid_min, gid_max;
};

/**
 * struct vxmmd_trav -
 * @rd_mod:	current RD module that is traversed
 * @wr_mod:	true if the WD module is to be traversed (changes)
 * @itrav:	traverser for current read module
 */
struct vxmmd_trav {
	void *itrav;
	struct HXdeque_node *rd_mod;
	bool wr_mod;
};

/* Functions */
static unsigned int vxmmd_autouid(struct vxdb_state *, unsigned int);
static unsigned int vxmmd_autogid(struct vxdb_state *, unsigned int);

static int modules_construct(struct multi_state *);
static int modules_open(struct multi_state *, unsigned int);
static void modules_close(struct multi_state *);
static void modules_destruct(struct multi_state *);
static void read_config(struct multi_state *, const char *);

//-----------------------------------------------------------------------------
static int vxmmd_init(struct vxdb_state *vp, const char *config_file)
{
	struct multi_state *state;

	if ((state = vp->state = calloc(1, sizeof(struct multi_state))) == NULL)
		return -errno;

	read_config(state, config_file);
	if (!modules_construct(state))
		goto out;

	return 1;

 out:
	modules_destruct(state);
	return -EINVAL;
}

static int vxmmd_open(struct vxdb_state *vp, unsigned int flags)
{
	struct multi_state *state = vp->state;
	int ret;

	if ((ret = modules_open(state, flags)) <= 0)
		return ret;

	return 1;
}

static void vxmmd_close(struct vxdb_state *vp)
{
	struct multi_state *state = vp->state;
	modules_close(state);
}

static void vxmmd_exit(struct vxdb_state *vp)
{
	struct multi_state *state = vp->state;
	modules_destruct(state);
	free(state);
}

static unsigned int vxmmd_modctl_count(const struct multi_state *state,
    unsigned int command)
{
	const struct HXdeque_node *trav;
	unsigned int number = 0;
	int ret;

	if (WR_OPEN(state)) {
		ret = vxdb_modctl(WR_MOD(state), command);
		if (ret >= 0)
			number += ret;
	}

	for (trav = state->rd_mod->first; trav != NULL; trav = trav->next) {
		const struct module_handle *mh = trav->ptr;
		ret = vxdb_modctl(mh->mh_instance, command);
		if (ret >= 0)
			number += ret;
	}

	return number;
}

static long vxmmd_modctl(struct vxdb_state *vp, unsigned int command, ...)
{
	const struct multi_state *state = vp->state;
	errno = 0;

	switch (command) {
		case VXDB_FLUSH:
			if (WR_OPEN(state))
				return vxdb_modctl(WR_MOD(state), command);
			return 1;
		case VXDB_COUNT_USERS:
			return vxmmd_modctl_count(state, command);
		case VXDB_COUNT_GROUPS:
			return vxmmd_modctl_count(state, command);
		case VXDB_NEXTUID_SYS:
			return vxmmd_autouid(vp, VXDB_AUTOUID_SYS);
		case VXDB_NEXTUID:
			return vxmmd_autouid(vp, VXDB_AUTOUID);
		case VXDB_NEXTGID_SYS:
			return vxmmd_autogid(vp, VXDB_AUTOGID_SYS);
		case VXDB_NEXTGID:
			return vxmmd_autogid(vp, VXDB_AUTOGID);
	}

	return -ENOSYS;
}

static int vxmmd_lock(struct vxdb_state *vp)
{
	const struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxdb_lock(WR_MOD(state));
	return 1;
}

static int vxmmd_unlock(struct vxdb_state *vp)
{
	const struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxdb_unlock(WR_MOD(state));
	return 1;
}

//-----------------------------------------------------------------------------
static int vxmmd_useradd(struct vxdb_state *vp, const struct vxdb_user *rq)
{
	const struct multi_state *state = vp->state;
	struct vxdb_user rq2 = {};
	long uid;
	int ret;

	if (!WR_OPEN(state))
		return -EROFS;

	if (rq->pw_uid != VXDB_AUTOUID && rq->pw_uid != VXDB_AUTOUID_SYS)
		return vxdb_useradd(WR_MOD(state), rq);

	uid = vxmmd_autouid(vp, rq->pw_uid);
	if (uid == -ENOSPC)
		return -ENOSPC;

	vxdb_user_copy(&rq2, rq);
	rq2.pw_uid = uid;
	ret = vxdb_useradd(WR_MOD(state), &rq2);
	vxdb_user_free(&rq2, false);
	return ret;
}

static int vxmmd_usermod(struct vxdb_state *vp, const char *name,
    const struct vxdb_user *newstuff)
{
	const struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxdb_usermod(WR_MOD(state), name, newstuff);
	return -EROFS;
}

static int vxmmd_userdel(struct vxdb_state *vp, const char *name)
{
	const struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxdb_userdel(WR_MOD(state), name);
	return -EROFS;
}

static int vxmmd_getpwuid(struct vxdb_state *vp, unsigned int uid,
    struct vxdb_user *dest)
{
	const struct HXdeque_node *rd_node;
	const struct multi_state *state = vp->state;
	int ret;

	if (WR_OPEN(state)) {
		ret = vxdb_getpwuid(WR_MOD(state), uid, dest);
		if (ret > 0)
			return ret;
	}

	for (rd_node = state->rd_mod->first; rd_node != NULL;
	    rd_node = rd_node->next)
	{
		const struct module_handle *mh = rd_node->ptr;
		ret = vxdb_getpwuid(mh->mh_instance, uid, dest);
		if (ret > 0)
			return ret;
	}

	return 0;
}

static int vxmmd_getpwnam(struct vxdb_state *vp, const char *user,
    struct vxdb_user *dest)
{
	const struct HXdeque_node *rd_node;
	const struct multi_state *state = vp->state;
	int ret;

	if (WR_OPEN(state)) {
		ret = vxdb_getpwnam(WR_MOD(state), user, dest);
		if (ret > 0)
			return ret;
	}

	for (rd_node = state->rd_mod->first; rd_node != NULL;
	    rd_node = rd_node->next)
	{
		const struct module_handle *mh = rd_node->ptr;
		ret = vxdb_getpwnam(mh->mh_instance, user, dest);
		if (ret > 0)
			return ret;
	}

	return 0;
}

static void *vxmmd_usertrav_init(struct vxdb_state *vp)
{
	const struct multi_state *state = vp->state;
	struct vxmmd_trav trav = {};

	trav.rd_mod = state->rd_mod->first;
	if (WR_OPEN(state)) {
		trav.wr_mod = true;
		trav.itrav  = vxdb_usertrav_init(WR_MOD(state));
		if (trav.itrav == NULL)
			trav.wr_mod = false;
	}

	return HX_memdup(&trav, sizeof(trav));
}

static int vxmmd_usertrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_user *dest)
{
	const struct multi_state *state = vp->state;
	struct vxmmd_trav *trav = ptr;
	int ret;

	if (trav->wr_mod) {
		ret = vxdb_usertrav_walk(WR_MOD(state), trav->itrav, dest);
		if (ret != 0)
			return ret;
		vxdb_usertrav_free(WR_MOD(state), trav->itrav);
		trav->wr_mod = false;
		trav->itrav  = NULL;
	}

	for (; trav->rd_mod != NULL; trav->rd_mod = trav->rd_mod->next) {
		const struct module_handle *mh = trav->rd_mod->ptr;

		if (trav->itrav == NULL) {
			trav->itrav = vxdb_usertrav_init(mh->mh_instance);
			if (trav->itrav == NULL)
				continue;
		}

		ret = vxdb_usertrav_walk(mh->mh_instance, trav->itrav, dest);
		if (ret != 0)
			return ret;
		vxdb_usertrav_free(mh->mh_instance, trav->itrav);
	}

	return 0;
}

static void vxmmd_usertrav_free(struct vxdb_state *vp, void *ptr)
{
	free(ptr);
}

//-----------------------------------------------------------------------------
static int vxmmd_groupadd(struct vxdb_state *vp, const struct vxdb_group *rq)
{
	const struct multi_state *state = vp->state;
	struct vxdb_group rq2 = {};
	long gid;
	int ret;

	if (!WR_OPEN(state))
		return -EROFS;

	if (rq->gr_gid != VXDB_AUTOGID && rq->gr_gid != VXDB_AUTOGID_SYS)
		return vxdb_groupadd(WR_MOD(state), rq);

	gid = vxmmd_autogid(vp, rq->gr_gid);
	if (gid == -ENOSPC)
		return -ENOSPC;

	vxdb_group_copy(&rq2, rq);
	rq2.gr_gid = gid;
	ret = vxdb_groupadd(WR_MOD(state), &rq2);
	vxdb_group_free(&rq2, false);
	return ret;
}

static int vxmmd_groupmod(struct vxdb_state *vp, const char *name,
    const struct vxdb_group *newstuff)
{
	const struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxdb_groupmod(WR_MOD(state), name, newstuff);
	return -EROFS;
}

static int vxmmd_groupdel(struct vxdb_state *vp, const char *name)
{
	const struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxdb_groupdel(WR_MOD(state), name);
	return -EROFS;
}

static int vxmmd_getgrgid(struct vxdb_state *vp, unsigned int gid,
    struct vxdb_group *dest)
{
	const struct HXdeque_node *rd_node;
	const struct multi_state *state = vp->state;
	int ret;

	if (WR_OPEN(state)) {
		ret = vxdb_getgrgid(WR_MOD(state), gid, dest);
		if (ret > 0)
			return ret;
	}

	for (rd_node = state->rd_mod->first; rd_node != NULL;
	    rd_node = rd_node->next)
	{
		const struct module_handle *mh = rd_node->ptr;
		ret = vxdb_getgrgid(mh->mh_instance, gid, dest);
		if (ret > 0)
			return ret;
	}

	return 0;
}

static int vxmmd_getgrnam(struct vxdb_state *vp, const char *group,
    struct vxdb_group *dest)
{
	const struct HXdeque_node *rd_node;
	const struct multi_state *state = vp->state;
	int ret;

	if (WR_OPEN(state)) {
		ret = vxdb_getgrnam(WR_MOD(state), group, dest);
		if (ret > 0)
			return ret;
	}

	for (rd_node = state->rd_mod->first; rd_node != NULL;
	    rd_node = rd_node->next)
	{
		const struct module_handle *mh = rd_node->ptr;
		ret = vxdb_getgrnam(mh->mh_instance, group, dest);
		if (ret > 0)
			return ret;
	}

	return 0;
}

static void *vxmmd_grouptrav_init(struct vxdb_state *vp)
{
	const struct multi_state *state = vp->state;
	struct vxmmd_trav trav = {};

	trav.rd_mod = state->rd_mod->first;
	if (WR_OPEN(state)) {
		trav.wr_mod = true;
		trav.itrav  = vxdb_grouptrav_init(WR_MOD(state));
		if (trav.itrav == NULL)
			trav.wr_mod = false;
	}

	return HX_memdup(&trav, sizeof(trav));
}

static int vxmmd_grouptrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_group *dest)
{
	const struct multi_state *state = vp->state;
	struct vxmmd_trav *trav = ptr;
	int ret;

	if (trav->wr_mod) {
		ret = vxdb_grouptrav_walk(WR_MOD(state), trav->itrav, dest);
		if (ret != 0)
			return ret;
		vxdb_grouptrav_free(WR_MOD(state), trav->itrav);
		trav->wr_mod = false;
		trav->itrav  = NULL;
	}

	for (; trav->rd_mod != NULL; trav->rd_mod = trav->rd_mod->next) {
		const struct module_handle *mh = trav->rd_mod->ptr;

		if (trav->itrav == NULL) {
			trav->itrav = vxdb_grouptrav_init(mh->mh_instance);
			if (trav->itrav == NULL)
				continue;
		}

		ret = vxdb_grouptrav_walk(mh->mh_instance, trav->itrav, dest);
		if (ret != 0)
			return ret;
		vxdb_grouptrav_free(mh->mh_instance, trav->itrav);
	}

	return 0;
}

static void vxmmd_grouptrav_free(struct vxdb_state *vp, void *ptr)
{
	free(ptr);
}

//-----------------------------------------------------------------------------
static unsigned int vxmmd_autouid(struct vxdb_state *vp, unsigned int wanted)
{
	const struct multi_state *state = vp->state;
	unsigned int accept, min, max;
	struct vxdb_user dest = {};
	int high = -1;
	void *trav;

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
	for (trav = vxmmd_usertrav_init(vp);
	    vxmmd_usertrav_walk(vp, trav, &dest) > 0;)
	{
		long uid = dest.pw_uid;
		if (uid >= min && uid <= max && uid > high)
			high = uid;
	}
	vxmmd_usertrav_free(vp, trav);

	/* If the successor id is not outside the boundaries, take it. */
	if (high >= 0 && high < max) {
		vxdb_user_free(&dest, false);
		return high + 1;
	}

	/* No? Then just scan for the next */
	while (accept <= max) {
		bool used = false;
		for (trav = vxmmd_usertrav_init(vp);
		    vxmmd_usertrav_walk(vp, trav, &dest) > 0;)
		{
			if (dest.pw_uid == accept) {
				used = true;
				break;
			}
		}
		vxmmd_usertrav_free(vp, trav);
		if (!used) {
			vxdb_user_free(&dest, false);
			return accept;
		}
		++accept;
	}
	vxdb_user_free(&dest, false);
	return -ENOSPC;
}

static unsigned int vxmmd_autogid(struct vxdb_state *vp, unsigned int wanted)
{
	const struct multi_state *state = vp->state;
	unsigned int accept, min, max;
	struct vxdb_group dest = {};
	int high = -1;
	void *trav;

	if (wanted == VXDB_AUTOGID_SYS) {
		min = accept = 1;
		max = state->gid_min - 1;
	} else if (wanted == VXDB_AUTOGID) {
		min = accept = state->gid_min;
		max = state->gid_max;
	} else {
		return wanted;
	}

	/* Find the highest ID */
	for (trav = vxmmd_grouptrav_init(vp);
	    vxmmd_grouptrav_walk(vp, trav, &dest) > 0;)
	{
		long gid = dest.gr_gid;
		if (gid >= min && gid <= max && gid > high)
			high = gid;
	}
	vxmmd_grouptrav_free(vp, trav);

	/* If the successor id is not outside the boundaries, take it. */
	if (high >= 0 && high < max) {
		vxdb_group_free(&dest, false);
		return high + 1;
	}

	/* No? Then just scan for the next */
	while (accept <= max) {
		bool used = false;
		for (trav = vxmmd_grouptrav_init(vp);
		    vxmmd_grouptrav_walk(vp, trav, &dest) > 0;)
		{
			if (dest.gr_gid == accept) {
				used = true;
				break;
			}
		}
		vxmmd_grouptrav_free(vp, trav);
		if (!used) {
			vxdb_group_free(&dest, false);
			return accept;
		}
		++accept;
	}
	vxdb_group_free(&dest, false);
	return -ENOSPC;
}

//-----------------------------------------------------------------------------
static int modules_construct(struct multi_state *state)
{
	char **rdmod_list = HX_split(state->rdmod_str, ":", NULL, 0);
	char **name = rdmod_list;
	int ret = 0;

	state->rd_mod = HXdeque_init();
	while (name != NULL && *name != NULL) {
		struct module_handle mh;

		mh.mh_name = *name;
		if ((mh.mh_instance = vxdb_load(*name)) == NULL)
			goto out;

		mh.mh_state = STATE_LOADED;
		HXdeque_push(state->rd_mod, HX_memdup(&mh, sizeof(mh)));
		++name;
	}

	state->wr_mod.mh_name     = state->wrmod_str;
	state->wr_mod.mh_state    = STATE_OUT;
	state->wr_mod.mh_instance = vxdb_load(state->wr_mod.mh_name);
	state->wrmod_str          = NULL;
	if (state->wr_mod.mh_instance == NULL) {
		ret = 0;
	} else {
		state->wr_mod.mh_state = STATE_LOADED;
		ret = 1;
	}

 out:
	HX_zvecfree(rdmod_list);
	free(state->rdmod_str);
	state->rdmod_str = NULL;
	return ret;
}

static int modules_open(struct multi_state *state, unsigned int flags)
{
	const struct HXdeque_node *node;
	int ret;

	ret = vxdb_open(state->wr_mod.mh_instance, flags);
	if (ret <= 0)
		return ret;
	state->wr_mod.mh_state = STATE_OPEN;

	for (node = state->rd_mod->first; node != NULL; node = node->next) {
		struct module_handle *mh = node->ptr;
		ret = vxdb_open(mh->mh_instance, flags & ~VXDB_WRLOCK);
		if (ret <= 0) {
			modules_close(state);
			return ret;
		}
		mh->mh_state = STATE_OPEN;
	}

	return ret;
}

static void modules_close(struct multi_state *state)
{
	const struct HXdeque_node *node;

	for (node = state->rd_mod->first; node != NULL; node = node->next) {
		struct module_handle *mod = node->ptr;
		if (mod->mh_state == STATE_OPEN)
			vxdb_close(mod->mh_instance);
		mod->mh_state = STATE_LOADED;
	}

	if (WR_OPEN(state)) {
		vxdb_close(WR_MOD(state));
		state->wr_mod.mh_state = STATE_LOADED;
	}
}

static void modules_destruct(struct multi_state *state)
{
	const struct HXdeque_node *node;

	for (node = state->rd_mod->first; node != NULL; node = node->next) {
		struct module_handle *mod = node->ptr;
		if (mod->mh_state == STATE_LOADED) {
			vxdb_unload(mod->mh_instance);
			mod->mh_state = STATE_OUT;
		}
	}

	if (state->wr_mod.mh_state == STATE_LOADED) {
		vxdb_unload(WR_MOD(state));
		state->wr_mod.mh_state = STATE_OUT;
		free(state->wr_mod.mh_name);
	}
}

static void read_config(struct multi_state *state, const char *file)
{
	struct HXoption autouid_table[] = {
		{.ln = "UID_MIN", .type = HXTYPE_UINT, .ptr = &state->uid_min},
		{.ln = "UID_MAX", .type = HXTYPE_UINT, .ptr = &state->uid_max},
		{.ln = "GID_MIN", .type = HXTYPE_UINT, .ptr = &state->gid_min},
		{.ln = "GID_MAX", .type = HXTYPE_UINT, .ptr = &state->gid_max},
		HXOPT_TABLEEND,
	};
	struct HXoption options_table[] = {
		{.ln = "RD_MODULES", .type = HXTYPE_STRING, .ptr = &state->rdmod_str},
		{.ln = "WR_MODULE",  .type = HXTYPE_STRING, .ptr = &state->wrmod_str},
		HXOPT_TABLEEND,
	};

	state->uid_min = state->gid_min = 1000;
	state->uid_max = state->gid_max = 60000;
	HX_shconfig(CONFIG_SYSCONFDIR "/autouid.conf", autouid_table);
	HX_shconfig(file, options_table);
}

static int vxmmd_sgmapadd(struct vxdb_state *vp, const char *user,
    const char *group)
{
	const struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxdb_sgmapadd(WR_MOD(state), user, group);
	return -EROFS;
}

static int vxmmd_sgmapmerge(char ***outp, unsigned int *out_entries,
    char **inp, unsigned int in_entries)
{
	char **out, **in = inp;

	if (outp == NULL) {
		*out_entries += in_entries;
		return 0;
	}

	out = realloc(*outp, sizeof(char *) * (*out_entries + in_entries + 1));
	if (out == NULL)
		return -ENOMEM;

	*outp = out;
	out = &out[*out_entries];
	*out_entries += in_entries;
	while (in_entries-- > 0)
		*out++ = *in++;

	*out = NULL;
	free(inp);
	return 0;
}

static int vxmmd_sgmapget(struct vxdb_state *vp, const char *user,
    char ***result)
{
	const struct multi_state *state = vp->state;
	const struct HXdeque_node *trav;
	unsigned int entries = 0;
	char **out = NULL;
	int ret;

	if (result != NULL)
		*result = NULL;

	if (WR_OPEN(state)) {
		ret = vxdb_sgmapget(WR_MOD(state), user,
		      (result == NULL) ? NULL : &out);
		if (ret > 0)
			ret = vxmmd_sgmapmerge(result, &entries, out, ret);
		if (ret == -ENOMEM)
			goto free;
	}

	for (trav = state->rd_mod->first; trav != NULL; trav = trav->next) {
		const struct module_handle *mh = trav->ptr;
		ret = vxdb_sgmapget(mh->mh_instance, user,
		      (result == NULL) ? NULL : &out);
		if (ret > 0)
			ret = vxmmd_sgmapmerge(result, &entries, out, ret);
		if (ret == -ENOMEM)
			goto free;
	}

	return entries;

 free:
	if (result != NULL)
		HX_zvecfree(*result);
	return ret;
}

static int vxmmd_sgmapdel(struct vxdb_state *vp, const char *user,
    const char *group)
{
	const struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxdb_sgmapdel(WR_MOD(state), user, group);
	return -EROFS;
}

EXPORT_SYMBOL struct vxdb_driver THIS_MODULE = {
	.name           = "Multiple Module driver",
	.desc           = "Logically combines databases",
	.init           = vxmmd_init,
	.open           = vxmmd_open,
	.close          = vxmmd_close,
	.exit           = vxmmd_exit,
	.modctl         = vxmmd_modctl,
	.lock           = vxmmd_lock,
	.unlock         = vxmmd_unlock,
	.useradd        = vxmmd_useradd,
	.usermod        = vxmmd_usermod,
	.userdel        = vxmmd_userdel,
	.getpwuid       = vxmmd_getpwuid,
	.getpwnam       = vxmmd_getpwnam,
	.usertrav_init  = vxmmd_usertrav_init,
	.usertrav_walk  = vxmmd_usertrav_walk,
	.usertrav_free  = vxmmd_usertrav_free,
	.groupadd       = vxmmd_groupadd,
	.groupmod       = vxmmd_groupmod,
	.groupdel       = vxmmd_groupdel,
	.getgrgid       = vxmmd_getgrgid,
	.getgrnam       = vxmmd_getgrnam,
	.grouptrav_init = vxmmd_grouptrav_init,
	.grouptrav_walk = vxmmd_grouptrav_walk,
	.grouptrav_free = vxmmd_grouptrav_free,
	.sgmapadd       = vxmmd_sgmapadd,
	.sgmapget       = vxmmd_sgmapget,
	.sgmapdel       = vxmmd_sgmapdel,
};
