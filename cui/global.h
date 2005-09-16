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
#ifndef CSPARK_GLOBAL_H
#define CSPARK_GLOBAL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define LOGFMT_VERSION "0" // increment whenever ... the format changes
#define MAXFNLEN 256       // max length for filename buffer
#define MAXLNLEN 1024      // max length for line buffer
#define MAXSNLEN 128       // max length for a short line

#ifndef _WIN32 // Win32 specific hacks/defines
#    define stricmp strcasecmp
#    define strnicmp strncasecmp
#endif

typedef int flag_t;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSPARK_GLOBAL_H

//=============================================================================
