/*=============================================================================
Vitalnix User Management Suite
clutils/usermod_lib.h
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
#ifndef VITALNIX_CLUTILS_USERMOD_LIB_H
#define VITALNIX_CLUTILS_USERMOD_LIB_H 1

#include <vitalnix/libvxpdb/config.h>
#include <vitalnix/libvxpdb/libvxpdb.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    UM_SUCCESS = 0,
    UM_EOTHER,       // other error, see errno
    UM_ELOAD,
    UM_EOPEN,        // unable to open back-end module or DB
    UM_EQUERY,       // querying PDB
    UM_ENOEXIST,     // user does not exist
    UM_EUIDUSED,     // UID already used and -o was not specified
    UM_ENAMEUSED,    // new user name (-n) already exists
    UM_EUPDATE,      // db->usermod() did not return ok
    UM_EPOST,        // moving home/etc.
    UM_ECLOSE,       // db->close() did not return ok

    UM_SHIFT = 16,
    UM_MASK  = 0xF0000,
};

struct usermod_state {
    char *username;
    struct vxpdb_user newstuff;
    struct vxconfig_usermod config;
    const char *database;
    int allow_dup, lock_account, move_home;
};

extern int usermod_fill_defaults(struct usermod_state *);
extern int usermod_get_options(int *, const char ***, struct usermod_state *);
extern int usermod_run(struct usermod_state *);
extern const char *usermod_strerror(int);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VITALNIX_CLUTILS_USERMOD_LIB_H

//=============================================================================
