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
