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
#ifndef CSPARK_DATA_H
#define CSPARK_DATA_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include "cui/global.h"
#include "cui/xml_in.h"

extern char *day_to_xuid(const char *, char *, size_t);
extern int compare_wbc(const char *, const char *);
extern void complete_pentry(struct eds_entry *, char *, size_t);
extern int file_exists(const char *);
extern char *fixup_login_name(const char *, unsigned int, char *, size_t);
extern char *make_login_name(const char *, const char *);
extern long now_in_days(void);
extern char *now_in_ymdhms(char *, size_t);
extern flag_t only_digits(const char *);
extern char *read_file(const char *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSPARK_DATA_H

//=============================================================================
