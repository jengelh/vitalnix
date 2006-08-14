/*=============================================================================
Vitalnix User Management Suite
libvxpdb/libvxpdb.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2006
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
#ifndef _VITALNIX_LIBVXPDB_LIBVXPDB_H
#define _VITALNIX_LIBVXPDB_LIBVXPDB_H 1

#include <sys/types.h>
#ifndef __cplusplus
#    include <stdlib.h>
#else
#    include <cstdlib>
#endif
#include <libHX.h>

#ifdef __cplusplus
extern "C" {
#endif

// Definitions
enum {
    PDB_NO_CHANGE   = -2,
    PDB_AUTOUID     = -1,
    PDB_AUTOUID_SYS = -2,
    PDB_AUTOGID     = -1,
    PDB_AUTOGID_SYS = -2,
    PDB_NOUID       = -1,
    PDB_NOGID       = -1,
    PDB_NO_EXPIRE   = -1,
    PDB_NO_INACTIVE = -1,
    PDB_DFL_KEEPMIN = 0,
    PDB_DFL_KEEPMAX = 10000,
    PDB_DFL_WARNAGE = 21,

    // Flags for pdb_open() and pdb_modctl(command=PDB_ADDFLAGS/PDB_DELFLAGS)
    PDB_SYNC   = 1 << 0,
    PDB_WRLOCK = 1 << 1,

    // pdb_modctl commands
    PDB_ADDFLAGS = 0x1,
    PDB_DELFLAGS,
    PDB_FLUSH,
    PDB_COUNT_USERS,
    PDB_COUNT_GROUPS,
    PDB_NEXTUID_SYS,
    PDB_NEXTUID,
    PDB_NEXTGID_SYS,
    PDB_NEXTGID,
};

struct vxconfig_useradd;
struct vxconfig_usermod;
struct vxconfig_userdel;

struct vxpdb_user {
    // passwd part
    char *pw_name, *pw_real, *pw_home, *pw_shell, *pw_igrp;
    long pw_uid, pw_gid;
    struct HXdeque *pw_sgrp;

    // shadow part
    char *sp_passwd;
    long sp_lastchg, sp_min, sp_max, sp_warn, sp_expire, sp_inact;

    // vxshadow part (vs_), used by libvxmdsync
    char *vs_uuid, *vs_pvgrp;
    long vs_defer; // day on which tagging occurred, not when deletion is due

    // backend-specific private stuff
    void *be_priv;
};

struct vxpdb_group {
    char *gr_name;
    long gr_gid;
    void *be_priv;

    // internal data storage
    hmc_t *_storage;
};

struct vxpdb_mvtable;
struct vxpdb_state {
    void *handle, *state;
    const struct vxpdb_mvtable *vtable;
};

struct vxpdb_mvtable {
    const char *name, *desc, *author;

    int (*init)(struct vxpdb_state *, const char *);
    int (*open)(struct vxpdb_state *, long);
    void (*close)(struct vxpdb_state *);
    void (*deinit)(struct vxpdb_state *);

    long (*modctl)(struct vxpdb_state *, long, ...);
    int (*lock)(struct vxpdb_state *);
    int (*unlock)(struct vxpdb_state *);

    int (*useradd)(struct vxpdb_state *, const struct vxpdb_user *);
    int (*usermod)(struct vxpdb_state *, const struct vxpdb_user *, const struct vxpdb_user *);
    int (*userdel)(struct vxpdb_state *, const struct vxpdb_user *);
    int (*userinfo)(struct vxpdb_state *, const struct vxpdb_user *, struct vxpdb_user *, size_t);
    void *(*usertrav_init)(struct vxpdb_state *);
    int (*usertrav_walk)(struct vxpdb_state *, void *, struct vxpdb_user *);
    void (*usertrav_free)(struct vxpdb_state *, void *);

    int (*groupadd)(struct vxpdb_state *, const struct vxpdb_group *);
    int (*groupmod)(struct vxpdb_state *, const struct vxpdb_group *, const struct vxpdb_group *);
    int (*groupdel)(struct vxpdb_state *, const struct vxpdb_group *);
    int (*groupinfo)(struct vxpdb_state *, const struct vxpdb_group *, struct vxpdb_group *, size_t);
    void *(*grouptrav_init)(struct vxpdb_state *);
    int (*grouptrav_walk)(struct vxpdb_state *, void *, struct vxpdb_group *);
    void (*grouptrav_free)(struct vxpdb_state *, void *);
};

/*
 *      AUX.C
 */
extern void *vxpdb_user_alloc(struct vxpdb_user *, size_t);
extern void vxpdb_user_clean(struct vxpdb_user *);
extern void vxpdb_user_copy(struct vxpdb_user *, const struct vxpdb_user *);
extern struct vxpdb_user *vxpdb_user_dup(const struct vxpdb_user *);
extern void vxpdb_user_free(struct vxpdb_user *, int);
extern void vxpdb_user_nomodify(struct vxpdb_user *);
extern int vxpdb_user_match(const struct vxpdb_user *, const struct vxpdb_user *);

extern void *vxpdb_group_alloc(struct vxpdb_group *, size_t);
extern void vxpdb_group_clean(struct vxpdb_group *);
extern void vxpdb_group_copy(struct vxpdb_group *, const struct vxpdb_group *);
extern struct vxpdb_group *vxpdb_group_dup(const struct vxpdb_group *);
extern void vxpdb_group_free(struct vxpdb_group *, int);
extern int vxpdb_group_match(const struct vxpdb_group *, const struct vxpdb_group *);
extern void vxpdb_group_nomodify(struct vxpdb_group *);

/*
 *      CONFIG.C
 */
extern int vxconfig_read_useradd(const char *, struct vxconfig_useradd *);
extern int vxconfig_read_usermod(const char *, struct vxconfig_usermod *);
extern int vxconfig_read_userdel(const char *, struct vxconfig_userdel *);

/*
 *      DUMMY.C
 */
extern void vxpdb_fix_vtable(struct vxpdb_mvtable *);

/*
 *      PDB.C
 */
extern struct vxpdb_state *vxpdb_load(const char *);
extern void vxpdb_unload(struct vxpdb_state *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXPDB_LIBVXPDB_H

//=============================================================================