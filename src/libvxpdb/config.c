/*
 *	libvxpdb/config.c - Common config interface for CLUTILS
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxpdb/config.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Functions */
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
static void parse_group(const struct HXoptcb *info)
{
	struct vxpdb_user *user = info->current->uptr;
	if (vxutil_only_digits(info->data))
		user->pw_gid = strtol(info->data, NULL, 0);
	else
		user->pw_igrp = HX_strdup(info->data);
	/* FIXME: complete this */
	return;
}

//=============================================================================
