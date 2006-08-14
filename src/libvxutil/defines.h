/*=============================================================================
Vitalnix User Management Suite
libvxutil/defines.h
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
#ifndef _VITALNIX_LIBVXUTIL_DEFINES_H
#define _VITALNIX_LIBVXUTIL_DEFINES_H 1

#ifndef _WIN32 // Win32 specific hacks/defines
#    define stricmp strcasecmp
#    define strnicmp strncasecmp
#endif

#define __STRINGIFY_EXPAND(s)   #s
#define __STRINGIFY(s)          __STRINGIFY_EXPAND(s)
#define ARRAY_SIZE(a)           (sizeof(a) / sizeof(*a))
#define MAXSNLEN                256
#define MAXFNLEN                256
#define MAXLNLEN                1024

#endif // _VITALNIX_LIBVXUTIL_DEFINES_H

//=============================================================================
