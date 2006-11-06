/*=============================================================================
Vitalnix User Management Suite
libvxpdb/config.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006
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
#include <libHX.h>
#include <vitalnix/compiler.h>
#include "libvxpdb/config.h"
#include "libvxpdb/libvxpdb.h"
#include <vitalnix/libvxutil/libvxutil.h>

// Functions
static void parse_group(const struct HXoptcb *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int vxconfig_read_useradd(const char *file,
  struct vxconfig_useradd *c)
{
    struct HXoption useradd_table[] = {
        {.ln = "MASTER_PREADD",     .type = HXTYPE_STRING, .ptr = &c->master_preadd},
        {.ln = "MASTER_POSTADD",    .type = HXTYPE_STRING, .ptr = &c->master_postadd},
        {.ln = "USER_PREADD",       .type = HXTYPE_STRING, .ptr = &c->user_preadd},
        {.ln = "USER_POSTADD",      .type = HXTYPE_STRING, .ptr = &c->user_postadd},

        {.ln = "HOME",              .type = HXTYPE_STRING, .ptr = &c->home_base},
        {.ln = "SKEL",              .type = HXTYPE_STRING, .ptr = &c->skel_dir},

        {.ln = "CREATE_HOME",       .type = HXTYPE_NONE,   .ptr = &c->create_home},
        {.ln = "SPLIT_LEVEL",       .type = HXTYPE_UCHAR,  .ptr = &c->split_level},
        {.ln = "UMASK",             .type = HXTYPE_LONG,   .ptr = &c->umask},

        {.ln = "GROUP",             .type = HXTYPE_STRING, .cb  = parse_group, .uptr = &c->defaults},
        {.ln = "PASS_EXPIRE",       .type = HXTYPE_LONG,   .ptr = &c->defaults.sp_expire},
        {.ln = "PASS_INACTIVE",     .type = HXTYPE_LONG,   .ptr = &c->defaults.sp_inact},
        {.ln = "PASS_KEEP_MAX",     .type = HXTYPE_LONG,   .ptr = &c->defaults.sp_max},
        {.ln = "PASS_KEEP_MIN",     .type = HXTYPE_LONG,   .ptr = &c->defaults.sp_min},
        {.ln = "PASS_WARN_AGE",     .type = HXTYPE_LONG,   .ptr = &c->defaults.sp_warn},
        {.ln = "SHELL",             .type = HXTYPE_STRING, .ptr = &c->defaults.pw_shell},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(file, useradd_table);
}

EXPORT_SYMBOL int vxconfig_read_usermod(const char *file,
  struct vxconfig_usermod *c)
{
    struct HXoption usermod_table[] = {
        {.ln = "USER_PREMOD",    .type = HXTYPE_STRING, .ptr = &c->user_premod},
        {.ln = "USER_POSTMOD",   .type = HXTYPE_STRING, .ptr = &c->user_postmod},
        {.ln = "MASTER_PREMOD",  .type = HXTYPE_STRING, .ptr = &c->master_premod},
        {.ln = "MASTER_POSTMOD", .type = HXTYPE_STRING, .ptr = &c->master_postmod},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(file, usermod_table);
}

EXPORT_SYMBOL int vxconfig_read_userdel(const char *file,
  struct vxconfig_userdel *c)
{
    struct HXoption userdel_table[] = {
        {.ln = "USER_PREDEL",    .type = HXTYPE_STRING, .ptr = &c->user_predel},
        {.ln = "USER_POSTDEL",   .type = HXTYPE_STRING, .ptr = &c->user_postdel},
        {.ln = "MASTER_PREDEL",  .type = HXTYPE_STRING, .ptr = &c->master_predel},
        {.ln = "MASTER_POSTDEL", .type = HXTYPE_STRING, .ptr = &c->master_postdel},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(file, userdel_table);
}

//-----------------------------------------------------------------------------
static void parse_group(const struct HXoptcb *info) {
    struct vxpdb_user *user = info->current->uptr;
    if(vxutil_only_digits(info->s))
        user->pw_gid = strtol(info->s, NULL, 0);
    else
        user->pw_igrp = HX_strdup(info->s);
    // FIXME: complete this
    return;
}

//=============================================================================
