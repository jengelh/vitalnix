/*=============================================================================
Vitalnix User Management Suite
libvxpdb/aux.c - auxiliary functions
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2007
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#include <sys/types.h>
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
    memset(u, 0, sizeof(struct vxpdb_user));
    u->pw_uid    = PDB_NOUID;
    u->pw_gid    = PDB_NOGID;
    u->sp_min    = PDB_DFL_KEEPMIN;
    u->sp_max    = PDB_DFL_KEEPMAX;
    u->sp_warn   = PDB_DFL_WARNAGE;
    u->sp_expire = PDB_NO_EXPIRE;
    u->sp_inact  = PDB_NO_INACTIVE;
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
    dest->sp_lastchg = src->sp_lastchg;
    dest->sp_min     = src->sp_min;
    dest->sp_max     = src->sp_max;
    dest->sp_warn    = src->sp_warn;
    dest->sp_expire  = src->sp_expire;
    dest->sp_inact   = src->sp_inact;
    dest->vs_defer   = src->vs_defer;
    hmc_strasg(&dest->vs_uuid, src->vs_uuid);
    hmc_strasg(&dest->vs_pvgrp, src->vs_pvgrp);
    dest->be_priv    = NULL;
    return;
}

EXPORT_SYMBOL struct vxpdb_user *vxpdb_user_dup(const struct vxpdb_user *src)
{
    struct vxpdb_user *dest;
    if((dest = calloc(1, sizeof(struct vxpdb_user))) == NULL)
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
    hmc_free(user->vs_uuid);
    hmc_free(user->vs_pvgrp);
    if(heap)
        free(user);
    return;
}

EXPORT_SYMBOL int vxpdb_user_match(const struct vxpdb_user *user,
  const struct vxpdb_user *mask)
{
    return
      (mask->pw_name == NULL || strcmp(user->pw_name, mask->pw_name) == 0) &&
      (mask->pw_uid == PDB_NOUID || user->pw_uid == mask->pw_uid) &&
      (mask->pw_gid == PDB_NOGID || user->pw_gid == mask->pw_gid) &&
      (mask->pw_real == NULL || strcmp(user->pw_real, mask->pw_real) == 0) &&
      (mask->pw_home == NULL || strcmp(user->pw_home, mask->pw_home) == 0) &&
      (mask->pw_shell == NULL || strcmp(user->pw_shell, mask->pw_shell) == 0);
    // lacks comparing mask->{sp,md}_*
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
    return;
}

EXPORT_SYMBOL void vxpdb_group_clean(struct vxpdb_group *group)
{
    group->gr_name = NULL;
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
    if((dest = calloc(1, sizeof(struct vxpdb_group))) == NULL)
        return NULL;
    vxpdb_group_copy(dest, src);
    return dest;
}

EXPORT_SYMBOL void vxpdb_group_free(struct vxpdb_group *group, int heap)
{
    hmc_free(group->gr_name);
    if(heap)
        free(group);
    return;
}

EXPORT_SYMBOL int vxpdb_group_match(const struct vxpdb_group *group,
 const struct vxpdb_group *mask)
{
    return
      (mask->gr_name == NULL || strcmp(group->gr_name, mask->gr_name) == 0) &&
      (mask->gr_gid == PDB_NOGID || group->gr_gid == mask->gr_gid);
}

EXPORT_SYMBOL void vxpdb_group_nomodify(struct vxpdb_group *group)
{
    group->gr_name = NULL;
    group->gr_gid  = PDB_NO_CHANGE;
    group->be_priv = NULL;
    return;
}

//=============================================================================
