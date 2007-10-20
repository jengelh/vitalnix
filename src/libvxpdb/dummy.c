/*
 *	libvxpdb/dummy.c - VTABLE crypt
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdio.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxpdb/libvxpdb.h>

#define ALIAS(orig, new) static typeof(orig) new __attribute__((alias(#orig)))

static int vdummy_init(struct vxpdb_state *vp, const char *config_file)
{
	return 1;
}

static int vdummy_open(struct vxpdb_state *vp, unsigned int flags)
{
	return 1;
}

static void vdummy_vT(struct vxpdb_state *vp)
{
	return;
}
ALIAS(vdummy_vT, vdummy_close);
ALIAS(vdummy_vT, vdummy_exit);

static int vdummy_iT(struct vxpdb_state *vp)
{
	return 1;
}
ALIAS(vdummy_iT, vdummy_lock);
ALIAS(vdummy_iT, vdummy_unlock);

static long vdummy_modctl(struct vxpdb_state *vp, unsigned int command, ...)
{
	switch (command) {
		case PDB_COUNT_USERS:
		case PDB_COUNT_GROUPS:
			return 0;
	}
	return -ENOSYS;
}

static int vdummy_getpwuid(struct vxpdb_state *vp, unsigned int uid,
    struct vxpdb_user *dest)
{
	return 0;
}

static int vdummy_getpwnam(struct vxpdb_state *vp, const char *name,
    struct vxpdb_user *dest)
{
	return 0;
}

static int vdummy_useradd(struct vxpdb_state *vp, const struct vxpdb_user *user)
{
	return -EPERM;
}

static int vdummy_usermod(struct vxpdb_state *vp, const char *name,
    const struct vxpdb_user *newstuff)
{
	return -EPERM;
}

static int vdummy_userdel(struct vxpdb_state *vp, const char *name)
{
	return -EPERM;
}

static void *vdummy_usertrav_init(struct vxpdb_state *vp)
{
	return vp;
}

static int vdummy_usertrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_user *result)
{
	return 0;
}

static void vdummy_usertrav_free(struct vxpdb_state *vp, void *ptr)
{
	return;
}

static int vdummy_getgrgid(struct vxpdb_state *vp, unsigned int gid,
    struct vxpdb_group *dest)
{
	return 0;
}

static int vdummy_getgrnam(struct vxpdb_state *vp, const char *name,
    struct vxpdb_group *dest)
{
	return 0;
}

static int vdummy_groupadd(struct vxpdb_state *vp, const struct vxpdb_group *u)
{
	return -EPERM;
}

static int vdummy_groupmod(struct vxpdb_state *vp, const char *name,
    const struct vxpdb_group *newstuff)
{
	return -EPERM;
}

static int vdummy_groupdel(struct vxpdb_state *vp, const char *name)
{
	return -EPERM;
}

static void *vdummy_grouptrav_init(struct vxpdb_state *vp)
{
	return vp;
}

static int vdummy_grouptrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_group *result)
{
	return 0;
}

static void vdummy_grouptrav_free(struct vxpdb_state *vp, void *ptr)
{
	return;
}

/*
 * vxpdb_fix_vtable -
 * @m:	vtable
 */
EXPORT_SYMBOL void vxpdb_fix_vtable(struct vxpdb_driver *m)
{
#define SET(x) if ((m->x) == NULL) (m->x) = vdummy_##x;
	SET(init);
	SET(open);
	SET(close);
	SET(modctl);
	SET(exit);
	SET(lock);
	SET(unlock);

	SET(getpwuid);
	SET(getpwnam);
	SET(useradd);
	SET(usermod);
	SET(userdel);
	if (m->usertrav_init == NULL && m->usertrav_walk == NULL &&
	    m->usertrav_free == NULL) {
			m->usertrav_init = vdummy_usertrav_init;
			m->usertrav_walk = vdummy_usertrav_walk;
			m->usertrav_free = vdummy_usertrav_free;
	}

	SET(getgrgid);
	SET(getgrnam);
	SET(groupadd);
	SET(groupmod);
	SET(groupdel);
	if (m->grouptrav_init == NULL && m->grouptrav_walk == NULL &&
	    m->grouptrav_free == NULL) {
			m->grouptrav_init = vdummy_grouptrav_init;
			m->grouptrav_walk = vdummy_grouptrav_walk;
			m->grouptrav_free = vdummy_grouptrav_free;
	}
	return;
#undef SET
}
