/*
 *	libvxpdb/aux.c - Auxiliary functions
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2005 - 2007
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
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

#define NDSTRLEN(s) (((s) != NULL) ? strlen(s) + 1 : 0)

//-----------------------------------------------------------------------------
EXPORT_SYMBOL void vxpdb_user_clean(struct vxpdb_user *u)
{
	hmc_strasg(&u->pw_name, NULL);
	u->pw_name   = NULL;
	u->pw_uid    = PDB_NOUID;
	u->pw_gid    = PDB_NOGID;
	u->pw_igrp   = NULL;
	hmc_strasg(&u->pw_real, NULL);
	hmc_strasg(&u->pw_home, NULL);
	hmc_strasg(&u->pw_shell, NULL);
	hmc_strasg(&u->sp_passwd, NULL);
	u->sp_lastchg = 0;
	u->sp_min    = PDB_DFL_KEEPMIN;
	u->sp_max    = PDB_DFL_KEEPMAX;
	u->sp_warn   = PDB_DFL_WARNAGE;
	u->sp_expire = PDB_NO_EXPIRE;
	u->sp_inact  = PDB_NO_INACTIVE;
	hmc_strasg(&u->vs_uuid, NULL);
	hmc_strasg(&u->vs_pvgrp, NULL);
	u->vs_defer   = 0;
	return;
}

EXPORT_SYMBOL void vxpdb_user_copy(struct vxpdb_user *dest,
    const struct vxpdb_user *src)
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
	return;
}

EXPORT_SYMBOL struct vxpdb_user *vxpdb_user_dup(const struct vxpdb_user *src)
{
	struct vxpdb_user *dest;
	if ((dest = calloc(1, sizeof(struct vxpdb_user))) == NULL)
		return NULL;
	vxpdb_user_copy(dest, src);
	return dest;
}

EXPORT_SYMBOL void vxpdb_user_free(struct vxpdb_user *user, int heap)
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
	return;
}

EXPORT_SYMBOL void vxpdb_user_nomodify(struct vxpdb_user *u)
{
	memset(u, 0, sizeof(struct vxpdb_user));
	u->pw_uid     = PDB_NO_CHANGE;
	u->pw_gid     = PDB_NO_CHANGE;
	u->sp_lastchg = PDB_NO_CHANGE;
	u->sp_min     = PDB_NO_CHANGE;
	u->sp_max     = PDB_NO_CHANGE;
	u->sp_warn    = PDB_NO_CHANGE;
	u->sp_expire  = PDB_NO_CHANGE;
	u->sp_inact   = PDB_NO_CHANGE;
	u->vs_defer   = PDB_NO_CHANGE;
	return;
}

EXPORT_SYMBOL void vxpdb_group_clean(struct vxpdb_group *group)
{
	hmc_strasg(&group->gr_name, NULL);
	group->gr_gid  = PDB_NOGID;
	group->be_priv = NULL;
	return;
}

EXPORT_SYMBOL void vxpdb_group_copy(struct vxpdb_group *dest,
    const struct vxpdb_group *src)
{
	hmc_strasg(&dest->gr_name, src->gr_name);
	dest->gr_gid  = src->gr_gid;
	dest->be_priv = NULL;
	return;
}

EXPORT_SYMBOL struct vxpdb_group *vxpdb_group_dup(const struct vxpdb_group *src)
{
	struct vxpdb_group *dest;
	if ((dest = calloc(1, sizeof(struct vxpdb_group))) == NULL)
		return NULL;
	vxpdb_group_copy(dest, src);
	return dest;
}

EXPORT_SYMBOL void vxpdb_group_free(struct vxpdb_group *group, int heap)
{
	hmc_free(group->gr_name);
	if (heap)
		free(group);
	return;
}

EXPORT_SYMBOL void vxpdb_group_nomodify(struct vxpdb_group *group)
{
	group->gr_name = NULL;
	group->gr_gid  = PDB_NO_CHANGE;
	group->be_priv = NULL;
	return;
}

//=============================================================================
