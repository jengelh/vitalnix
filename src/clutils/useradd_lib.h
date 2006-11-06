/*=============================================================================
Vitalnix User Management Suite
clutils/useradd_lib.h
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
#ifndef VITALNIX_CLUTILS_USERADD_LIB_H
#define VITALNIX_CLUTILS_USERADD_LIB_H 1

#include <vitalnix/libvxpdb/libvxpdb.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    UA_SUCCESS = 0,
    UA_EOTHER,
    UA_ELOAD,
    UA_EOPEN,     // unable to open back-end module or DB
    UA_EQUERY,
    UA_EUIDUSED,  // UID already used and -o was not specified
    UA_ENOSYS,
    UA_ENAMEUSED, // user already exists
    UA_EUPDATE,   // db->useradd() did not return ok
    UA_EPOST,     // post-stuff failed (home directory, skeleton copying)
    UA_ECLOSE,

    UA_SHIFT = 16,
    UA_MASK  = 0xF0000,
};

struct useradd_state {
    struct vxpdb_user user;

    // internal
    const char *ac_after, *ac_before, *db_module, *homebase, *skeldir;
    unsigned long umask;
    int allow_dup, create_home, force, split_lvl, sys_uid;
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
