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
#ifndef CSPARK_MAIN_H
#define CSPARK_MAIN_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "include/accdb.h"

enum {
    AFLUSH_OFF = 0,
    AFLUSH_ON,
    AFLUSH_DEFAULT,
    AFLUSH_POSTADD,
};

struct optmap_s {
    // general
    char *f_shell, // default shell for new users
         *f_skeld, // path to lockfile and skeleton dir (if any)
         *f_home;  // home base
    char b_pwphon; // use phonetic passwords
    int i_pswdlen;
    char c_aflush, c_crymeth, c_split;
    long umask;

    // main.c
    char *progv, *progd, *f_lock, *module_path;
    char nterm;

    // sync.c
    char *master_preadd, *master_postadd, *master_predel,
     *master_postdel, *user_preadd, *user_postadd, *user_predel,
     *user_postdel;
    struct adb_user default_times;
};

extern struct optmap_s Opt;
extern struct adb_module *mm_output;

extern int B_OPEN(flag_t);
#define B_USERTRAV_INIT(p) (mm_output->usertrav(mm_output, (p), NULL))
#define B_USERTRAV(tp, req) (mm_output->usertrav(mm_output, (tp), (req)))
#define B_USERINFO(req, dest, s) \
     (mm_output->userinfo(mm_output, (req), (dest), (s)))
#define B_USERADD(req) (mm_output->useradd(mm_output, (req)))
#define B_USERMOD(s, m) (mm_output->usermod(mm_output, (s), (m)))
#define B_USERDEL(req) (mm_output->userdel(mm_output, (req)))
#define B_GROUPINFO(req, dest, s) \
     (mm_output->groupinfo(mm_output, (req), (dest), (s)))
#define B_MODCTL(req, args...) \
     (mm_output->modctl(mm_output, (req), mm_output, ## args))
extern void B_CLOSE(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSPARK_MAIN_H

//=============================================================================
