/*
 *	libvxdb/dummy.c - VTABLE crypt
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
#include <vitalnix/libvxdb/libvxdb.h>

#define ALIAS(orig, new) static typeof(orig) new __attribute__((alias(#orig)))

static int vxdummy_init(struct vxdb_state *vp, const char *config_file)
{
	return 1;
}

static int vxdummy_open(struct vxdb_state *vp, unsigned int flags)
{
	return 1;
}

static void vxdummy_close(struct vxdb_state *vp)
{
}

static void vxdummy_exit(struct vxdb_state *vp)
{
}

static int vxdummy_lock(struct vxdb_state *vp)
{
	return 1;
}

static int vxdummy_unlock(struct vxdb_state *vp)
{
	return 1;
}

static long vxdummy_modctl(struct vxdb_state *vp, unsigned int command, ...)
{
	switch (command) {
		case VXDB_COUNT_USERS:
		case VXDB_COUNT_GROUPS:
			return 0;
	}
	return -ENOSYS;
}

static int vxdummy_getpwuid(struct vxdb_state *vp, unsigned int uid,
    struct vxdb_user *dest)
{
	return 0;
}

static int vxdummy_getpwnam(struct vxdb_state *vp, const char *name,
    struct vxdb_user *dest)
{
	return 0;
}

static int vxdummy_useradd(struct vxdb_state *vp, const struct vxdb_user *user)
{
	return -EPERM;
}

static int vxdummy_usermod(struct vxdb_state *vp, const char *name,
    const struct vxdb_user *newstuff)
{
	return -EPERM;
}

static int vxdummy_userdel(struct vxdb_state *vp, const char *name)
{
	return -EPERM;
}

static void *vxdummy_usertrav_init(struct vxdb_state *vp)
{
	return vp;
}

static int vxdummy_usertrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_user *result)
{
	return 0;
}

static void vxdummy_usertrav_free(struct vxdb_state *vp, void *ptr)
{
}

static int vxdummy_getgrgid(struct vxdb_state *vp, unsigned int gid,
    struct vxdb_group *dest)
{
	return 0;
}

static int vxdummy_getgrnam(struct vxdb_state *vp, const char *name,
    struct vxdb_group *dest)
{
	return 0;
}

static int vxdummy_groupadd(struct vxdb_state *vp, const struct vxdb_group *u)
{
	return -EPERM;
}

static int vxdummy_groupmod(struct vxdb_state *vp, const char *name,
    const struct vxdb_group *newstuff)
{
	return -EPERM;
}

static int vxdummy_groupdel(struct vxdb_state *vp, const char *name)
{
	return -EPERM;
}

static void *vxdummy_grouptrav_init(struct vxdb_state *vp)
{
	return vp;
}

static int vxdummy_grouptrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_group *result)
{
	return 0;
}

static void vxdummy_grouptrav_free(struct vxdb_state *vp, void *ptr)
{
}

static int vxdummy_sgmapadd(struct vxdb_state *vp, const char *user,
    const char *group)
{
	return -EPERM;
}

static int vxdummy_sgmapget(struct vxdb_state *vp, const char *user,
    char ***result)
{
	return 0;
}

static int vxdummy_sgmapdel(struct vxdb_state *vp, const char *user,
    const char *group)
{
	return 0;
}

/*
 * vxdb_fix_vtable -
 * @m:	vtable
 */
EXPORT_SYMBOL void vxdb_fix_vtable(struct vxdb_driver *m)
{
#define SET(x) if ((m->x) == NULL) (m->x) = vxdummy_##x;
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
		m->usertrav_init = vxdummy_usertrav_init;
		m->usertrav_walk = vxdummy_usertrav_walk;
		m->usertrav_free = vxdummy_usertrav_free;
	}

	SET(getgrgid);
	SET(getgrnam);
	SET(groupadd);
	SET(groupmod);
	SET(groupdel);
	if (m->grouptrav_init == NULL && m->grouptrav_walk == NULL &&
	    m->grouptrav_free == NULL) {
		m->grouptrav_init = vxdummy_grouptrav_init;
		m->grouptrav_walk = vxdummy_grouptrav_walk;
		m->grouptrav_free = vxdummy_grouptrav_free;
	}
	SET(sgmapadd);
	SET(sgmapget);
	SET(sgmapdel);
#undef SET
}
