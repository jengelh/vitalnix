/*
 *	mmd.c - Multiple module back-end
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX.h>
#include <vitalnix/config.h>
#include "drivers/proto.h"
#include "drivers/static-build.h"
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>

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
	struct vxpdb_state *mh_instance;
};

struct multi_state {
	char *rdmod_str, *wrmod_str;
	struct HXdeque *rd_mod;
	struct module_handle wr_mod;
};

/* Functions */
static int modules_construct(struct multi_state *);
static int modules_open(struct multi_state *, long);
static void modules_close(struct multi_state *);
static void modules_destruct(struct multi_state *);
static void read_config(struct multi_state *);

//-----------------------------------------------------------------------------
static int vmmd_init(struct vxpdb_state *vp, const char *config_file)
{
	struct multi_state *state;

	if ((state = vp->state = calloc(1, sizeof(struct multi_state))) == NULL)
		return -errno;

	read_config(state);
	if (!modules_construct(state))
		goto out;

	return 1;

 out:
	modules_destruct(state);
	return -EINVAL;
}

static int vmmd_open(struct vxpdb_state *vp, long flags)
{
	struct multi_state *state = vp->state;
	int ret;

	if ((ret = modules_open(state, flags)) <= 0)
		return ret;

	return 1;
}

static void vmmd_close(struct vxpdb_state *vp)
{
	struct multi_state *state = vp->state;
	modules_close(state);
	return;
}

static void vmmd_exit(struct vxpdb_state *vp)
{
	struct multi_state *state = vp->state;
	modules_destruct(state);
	free(state);
	return;
}

static long vmmd_modctl(struct vxpdb_state *vp, long command, ...)
{
	return -ENOSYS;
}

static int vmmd_lock(struct vxpdb_state *vp)
{
	struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxpdb_lock(WR_MOD(state));
	return 1;
}

static int vmmd_unlock(struct vxpdb_state *vp)
{
	struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxpdb_unlock(WR_MOD(state));
	return 1;
}

//-----------------------------------------------------------------------------
static int vmmd_useradd(struct vxpdb_state *vp, const struct vxpdb_user *rq)
{
	struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxpdb_useradd(WR_MOD(state), rq);
	return -EROFS;
}

static int vmmd_usermod(struct vxpdb_state *vp,
    const struct vxpdb_user *sr_mask, const struct vxpdb_user *mod_mask)
{
	struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxpdb_usermod(WR_MOD(state), sr_mask, mod_mask);
	return -EROFS;
}

static int vmmd_userdel(struct vxpdb_state *vp,
    const struct vxpdb_user *sr_mask)
{
	struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxpdb_userdel(WR_MOD(state), sr_mask);
	return -EROFS;
}

static int vmmd_userinfo(struct vxpdb_state *vp,
    const struct vxpdb_user *sr_mask, struct vxpdb_user *dest, size_t size)
{
/*
	struct shadow_state *state = vp->state;
	const struct HXdeque_node *travp = state->dq_user->first;
	struct vxpdb_user temp_mask;
	int found = 0;

	if (mask == result) {
		memcpy(&temp_mask, mask, sizeof(struct vxpdb_user));
		mask = &temp_mask;
	}

	for (travp = state->dq_user->first; travp != NULL &&
	    (dest == NULL || size > 0); travp = travp->Next)
	{
		const struct vxpdb_user *src = travp->ptr;
		if (!vxpdb_user_match(src, mask))
			continue;
		if (result != NULL) {
			vxpdb_user_copy(result, src);
			++result;
			++found;
			--size;
		} else {
			if (size == 0)
				return 1;
			++found;
		}
	}

	return found;
	*/
	return 0;
}

static void *vmmd_usertrav_init(struct vxpdb_state *vp)
{
	return NULL;
}

static int vmmd_usertrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_user *dest)
{
	return 1;
}

static void vmmd_usertrav_free(struct vxpdb_state *vp, void *ptr)
{
	return;
}

//-----------------------------------------------------------------------------
static int vmmd_groupadd(struct vxpdb_state *vp, const struct vxpdb_group *rq)
{
	struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxpdb_groupadd(WR_MOD(state), rq);
	return -EROFS;
}

