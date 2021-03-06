/*
 *	nss1.c - Example NSS backend
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2005 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <shadow.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Functions */
static unsigned int count_users(void);
static unsigned int count_groups(void);
static void nssuser_copy(struct vxdb_user *, const struct passwd *, const struct spwd *);
static void nssgroup_copy(struct vxdb_group *, const struct group *);

//-----------------------------------------------------------------------------
static long vxnss1_modctl(struct vxdb_state *this, unsigned int command, ...)
{
	errno = 0;
	switch (command) {
		case VXDB_COUNT_USERS:
			return count_users();
		case VXDB_COUNT_GROUPS:
			return count_groups();
	}
	return -ENOSYS;
}

static int vxnss1_getpwuid(struct vxdb_state *this, unsigned int uid,
    struct vxdb_user *dest)
{
	struct passwd *p;

	errno = 0;
	if ((p = getpwuid(uid)) == NULL)
		return -errno;

	if (dest != NULL)
		nssuser_copy(dest, p, getspnam(p->pw_name));
	return 1;
}

static int vxnss1_getpwnam(struct vxdb_state *this, const char *name,
    struct vxdb_user *dest)
{
	struct passwd *p;

	errno = 0;
	if ((p = getpwnam(name)) == NULL)
		return -errno;

	if (dest != NULL)
		nssuser_copy(dest, p, getspnam(name));
	return 1;
}

static void *vxnss1_usertrav_init(struct vxdb_state *this)
{
	setpwent();
	return this;
}

static int vxnss1_usertrav_walk(struct vxdb_state *this, void *priv_data,
    struct vxdb_user *dest)
{
	struct passwd *pe;
	struct spwd *se;

	errno = 0;
	if ((pe = getpwent()) == NULL)
		/* if errno=0, we return 0, which is fine */
		return -errno;

	vxdb_user_clean(dest);
	if (*pe->pw_name == '+' || *pe->pw_name == '-')
		++pe->pw_name;

	se = getspnam(pe->pw_name);
	nssuser_copy(dest, pe, se);
	return 1;
}

static void vxnss1_usertrav_free(struct vxdb_state *this, void *priv_data)
{
	endpwent();
}

static int vxnss1_getgrgid(struct vxdb_state *this, unsigned int gid,
    struct vxdb_group *dest)
{
	struct group *g;

	errno = 0;
	if ((g = getgrgid(gid)) == NULL)
		return -errno;

	if (dest != NULL)
		nssgroup_copy(dest, g);
	return 1;
}

static int vxnss1_getgrnam(struct vxdb_state *this, const char *name,
    struct vxdb_group *dest)
{
	struct group *g;

	errno = 0;
	if ((g = getgrnam(name)) == NULL)
		return -errno;

	if (dest != NULL)
		nssgroup_copy(dest, g);
	return 1;
}

static void *vxnss1_grouptrav_init(struct vxdb_state *this)
{
	setgrent();
	return this;
}

static int vxnss1_grouptrav_walk(struct vxdb_state *this, void *priv_data,
    struct vxdb_group *dest)
{
	struct group *gr;

	errno = 0;
	if ((gr = getgrent()) == NULL)
		return -errno;

	nssgroup_copy(dest, gr);
	return 1;
}

static void vxnss1_grouptrav_free(struct vxdb_state *this, void *priv_data)
{
	endgrent();
}

//-----------------------------------------------------------------------------
static unsigned int count_users(void)
{
	unsigned int n = 0;

	for (setpwent(); getpwent() != NULL; ++n)
		;
	endpwent();
	return n;
}

static unsigned int count_groups(void)
{
	unsigned int n = 0;

	for (setgrent(); getgrent() != NULL; ++n)
		;
	endpwent();
	return n;
}

static void nssuser_copy(struct vxdb_user *dest, const struct passwd *pe,
    const struct spwd *se)
{
	HX_strclone(&dest->pw_name, pe->pw_name);
	dest->pw_uid     = pe->pw_uid;
	dest->pw_gid     = pe->pw_gid;
	HX_strclone(&dest->pw_real, pe->pw_gecos);
	HX_strclone(&dest->pw_home, pe->pw_dir);
	HX_strclone(&dest->pw_shell, pe->pw_shell);

	if (se != NULL) {
		HX_strclone(&dest->sp_passwd, se->sp_pwdp);
		dest->sp_lastchg = se->sp_lstchg;
		dest->sp_min     = se->sp_min;
		dest->sp_max     = se->sp_max;
		dest->sp_warn    = se->sp_warn;
		dest->sp_expire  = se->sp_expire;
		dest->sp_inact   = se->sp_inact;
	}

	dest->vs_uuid    = NULL;
	dest->vs_pvgrp   = NULL;
	dest->vs_defer   = 0;
	dest->be_priv    = NULL;
}

static void nssgroup_copy(struct vxdb_group *dest, const struct group *src)
{
	HX_strclone(&dest->gr_name, src->gr_name);
	dest->gr_gid  = src->gr_gid;
	dest->be_priv = NULL;
}

EXPORT_SYMBOL struct vxdb_driver THIS_MODULE = {
	.name           = "NSS back-end module (not MU/MT-safe)",
	.desc           = "API demonstration",
	.modctl         = vxnss1_modctl,
	.getpwuid       = vxnss1_getpwuid,
	.getpwnam       = vxnss1_getpwnam,
	.usertrav_init  = vxnss1_usertrav_init,
	.usertrav_walk  = vxnss1_usertrav_walk,
	.usertrav_free  = vxnss1_usertrav_free,
	.getgrgid       = vxnss1_getgrgid,
	.getgrnam       = vxnss1_getgrnam,
	.grouptrav_init = vxnss1_grouptrav_init,
	.grouptrav_walk = vxnss1_grouptrav_walk,
	.grouptrav_free = vxnss1_grouptrav_free,
};
