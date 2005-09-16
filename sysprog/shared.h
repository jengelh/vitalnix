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
#ifndef VITALNIX_SYSPROG_SHARED_H
#define VITALNIX_SYSPROG_SHARED_H 1

#include <libHX.h>

#define MAXFNLEN 256  // max length for filename buffer
#define MAXLNLEN 1024 // max length for line buffer
#define MAXSNLEN 128  // max length for a short line

#ifndef S_IRGRP
#    define S_IRGRP 0040
#endif
#ifndef S_IWGRP
#    define S_IWGRP 0020
#endif
#ifndef S_IROTH
#    define S_IROTH 0004
#endif
#ifndef S_IWOTH
#    define S_IWOTH 0002
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct itab {
    const char *fmt;
    char type;
    void *ptr;
    int (*callback)(const char *, char, void *);
};

extern int useradd_main(int, const char **);
extern int usermod_main(int, const char **);
extern int userdel_main(int, const char **);
extern int groupadd_main(int, const char **);
extern int groupmod_main(int, const char **);
extern int groupdel_main(int, const char **);

extern void SH_interactive(struct itab *);
extern int SH_runcmd(const char *, struct HX_repmap *);
extern long SH_string_to_iday(const char *);
extern int SH_only_digits(char *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VITALNIX_SYSPROG_SHARED_H

//=============================================================================
