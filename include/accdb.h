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
#ifndef _ACCDB_H
#define _ACCDB_H 1

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    // am_open(), am_modctl() flags
    ADB_SYNC   = 1 << 0,
    ADB_WRLOCK = 1 << 1,

    // Modctl commands
    ADB_ADDFLAGS = 0x1,
    ADB_DELFLAGS,
    ADB_FLUSH,
    ADB_COUNT_USERS,
    ADB_COUNT_GROUPS,
    ADB_NEXTUID_SYS,
    ADB_NEXTUID,
    ADB_NEXTGID_SYS,
    ADB_NEXTGID,
    ADB_MODSPEC = 0x100,

    //
    ADB_KEEPMAX_DEFL   = 10000,
    ADB_WARNAGE_DEFL   = 30,
    ADB_EXPIRE_NEVER   = -2,
    ADB_INACTIVE_NEVER = -2,

    // genpw stuff
    GENPW_PHONEMIC  = 1 << 0,
    GENPW_ONE_DIGIT = 1 << 1,
    GENPW_ONE_CASE  = 1 << 2,
    CRYPW_DES = 1,
    CRYPW_MD5,
    CRYPW_BLOWFISH,
};

struct adb_user {
    // passwd part
    char *lname;
    long uid, gid;
    char *gecos, *home, *shell, *igrp, *sgrp;

    // shadow part
    char *passwd;
    long last_change, keep_min, keep_max, warn_age, expire, inactive;

    // vitalnix part
    char *xuid, *sgroup;
    long defer_del;

    // backend-specific private stuff
    void *priv;
};

struct adb_group {
    char *gname;
    long gid;
    void *priv;
};

struct adb_module {
    const char *name, *desc, *info;
    void *handle, *state;
    int (*init)(struct adb_module *, void *);
    int (*open)(struct adb_module *, long);
    int (*lock)(struct adb_module *);
    int (*unlock)(struct adb_module *);
    int (*close)(struct adb_module *);
    long (*modctl)(struct adb_module *, long, ...);
    void (*deinit)(struct adb_module *);

    int (*useradd)(struct adb_module *, struct adb_user *);
    int (*usermod)(struct adb_module *, struct adb_user *, struct adb_user *);
    int (*userdel)(struct adb_module *, struct adb_user *);
    int (*usertrav)(struct adb_module *, void **, struct adb_user *);
    int (*userinfo)(struct adb_module *, struct adb_user *, struct adb_user *, size_t);

    int (*groupadd)(struct adb_module *, struct adb_group *);
    int (*groupmod)(struct adb_module *, struct adb_group *, struct adb_group *);
    int (*groupdel)(struct adb_module *, struct adb_group *);
    int (*grouptrav)(struct adb_module *, void **, struct adb_group *);
    int (*groupinfo)(struct adb_module *, struct adb_group *, struct adb_group *, size_t);
};

struct accdb_cryptinfo {
    // Input fields
    unsigned char *key;
    int method;

    // Output fields
    unsigned char *crypted;
};

extern struct adb_module *adb_load(const char *, void *);
extern void adb_unload(struct adb_module *);
extern int vx_genpw(char *, size_t, unsigned long);
extern int vx_cryptpw(const char *, const char *, int, char **);
extern int genpw_phonemic(char *, size_t, unsigned long, void *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _ACCDB_H

//=============================================================================
