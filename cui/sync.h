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
#ifndef CSPARK_SYNC_H
#define CSPARK_SYNC_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <libHX.h>
#include "include/accdb.h"

extern int sync_read_file(const char *, struct HXbtree **, int);
extern int sync_prep_dgrp(struct adb_group *, const char *);
extern void sync_compare(struct adb_group *, struct HXbtree *,
 struct HXdeque **, struct HXbtree **);
extern void sync_fixup_addl(struct HXbtree *, struct HXbtree *);
extern void sync_set_aflush(void);
extern FILE *sync_open_log(const char *, char);
extern int sync_add(struct adb_group *, struct HXbtree *, FILE *);
extern int sync_del(struct HXdeque *, FILE *);
extern void sync_free(struct HXbtree *, struct HXdeque *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSPARK_SYNC_H

//=============================================================================
