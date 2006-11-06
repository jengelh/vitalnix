/*=============================================================================
Vitalnix User Management Suite
libvxpdb/xwfunc.h - simple wrappers
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006
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
#ifndef _VITALNIX_LIBVXPDB_XWFUNC_H
#define _VITALNIX_LIBVXPDB_XWFUNC_H 1

#include <sys/types.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *      INLINE FUNCTIONS
 */
static inline int vxpdb_getpwnam(struct vxpdb_state *state, const char *user,
  struct vxpdb_user *result)
{
    struct vxpdb_user mask;
    vxpdb_user_clean(&mask);
    mask.pw_name = const_cast(char *, user);
    return vxpdb_userinfo(state, &mask, result, result != NULL);
}

static inline int vxpdb_getpwuid(struct vxpdb_state *state, long uid,
  struct vxpdb_user *result)
{
    struct vxpdb_user mask;
    vxpdb_user_clean(&mask);
    mask.pw_uid = uid;
    return vxpdb_userinfo(state, &mask, result, result != NULL);
}

static inline int vxpdb_getgrnam(struct vxpdb_state *state, const char *group,
  struct vxpdb_group *result)
{
    struct vxpdb_group mask;
    mask.gr_name = const_cast(char *, group);
    mask.gr_gid  = PDB_NOGID;
    return vxpdb_groupinfo(state, &mask, result, result != NULL);
}

static inline int vxpdb_getgrgid(struct vxpdb_state *state, long gid,
  struct vxpdb_group *result)
{
    struct vxpdb_group mask;
    mask.gr_name = NULL;
    mask.gr_gid  = gid;
    return vxpdb_groupinfo(state, &mask, result, result != NULL);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXPDB_XWFUNC_H

//=============================================================================
