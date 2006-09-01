/*=============================================================================
Vitalnix User Management Suite
libvxpdb/dummy.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2006
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
#include <errno.h>
#include <stdio.h>
#include "compiler.h"
#include "drivers/proto.h"
#include "libvxpdb/libvxpdb.h"

#define ALIAS(orig, new) static typeof(orig) new __attribute__((alias(#orig)))

//-----------------------------------------------------------------------------
static int vdummy_init(struct vxpdb_state *vp, const char *config_file) {
    return 1;
}

static int vdummy_open(struct vxpdb_state *vp, long flags) {
    return 1;
}

static void vdummy_vT(struct vxpdb_state *vp) {
    return;
}
ALIAS(vdummy_vT, vdummy_close);
ALIAS(vdummy_vT, vdummy_deinit);

static int vdummy_iT(struct vxpdb_state *vp) {
    return 1;
}
ALIAS(vdummy_iT, vdummy_lock);
ALIAS(vdummy_iT, vdummy_unlock);

static long vdummy_modctl(struct vxpdb_state *vp, long command, ...) {
    switch(command) {
        case PDB_COUNT_USERS:
        case PDB_COUNT_GROUPS:
            return 0;
    }
    return -ENOSYS;
}

static int vdummy_useradd(struct vxpdb_state *vp,
  const struct vxpdb_user *user)
{
    return -EPERM;
}

static int vdummy_usermod(struct vxpdb_state *vp,
  const struct vxpdb_user *mask, const struct vxpdb_user *mod)
{
    return -EPERM;
}

static int vdummy_userdel(struct vxpdb_state *vp,
  const struct vxpdb_user *hint)
{
    return -EPERM;
}

static void *vdummy_usertrav_init(struct vxpdb_state *vp) {
    return vp;
}

static int vdummy_usertrav_walk(struct vxpdb_state *vp, void *ptr,
  struct vxpdb_user *result)
{
    return 0;
}

static void vdummy_usertrav_free(struct vxpdb_state *vp, void *ptr) {
    return;
}

static int vdummy_userinfo(struct vxpdb_state *vp,
  const struct vxpdb_user *mask, struct vxpdb_user *result, size_t size)
{
    return 0;
}

static int vdummy_groupadd(struct vxpdb_state *vp,
  const struct vxpdb_group *u) {
    return -EPERM;
}

static int vdummy_groupmod(struct vxpdb_state *vp,
  const struct vxpdb_group *vprk, const struct vxpdb_group *mod)
{
    return -EPERM;
}

static int vdummy_groupdel(struct vxpdb_state *vp,
  const struct vxpdb_group *hint)
{
    return -EPERM;
}

static void *vdummy_grouptrav_init(struct vxpdb_state *vp) {
    return vp;
}

static int vdummy_grouptrav_walk(struct vxpdb_state *vp, void *ptr,
  struct vxpdb_group *result)
{
    return 0;
}

static void vdummy_grouptrav_free(struct vxpdb_state *vp, void *ptr) {
    return;
}

static int vdummy_groupinfo(struct vxpdb_state *vp,
  const struct vxpdb_group *mask, struct vxpdb_group *result, size_t size)
{
    return 0;
}

//-----------------------------------------------------------------------------
/*  vxpdb_fix_vtable
    @m: vtable

*/
EXPORT_SYMBOL void vxpdb_fix_vtable(struct vxpdb_mvtable *m)
{
#define SET(x) if((m->x) == NULL) (m->x) = vdummy_##x;
    SET(init);
    SET(open);
    SET(close);
    SET(modctl);
    SET(deinit);
    SET(lock);
    SET(unlock);

    SET(useradd);
    SET(usermod);
    SET(userdel);
    if(m->usertrav_init == NULL && m->usertrav_walk == NULL &&
     m->usertrav_free == NULL) {
        m->usertrav_init = vdummy_usertrav_init;
        m->usertrav_walk = vdummy_usertrav_walk;
        m->usertrav_free = vdummy_usertrav_free;
    }
    SET(userinfo);

    SET(groupadd);
    SET(groupmod);
    SET(groupdel);
    if(m->grouptrav_init == NULL && m->grouptrav_walk == NULL &&
     m->grouptrav_free == NULL) {
        m->grouptrav_init = vdummy_grouptrav_init;
        m->grouptrav_walk = vdummy_grouptrav_walk;
        m->grouptrav_free = vdummy_grouptrav_free;
    }
    SET(groupinfo);
    return;
#undef SET
}

//=============================================================================
