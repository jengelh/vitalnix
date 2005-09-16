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
#ifndef CSPARK_AUTORUN_H
#define CSPARK_AUTORUN_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <libHX.h>

struct pwl_param {
    char *in, *out, *style, *tpl;
};

struct sync_param {
    char *gname, *in, *out;
    int no_add, no_del;
};

struct user_param {
    char *vname, *nname, *xuid, *gname, *sgroup;
};

extern void autorun_pwl(struct pwl_param *);
extern void autorun_sync(struct sync_param *);
extern void autorun_user(struct user_param *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSPARK_AUTORUN_H

//=============================================================================
