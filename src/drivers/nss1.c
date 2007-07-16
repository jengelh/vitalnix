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
#include "drivers/proto.h"
#include "drivers/static-build.h"
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

// Functions
static long count_users(void);
static long count_groups(void);
static void nssuser_copy(struct vxpdb_user *, const struct passwd *, const struct spwd *);
static void nssgroup_copy(struct vxpdb_group *, const struct group *);
static inline int nssuser_match(const struct passwd *, const struct vxpdb_user *);
static inline int nssgroup_match(const struct group *, const struct vxpdb_group *);

//-----------------------------------------------------------------------------
static long vnss1_modctl(struct vxpdb_state *this, long command, ...) {
    errno = 0;
    switch(command) {
        case PDB_COUNT_USERS:
            return count_users();
        case PDB_COUNT_GROUPS:
            return count_groups();
    }
    return -ENOSYS;
}

//-----------------------------------------------------------------------------
static int vnss1_userinfo(struct vxpdb_state *this,
  const struct vxpdb_user *sr_mask, struct vxpdb_user *dest, size_t size)
{
    struct vxpdb_user temp_mask;
    struct passwd *pe;
    int found = 0;

    if(sr_mask == dest) {
        memcpy(&temp_mask, sr_mask, sizeof(struct vxpdb_user));
        sr_mask = &temp_mask;
    }

    setpwent();
    while((pe = getpwent()) != NULL && (dest == NULL || size > 0)) {
        if(!nssuser_match(pe, sr_mask))
            continue;

        if(dest != NULL) {
            nssuser_copy(dest, pe, getspnam(pe->pw_name));
            ++dest;
            ++found;
            --size;
        } else {
            if(size == 0) {
                endpwent();
                return 1;
            }
            ++found;
        }
    }

    return found;
}

static void *vnss1_usertrav_init(struct vxpdb_state *this) {
    setpwent();
    return this;
}

static int vnss1_usertrav_walk(struct vxpdb_state *this, void *priv_data,
  struct vxpdb_user *dest)
{
    struct passwd *pe;
    struct spwd *se;

    errno = 0;
    if((pe = getpwent()) == NULL)
        return -errno; // if errno=0, we return 0, which is fine

    vxpdb_user_clean(dest);
    if(*pe->pw_name == '+' || *pe->pw_name == '-')
        ++pe->pw_name;

    se = getspnam(pe->pw_name);
    nssuser_copy(dest, pe, se);
    return 1;
}

static void vnss1_usertrav_free(struct vxpdb_state *this, void *priv_data) {
    endpwent();
    return;
}

//-----------------------------------------------------------------------------
static int vnss1_groupinfo(struct vxpdb_state *this,
  const struct vxpdb_group *sr_mask, struct vxpdb_group *dest, size_t size)
{
    struct vxpdb_group temp_mask;
    struct group *ge;
    int found = 0;

    if(sr_mask == dest) {
        memcpy(&temp_mask, sr_mask, sizeof(struct vxpdb_group));
        sr_mask = &temp_mask;
    }

    setgrent();
    while((ge = getgrent()) != NULL && (dest != NULL || size > 0)) {
        if(!nssgroup_match(ge, sr_mask))
            continue;

        if(dest != NULL) {
            nssgroup_copy(dest, ge);
            ++dest;
            ++found;
            --size;
        } else {
            if(size == 0) {
                endgrent();
                return 1;
            }
            ++found;
        }
    }

    endgrent();
    return found;
}

static void *vnss1_grouptrav_init(struct vxpdb_state *this) {
    setgrent();
    return this;
}

static int vnss1_grouptrav_walk(struct vxpdb_state *this, void *priv_data,
  struct vxpdb_group *dest)
{
    struct group *gr;

    errno = 0;
    if((gr = getgrent()) == NULL)
        return -errno;

    nssgroup_copy(dest, gr);
    return 1;
}

static void vnss1_grouptrav_free(struct vxpdb_state *this, void *priv_data) {
    endgrent();
    return;
}

//-----------------------------------------------------------------------------
static long count_users(void) {
    long n = 0;
    setpwent();
    while(getpwent() != NULL)
        ++n;
    endpwent();
    return n;
}

static long count_groups(void) {
    long n = 0;
    setgrent();
    while(getgrent() != NULL)
        ++n;
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

    if(se != NULL) {
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

static void nssgroup_copy(struct vxpdb_group *dest, const struct group *src) {
    HX_strclone(&dest->gr_name, src->gr_name);
    dest->gr_gid  = src->gr_gid;
    dest->be_priv = NULL;
    return;
}

static inline int nssuser_match(const struct passwd *user,
  const struct vxpdb_user *mask)
{
    return
      (mask->pw_name == NULL || strcmp(user->pw_name, mask->pw_name) == 0) &&
      (mask->pw_uid == PDB_NOUID || user->pw_uid == mask->pw_uid) &&
      (mask->pw_gid == PDB_NOGID || user->pw_gid == mask->pw_gid) &&
      (mask->pw_real == NULL || strcmp(user->pw_gecos, mask->pw_real) == 0) &&
      (mask->pw_home == NULL || strcmp(user->pw_dir, mask->pw_home) == 0) &&
      (mask->pw_shell == NULL || strcmp(user->pw_shell, mask->pw_shell) == 0);
}

static inline int nssgroup_match(const struct group *group,
 const struct vxpdb_group *mask)
{
    return
      (mask->gr_name == NULL || strcmp(group->gr_name, mask->gr_name) == 0) &&
      (mask->gr_gid == PDB_NOGID || group->gr_gid == mask->gr_gid);
}

//-----------------------------------------------------------------------------
static struct vxpdb_driver THIS_MODULE = {
    .name           = "NSS back-end module (not MU/MT-safe)",
    .desc           = "API demonstration",

    .modctl         = vnss1_modctl,

    .userinfo       = vnss1_userinfo,
    .usertrav_init  = vnss1_usertrav_init,
    .usertrav_walk  = vnss1_usertrav_walk,
    .usertrav_free  = vnss1_usertrav_free,

    .groupinfo      = vnss1_groupinfo,
    .grouptrav_init = vnss1_grouptrav_init,
    .grouptrav_walk = vnss1_grouptrav_walk,
    .grouptrav_free = vnss1_grouptrav_free,
};

REGISTER_MODULE(nss1, &THIS_MODULE);

//=============================================================================
