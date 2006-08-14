/*=============================================================================
Vitalnix User Management Suite
libvxplex/libvxplex.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2006
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
#ifndef _VITALNIX_LIBVXPLEX_H
#define _VITALNIX_LIBVXPLEX_H 1

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PLEXUI_AUTO = 0,
    PLEXUI_NIO,
    PLEXUI_CLI,
    PLEXUI_GUI,
};

extern int vxplex_enter(const char *, const char *, int, const char **, void *);
extern unsigned int vxplex_select_ui(int *, const char ***);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXPLEX_H

//=============================================================================
