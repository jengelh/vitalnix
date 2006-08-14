/*=============================================================================
Vitalnix User Management Suite
clutils/userdel.h
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
#ifndef VITALNIX_CLUTILS_USERDEL_LIB_H
#define VITALNIX_CLUTILS_USERDEL_LIB_H 1

#ifdef __cplusplus
extern "C" {
#endif

enum {
    UD_SUCCESS = 0,
    UD_EOTHER,
    UD_ELOAD,
    UD_EOPEN,        // unable to open back-end module or DB
    UD_EQUERY,
    UD_ENOEXIST,     // user does not exist
    UD_EDENY,        // will not remove 'root' or UID 0 without -F (force)
    UD_EUPDATE,      // db->userdel() did not return ok
    UD_EPOST,
    UD_ECLOSE,

    UD_SHIFT = 16,
    UD_MASK  = 0xF0000,
};

struct userdel_state {
    char *username;
    const char *ac_after, *ac_before, *db_module;
    int force, rm_cron, rm_home, rm_mail;
};

extern int userdel_fill_defaults(struct userdel_state *);
extern int userdel_get_options(int *, const char ***, struct userdel_state *);
extern int userdel_run(struct userdel_state *);
extern const char *userdel_strerror(int);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VITALNIX_CLUTILS_USERDEL_LIB_H

//=============================================================================
