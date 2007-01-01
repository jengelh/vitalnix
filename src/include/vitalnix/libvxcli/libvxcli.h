/*=============================================================================
Vitalnix User Management Suite
libvxcli/libvxcli.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2007
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
#ifndef _VITALNIX_LIBVXCLI_LIBVXCLI_H
#define _VITALNIX_LIBVXCLI_LIBVXCLI_H 1

#include <sys/types.h>
#ifdef __cplusplus
#    include <cstdio>
#else
#    include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#    define VXCQ_TABLE_END {NULL, NULL}
#else
#    define VXCQ_TABLE_END {.msg = NULL, .prompt = NULL}
#endif

enum {
    VXCQ_NONE  = 0,
    VXCQ_ABORT = 1 << 0, // allow abort
    VXCQ_EMPTY = 1 << 1, // allow empty
    VXCQ_ZNULL = 1 << 2, // empty answer will put NULL into *ptr
};

struct vxcq_entry {
    const char *msg, *prompt, *defl;
    int type;
    void *ptr, *uptr;
    long flags;
    int (*validate)(const struct vxcq_entry *);
};

extern char *vxcli_query(const char *, const char *, const char *, long,
    char *, size_t);
extern int vxcli_query_v(const struct vxcq_entry *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXCLI_LIBVXCLI_H

//=============================================================================
