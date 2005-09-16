/*=============================================================================
Vitalnix User Management Suite
  Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2003 - 2005
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#ifndef _ACCDB_INT_H
#define _ACCDB_INT_H 1

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#    define MODULE_NAME(s) extern "C" const char _module_name[] = (s)
#    define MODULE_DESC(s) extern "C" const char _module_desc[] = (s)
#    define MODULE_INFO(s) extern "C" const char _module_info[] = (s)
#else
#    define MODULE_NAME(s) const char _module_name[] = (s)
#    define MODULE_DESC(s) const char _module_desc[] = (s)
#    define MODULE_INFO(s) const char _module_info[] = (s)
#endif

extern int am_init(struct adb_module *, void *);
extern int am_open(struct adb_module *, long);
extern int am_lock(struct adb_module *);
extern int am_unlock(struct adb_module *);
extern int am_close(struct adb_module *);
extern long am_modctl(struct adb_module *, long, ...);
extern void am_deinit(struct adb_module *);

extern int am_useradd(struct adb_module *, struct adb_user *);
extern int am_usermod(struct adb_module *, struct adb_user *, struct adb_user *);
extern int am_userdel(struct adb_module *, struct adb_user *);
extern int am_usertrav(struct adb_module *, void **, struct adb_user *);
extern int am_userinfo(struct adb_module *, struct adb_user *, struct adb_user *, size_t);

extern int am_groupadd(struct adb_module *, struct adb_group *);
extern int am_groupmod(struct adb_module *, struct adb_group *, struct adb_group *);
extern int am_groupdel(struct adb_module *, struct adb_group *);
extern int am_grouptrav(struct adb_module *, void **, struct adb_group *);
extern int am_groupinfo(struct adb_module *, struct adb_group *, struct adb_group *, size_t);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _ACCDB_INT_H

//=============================================================================
