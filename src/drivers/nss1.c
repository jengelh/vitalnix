/*
 *	nss1.c - Example NSS backend
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2005 - 2007
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
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Functions */
static unsigned int count_users(void);
static unsigned int count_groups(void);
static void nssuser_copy(struct vxpdb_user *, const struct passwd *, const struct spwd *);
static void nssgroup_copy(struct vxpdb_group *, const struct group *);

//-----------------------------------------------------------------------------
static long vnss1_modctl(struct vxpdb_state *this, unsigned int command, ...)
{
	errno = 0;
	switch (command) {
		case PDB_COUNT_USERS:
			return count_users();
		case PDB_COUNT_GROUPS:
			return count_groups();
	}
	return -ENOSYS;
}

static int vnss1_getpwuid(struct vxpdb_state *this, unsigned int uid,
    struct vxpdb_user *dest)
{
	struct passwd *p;

	errno = 0;
	if ((p = getpwuid(uid)) == NULL)
		return -errno;

	nssuser_copy(dest, p, getspnam(p->pw_name));
	return 1;
}

static int vnss1_getpwnam(struct vxpdb_state *this, const char *name,
    struct vxpdb_user *dest)
{
	struct passwd *p;

	errno = 0;
	if ((p = getpwnam(name)) == NULL)
		return -errno;

	nssuser_copy(dest, p, getspnam(name));
	return 1;
}

static void *vnss1_usertrav_init(struct vxpdb_state *this)
{
	setpwent();
	return this;
}

static int vnss1_usertrav_walk(struct vxpdb_state *this, void *priv_data,
    struct vxpdb_user *dest)
{
	struct passwd *pe;
	struct spwd *se;

	errno = 0;
	if ((pe = getpwent()) == NULL)
		/* if errno=0, we return 0, which is fine */
		return -errno;

	vxpdb_user_clean(dest);
	if (*pe->pw_name == '+' || *pe->pw_name == '-')
		++pe->pw_name;

	se = getspnam(pe->pw_name);
	nssuser_copy(dest, pe, se);
	return 1;
}

static void vnss1_usertrav_free(struct vxpdb_state *this, void *priv_data)
{
	endpwent();
	return;
}

static int vnss1_getgrgid(struct vxpdb_state *this, unsigned int gid,
    struct vxpdb_group *dest)
{
	struct group *g;

	errno = 0;
	if ((g = getgrgid(gid)) == NULL)
		return -errno;

	nssgroup_copy(dest, g);
	return 1;
}

static int vnss1_getgrnam(struct vxpdb_state *this, const char *name,
    struct vxpdb_group *dest)
{
	struct group *g;

	errno = 0;
	if ((g = getgrnam(name)) == NULL)
		return -errno;

	nssgroup_copy(dest, g);
	return 1;
}

static void *vnss1_grouptrav_init(struct vxpdb_state *this)
{
	setgrent();
	return this;
}

static int vnss1_grouptrav_walk(struct vxpdb_state *this, void *priv_data,
    struct vxpdb_group *dest)
{
	struct group *gr;

	errno = 0;
	if ((gr = getgrent()) == NULL)
		return -errno;

	nssgroup_copy(dest, gr);
	return 1;
}

static void vnss1_grouptrav_free(struct vxpdb_state *this, void *priv_data)
{
	endgrent();
	return;
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

static void nssuser_copy(struct vxpdb_user *dest, const struct passwd *pe,
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
	return;
}

static void nssgroup_copy(struct vxpdb_group *dest, const struct group *src)
{
	HX_strclone(&dest->gr_name, src->gr_name);
	dest->gr_gid  = src->gr_gid;
	dest->be_priv = NULL;
	return;
}

struct vxpdb_driver THIS_MODULE = {
	.name           = "NSS back-end module (not MU/MT-safe)",
	.desc           = "API demonstration",
	.modctl         = vnss1_modctl,
	.getpwuid       = vnss1_getpwuid,
	.getpwnam       = vnss1_getpwnam,
	.usertrav_init  = vnss1_usertrav_init,
	.usertrav_walk  = vnss1_usertrav_walk,
	.usertrav_free  = vnss1_usertrav_free,
	.getgrgid       = vnss1_getgrgid,
	.getgrnam       = vnss1_getgrnam,
	.grouptrav_init = vnss1_grouptrav_init,
	.grouptrav_walk = vnss1_grouptrav_walk,
	.grouptrav_free = vnss1_grouptrav_free,
};
