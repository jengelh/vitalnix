/*=============================================================================
Vitalnix User Management Suite
libvxpdb/config.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007
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
#ifndef _VITALNIX_LIBVXPDB_CONFIG_H
#define _VITALNIX_LIBVXPDB_CONFIG_H 1

#include <vitalnix/libvxpdb/libvxpdb.h>

#ifdef __cplusplus
extern "C" {
#endif

struct vxconfig_useradd {
    char *master_preadd, *master_postadd, *user_preadd, *user_postadd,
         *default_group, *home_base, *skel_dir;
    unsigned int create_home, split_level, umask;

    // pw_igrp, pw_shell and sp_*
    struct vxpdb_user defaults;
};

struct vxconfig_usermod { // currently unused
    char *master_premod, *master_postmod, *user_premod, *user_postmod;
};

struct vxconfig_userdel {
    char *master_predel, *master_postdel, *user_predel, *user_postdel;
};

/*
 *      CONFIG.C
 */
extern int vxconfig_read_useradd(const char *, struct vxconfig_useradd *);
extern int vxconfig_read_usermod(const char *, struct vxconfig_usermod *);
extern int vxconfig_read_userdel(const char *, struct vxconfig_userdel *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXPDB_CONFIG_H

//=============================================================================
