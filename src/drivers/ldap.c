/*=============================================================================
Vitalnix User Management Suite
drivers/ldap.c - LDAP back-end module
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2006

  This file is part of Vitalnix 2006 ENTERPRISE EDITION.
  You may not redistribute this file in any form without obtaining
  written permission (includes email) from Alphagate Systems Indp.

  This PDB back-end module comes WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  By using it, you agree to this.
=============================================================================*/
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>
#include "drivers/proto.h"
#include "drivers/static-build.h"
#include "libvxpdb/libvxpdb.h"

struct ldap_state {
    // Connection
    LDAP *cn_handle;
    char *cn_host, *cn_socket, *cn_user, *cn_passwd, *cn_database,
         *cn_user2, *cn_passwd2;
    int cn_port;

    // Misc
    long uid_min, uid_max, gid_min, gid_max;
};

// Functions
DRIVER_PROTO_BASE1(vldap);
DRIVER_PROTO_USER(vldap);
DRIVER_PROTO_GROUP(vldap);

// Variables
static struct vxpdb_mvtable THIS_MODULE = {
    .name           = "LDAP back-end module",
    .author         = "Alphagate Systems <support [at] ahn hopto org>, 2005 - 2006",
    DRIVER_CB_BASE1(vldap),
    DRIVER_CB_USER(vldap),
    DRIVER_CB_GROUP(vldap),
};

//-----------------------------------------------------------------------------
REGISTER_MODULE(ldap, &THIS_MODULE);

static int vldap_init(struct vxpdb_state *vp, const char *config_file) {
    struct ldap_state *state;

    if((state = vp->state = calloc(1, sizeof(struct ldap_state))) == NULL)
        return -errno;

    return 1;
}

static int vldap_open(struct vxpdb_state *vp, long flags) {
    struct ldap_state *state = vp->state;
    int ret;

    ret = ldap_initialize(&state->cn_handle, NULL);
    if(ret != LDAP_SUCCESS)
        return ret;

    //ldap_set_option(state->cn_handle, LDAP_OPT_DEREF, LDAP_DEREF_NEVER);
    //ldap_set_option(state->cn_handle, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
//    ldap_bind_s(ld, pv->binddn, pv->bindpw, LDAP_AUTH_SIMPLE);

    return 1;
}

static void vldap_close(struct vxpdb_state *vp) {
    return;
}

static void vldap_deinit(struct vxpdb_state *vp) {       
    struct ldap_state *state = vp->state;
    free(state);
    return;
}

//-----------------------------------------------------------------------------
static int vldap_useradd(struct vxpdb_state *vp, const struct vxpdb_user *rq) {
    return 0;
}

static int vldap_usermod(struct vxpdb_state *vp,
 const struct vxpdb_user *sr_mask, const struct vxpdb_user *mod_mask)
{
    return 0;
}

static int vldap_userdel(struct vxpdb_state *vp,
 const struct vxpdb_user *sr_mask)
{
    return 0;
}

static int vldap_userinfo(struct vxpdb_state *vp,
 const struct vxpdb_user *sr_mask, struct vxpdb_user *dest, size_t size)
{
    return 0;
}

static void *vldap_usertrav_init(struct vxpdb_state *vp) {
    return vp;
}

static int vldap_usertrav_walk(struct vxpdb_state *vp, void *ptr,
 struct vxpdb_user *dest)
{
    return 0;
}

static void vldap_usertrav_free(struct vxpdb_state *vp, void *ptr) {
    return;
}

//-----------------------------------------------------------------------------
static int vldap_groupadd(struct vxpdb_state *vp,
 const struct vxpdb_group *rq)
{
    return 0;
}

static int vldap_groupmod(struct vxpdb_state *vp,
 const struct vxpdb_group *sr_mask, const struct vxpdb_group *mod_mask)
{
    return 0;
}

static int vldap_groupdel(struct vxpdb_state *vp,
 const struct vxpdb_group *sr_mask)
{
    return 0;
}

static int vldap_groupinfo(struct vxpdb_state *vp,
 const struct vxpdb_group *sr_mask, struct vxpdb_group *dest, size_t size)
{
    return 0;
}

static void *vldap_grouptrav_init(struct vxpdb_state *vp) {
    return vp;
}

static int vldap_grouptrav_walk(struct vxpdb_state *vp, void *ptr,
 struct vxpdb_group *dest)
{
    return 0;
}

static void vldap_grouptrav_free(struct vxpdb_state *vp, void *ptr) {
    return;
}

//=============================================================================