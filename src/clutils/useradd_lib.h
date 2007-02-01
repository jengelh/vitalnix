/*=============================================================================
Vitalnix User Management Suite
clutils/useradd_lib.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2007
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
#ifndef VITALNIX_CLUTILS_USERADD_LIB_H
#define VITALNIX_CLUTILS_USERADD_LIB_H 1

#include <vitalnix/libvxpdb/config.h>
#include <vitalnix/libvxpdb/libvxpdb.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    E_SUCCESS = 0,
    E_OTHER,
    E_OPEN,
    E_UID_USED,
    E_NAME_USED,
    E_UPDATE,
    E_POST,
};

struct useradd_state {
    struct vxconfig_useradd config;
    const char *database;
    int allow_dup, force, sys_uid;
};

extern int useradd_fill_defaults(struct useradd_state *);
extern char *useradd_genhome(struct useradd_state *);
extern int useradd_get_options(int *, const char ***, struct useradd_state *);
extern int useradd_run(struct useradd_state *);
extern const char *useradd_strerror(int);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VITALNIX_CLUTILS_USERADD_LIB_H

//=============================================================================
