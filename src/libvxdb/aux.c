/*
 *	libvxdb/aux.c - Auxiliary functions
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

#define NDSTRLEN(s) (((s) != NULL) ? strlen(s) + 1 : 0)

EXPORT_SYMBOL void vxdb_user_clean(struct vxdb_user *u)
{
	memset(u, 0, sizeof(struct vxdb_user));
	u->pw_uid    = VXDB_NOUID;
	u->pw_gid    = VXDB_NOGID;
	u->sp_min    = VXDB_DFL_KEEPMIN;
	u->sp_max    = VXDB_DFL_KEEPMAX;
	u->sp_warn   = VXDB_DFL_WARNAGE;
	u->sp_expire = VXDB_NO_EXPIRE;
	u->sp_inact  = VXDB_NO_INACTIVE;
}

EXPORT_SYMBOL void vxdb_user_copy(struct vxdb_user *dest,
    const struct vxdb_user *src)
{
	hmc_strasg(&dest->pw_name, src->pw_name);
	dest->pw_uid     = src->pw_uid;
	dest->pw_gid     = src->pw_gid;
	hmc_strasg(&dest->pw_igrp, src->pw_igrp);
	hmc_strasg(&dest->pw_real, src->pw_real);
	hmc_strasg(&dest->pw_home, src->pw_home);
	hmc_strasg(&dest->pw_shell, src->pw_shell);
	hmc_strasg(&dest->sp_passwd, src->sp_passwd);
	hmc_strasg(&dest->sp_ntpasswd, src->sp_ntpasswd);
	dest->sp_lastchg = src->sp_lastchg;
	dest->sp_min     = src->sp_min;
	dest->sp_max     = src->sp_max;
	dest->sp_warn    = src->sp_warn;
	dest->sp_expire  = src->sp_expire;
	dest->sp_inact   = src->sp_inact;
	hmc_strasg(&dest->vs_uuid, src->vs_uuid);
	hmc_strasg(&dest->vs_pvgrp, src->vs_pvgrp);
	dest->vs_defer   = src->vs_defer;
	dest->be_priv    = NULL;
}

EXPORT_SYMBOL struct vxdb_user *vxdb_user_dup(const struct vxdb_user *src)
{
	struct vxdb_user *dest;
	if ((dest = calloc(1, sizeof(struct vxdb_user))) == NULL)
		return NULL;
	vxdb_user_copy(dest, src);
	return dest;
}

EXPORT_SYMBOL void vxdb_user_free(struct vxdb_user *user, bool heap)
{
	hmc_free(user->pw_name);
	hmc_free(user->pw_igrp);
	hmc_free(user->pw_real);
	hmc_free(user->pw_home);
	hmc_free(user->pw_shell);
	hmc_free(user->sp_passwd);
	hmc_free(user->sp_ntpasswd);
	hmc_free(user->vs_uuid);
	hmc_free(user->vs_pvgrp);
	if (heap)
		free(user);
}

EXPORT_SYMBOL void vxdb_user_nomodify(struct vxdb_user *u)
{
	memset(u, 0, sizeof(struct vxdb_user));
	u->pw_uid     = VXDB_NO_CHANGE;
	u->pw_gid     = VXDB_NO_CHANGE;
	u->sp_lastchg = VXDB_NO_CHANGE;
	u->sp_min     = VXDB_NO_CHANGE;
	u->sp_max     = VXDB_NO_CHANGE;
	u->sp_warn    = VXDB_NO_CHANGE;
	u->sp_expire  = VXDB_NO_CHANGE;
	u->sp_inact   = VXDB_NO_CHANGE;
	u->vs_defer   = VXDB_NO_CHANGE;
}

EXPORT_SYMBOL void vxdb_group_clean(struct vxdb_group *group)
{
	group->gr_name = NULL;
	group->gr_gid  = VXDB_NOGID;
	group->be_priv = NULL;
}

EXPORT_SYMBOL void vxdb_group_copy(struct vxdb_group *dest,
    const struct vxdb_group *src)
{
	hmc_strasg(&dest->gr_name, src->gr_name);
	dest->gr_gid  = src->gr_gid;
	dest->be_priv = NULL;
}

EXPORT_SYMBOL struct vxdb_group *vxdb_group_dup(const struct vxdb_group *src)
{
	struct vxdb_group *dest;
	if ((dest = calloc(1, sizeof(struct vxdb_group))) == NULL)
		return NULL;
	vxdb_group_copy(dest, src);
	return dest;
}

EXPORT_SYMBOL void vxdb_group_free(struct vxdb_group *group, bool heap)
{
	hmc_free(group->gr_name);
	if (heap)
		free(group);
}

EXPORT_SYMBOL void vxdb_group_nomodify(struct vxdb_group *group)
{
	group->gr_name = NULL;
	group->gr_gid  = VXDB_NO_CHANGE;
	group->be_priv = NULL;
}