static int vmmd_groupmod(struct vxpdb_state *vp,
    const struct vxpdb_group *sr_mask, const struct vxpdb_group *mod_mask)
{
	struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxpdb_groupmod(WR_MOD(state), sr_mask, mod_mask);
	return -EROFS;
}

static int vmmd_groupdel(struct vxpdb_state *vp,
    const struct vxpdb_group *sr_mask)
{
	struct multi_state *state = vp->state;
	if (WR_OPEN(state))
		return vxpdb_groupdel(WR_MOD(state), sr_mask);
	return -EROFS;
}

static void *vmmd_grouptrav_init(struct vxpdb_state *vp)
{
	return NULL;
}

static int vmmd_grouptrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_group *dest)
{
	return 1;
}

static void vmmd_grouptrav_free(struct vxpdb_state *vp, void *ptr)
{
	return;
}

static int vmmd_groupinfo(struct vxpdb_state *vp,
    const struct vxpdb_group *sr_mask, struct vxpdb_group *dest, size_t size)
{
	return 0;
}

//-----------------------------------------------------------------------------
static int modules_construct(struct multi_state *state)
{
	char **rdmod_list = HX_split(state->rdmod_str, ":", NULL, 0);
	struct module_handle *wr;
	char **name = rdmod_list;
	int ret = 0;

	state->rd_mod = HXdeque_init();
	while (name != NULL && *name != NULL) {
		struct module_handle mh;
		mh.mh_name = *name;
		if ((mh.mh_instance = vxpdb_load(*name)) == NULL)
			goto out;
		mh.mh_state = STATE_LOADED;
		HXdeque_push(state->rd_mod, HX_memdup(&mh, sizeof(mh)));
		++name;
	}

	wr = &state->wr_mod;
	wr->mh_name      = state->wrmod_str;
	wr->mh_state     = STATE_OUT;
	state->wrmod_str = NULL;
	if ((wr->mh_instance = vxpdb_load(wr->mh_name)) == NULL) {
		ret = 0;
	} else {
		wr->mh_state = STATE_LOADED;
		ret = 1;
	}

 out:
	HX_zvecfree(rdmod_list);
	free(state->rdmod_str);
	state->rdmod_str = NULL;
	return ret;
}

static int modules_open(struct multi_state *state, long flags)
{
	struct HXdeque_node *node = state->rd_mod->first;
	int ret = 1;

	for (node = state->rd_mod->first; node != NULL; node = node->next) {
		struct module_handle *mh = node->ptr;
		ret = vxpdb_open(mh->mh_instance, flags & ~PDB_WRLOCK);
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
	struct HXdeque_node *node;

	for (node = state->rd_mod->first; node != NULL; node = node->next) {
		struct module_handle *mod = node->ptr;
		if (mod->mh_state == STATE_OPEN)
			vxpdb_close(mod->mh_instance);
		mod->mh_state = STATE_LOADED;
	}

	if (WR_OPEN(state)) {
		vxpdb_close(WR_MOD(state));
		state->wr_mod.mh_state = STATE_LOADED;
	}

	return;
}

static void modules_destruct(struct multi_state *state)
{
	struct HXdeque_node *node;

	for (node = state->rd_mod->first; node != NULL; node = node->next) {
		struct module_handle *mod = node->ptr;
		if (mod->mh_state == STATE_LOADED) {
			vxpdb_unload(mod->mh_instance);
			mod->mh_state = STATE_OUT;
		}
	}

	if (state->wr_mod.mh_state == STATE_LOADED) {
		vxpdb_unload(WR_MOD(state));
		state->wr_mod.mh_state = STATE_OUT;
		free(state->wr_mod.mh_name);
	}

	return;
}

static void read_config(struct multi_state *state)
{
	struct HXoption options_table[] = {
		{.ln = "RD_MODULES", .type = HXTYPE_STRING, .ptr = state->rdmod_str},
		{.ln = "WR_MODULE",  .type = HXTYPE_STRING, .ptr = state->wrmod_str},
		HXOPT_TABLEEND,
	};

	HX_shconfig(CONFIG_SYSCONFDIR "/db_multi.conf", options_table);
	return;
}

static struct vxpdb_driver THIS_MODULE = {
	.name   = "Multiple Module driver",
	DRIVER_CB_ALL(vmmd),
};

REGISTER_MODULE(mmd, &THIS_MODULE);
