/*=============================================================================
Vitalnix User Management Suite
drivers/proto.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2006
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
#ifndef _VITALNIX_DRIVERS_PROTO_H
#define _VITALNIX_DRIVERS_PROTO_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct vxpdb_state;
struct vxpdb_user;
struct vxpdb_group;

typedef int (driver_init_t)(struct vxpdb_state *, const char *);
typedef int (driver_open_t)(struct vxpdb_state *, long);
typedef void (driver_close_t)(struct vxpdb_state *);
typedef void (driver_deinit_t)(struct vxpdb_state *);
typedef long (driver_modctl_t)(struct vxpdb_state *, long, ...);
typedef int (driver_lock_t)(struct vxpdb_state *);
typedef int (driver_unlock_t)(struct vxpdb_state *);

typedef int (driver_useradd_t)(struct vxpdb_state *, const struct vxpdb_user *);
typedef int (driver_usermod_t)(struct vxpdb_state *, const struct vxpdb_user *, const struct vxpdb_user *);
typedef int (driver_userdel_t)(struct vxpdb_state *, const struct vxpdb_user *);
typedef int (driver_userinfo_t)(struct vxpdb_state *, const struct vxpdb_user *, struct vxpdb_user *, size_t);
typedef void *(driver_usertrav_init_t)(struct vxpdb_state *);
typedef int (driver_usertrav_walk_t)(struct vxpdb_state *, void *, struct vxpdb_user *);
typedef void (driver_usertrav_free_t)(struct vxpdb_state *, void *);

typedef int (driver_groupadd_t)(struct vxpdb_state *, const struct vxpdb_group *);
typedef int (driver_groupmod_t)(struct vxpdb_state *, const struct vxpdb_group *, const struct vxpdb_group *);
typedef int (driver_groupdel_t)(struct vxpdb_state *, const struct vxpdb_group *);
typedef int (driver_groupinfo_t)(struct vxpdb_state *, const struct vxpdb_group *, struct vxpdb_group *, size_t);
typedef void *(driver_grouptrav_init_t)(struct vxpdb_state *);
typedef int (driver_grouptrav_walk_t)(struct vxpdb_state *, void *, struct vxpdb_group *);
typedef void (driver_grouptrav_free_t)(struct vxpdb_state *, void *);

#define DRIVER_PROTO_ALL(name) \
    DRIVER_PROTO_BASE(name); \
    DRIVER_PROTO_USER(name); \
    DRIVER_PROTO_GROUP(name);

#define DRIVER_PROTO_BASE1(name) \
    static driver_init_t name##_init; \
    static driver_open_t name##_open; \
    static driver_close_t name##_close; \
    static driver_deinit_t name##_deinit;

#define DRIVER_PROTO_BASE(name) \
    DRIVER_PROTO_BASE1(name); \
    static driver_modctl_t name##_modctl; \
    static driver_lock_t name##_lock; \
    static driver_unlock_t name##_unlock;

#define DRIVER_PROTO_USER(name) \
    static driver_useradd_t name##_useradd; \
    static driver_usermod_t name##_usermod; \
    static driver_userdel_t name##_userdel; \
    static driver_userinfo_t name##_userinfo; \
    static driver_usertrav_init_t name##_usertrav_init; \
    static driver_usertrav_walk_t name##_usertrav_walk; \
    static driver_usertrav_free_t name##_usertrav_free;

#define DRIVER_PROTO_GROUP(name) \
    static driver_groupadd_t name##_groupadd; \
    static driver_groupmod_t name##_groupmod; \
    static driver_groupdel_t name##_groupdel; \
    static driver_groupinfo_t name##_groupinfo; \
    static driver_grouptrav_init_t name##_grouptrav_init; \
    static driver_grouptrav_walk_t name##_grouptrav_walk; \
    static driver_grouptrav_free_t name##_grouptrav_free;

#define DRIVER_CB_ALL(name) \
    DRIVER_CB_BASE(name), \
    DRIVER_CB_USER(name), \
    DRIVER_CB_GROUP(name)

#ifndef __cplusplus
#    define DRIVER_CB_BASE1(name) \
        .init           = name##_init, \
        .open           = name##_open, \
        .close          = name##_close, \
        .deinit         = name##_deinit
#    define DRIVER_CB_BASE(name) \
        DRIVER_CB_BASE1(name), \
        .modctl         = name##_modctl, \
        .lock           = name##_lock, \
        .unlock         = name##_unlock
#    define DRIVER_CB_USER(name) \
        .useradd        = name##_useradd, \
        .usermod        = name##_usermod, \
        .userdel        = name##_userdel, \
        .userinfo       = name##_userinfo, \
        .usertrav_init  = name##_usertrav_init, \
        .usertrav_walk  = name##_usertrav_walk, \
        .usertrav_free  = name##_usertrav_free
#    define DRIVER_CB_GROUP(name) \
        .groupadd       = name##_groupadd, \
        .groupmod       = name##_groupmod, \
        .groupdel       = name##_groupdel, \
        .groupinfo      = name##_groupinfo, \
        .grouptrav_init = name##_grouptrav_init, \
        .grouptrav_walk = name##_grouptrav_walk, \
        .grouptrav_free = name##_grouptrav_free
#endif // __cplusplus

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_DRIVERS_PROTO_H

//=============================================================================
