/*
 *	ldap.c - LDAP back-end
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>
#include "drivers/proto.h"
#include "drivers/static-build.h"
#include <vitalnix/libvxpdb/libvxpdb.h>

struct ldap_state {
	/* Connection */
	LDAP *conn;
	char *cn_host, *cn_socket, *cn_user, *cn_passwd, *cn_database,
		 *cn_user2, *cn_passwd2;
	int cn_port;

	/* Misc */
	long uid_min, uid_max, gid_min, gid_max;
};

struct ldaptrav {
	LDAPMessage *base, *current;
};

//-----------------------------------------------------------------------------
static int vldap_init(struct vxpdb_state *vp, const char *config_file)
{
	struct ldap_state *state;

	if ((state = vp->state = calloc(1, sizeof(struct ldap_state))) == NULL)
		return -errno;

	return 1;
}

static int vldap_open(struct vxpdb_state *vp, long flags)
{
	struct ldap_state *state = vp->state;
	int ret;

	ret = ldap_initialize(&state->conn, "ldap://127.0.0.1/");
	if (ret != LDAP_SUCCESS)
		return -ret;

	ret = LDAP_VERSION3;
	ldap_set_option(state->conn, LDAP_OPT_PROTOCOL_VERSION, &ret);

	/*
	
	ldap_bind_s(ld, pv->binddn, pv->bindpw, LDAP_AUTH_SIMPLE);
	*/

	return 1;
}

static void vldap_close(struct vxpdb_state *vp)
{
	struct ldap_state *state = vp->state;
	ldap_unbind_ext(state->conn, NULL, NULL);
	return;
}

static void vldap_exit(struct vxpdb_state *vp)
{
	struct ldap_state *state = vp->state;
	free(state);
	return;
}

static int vldap_useradd(struct vxpdb_state *vp, const struct vxpdb_user *rq)
{
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

static void *vldap_usertrav_init(struct vxpdb_state *vp)
{
	struct ldap_state *state = vp->state;
	struct ldaptrav trav;
	int ret;

	ret = ldap_search_ext_s(state->conn, NULL, LDAP_SCOPE_SUBTREE,
	      "(objectClass=posixAccount)", NULL, 0, NULL, NULL,
	      NULL, 0, &trav.base);
	if (ret != LDAP_SUCCESS) {
		errno = 1600 + ret;
		return NULL;
	}

	trav.current = ldap_first_entry(state->conn, trav.base);
	return HX_memdup(&trav, sizeof(trav));
}

static int vldap_usertrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_user *dest)
{
	struct ldap_state *state  = vp->state;
	struct ldaptrav *trav = ptr;
	BerElement *ber;
	char *attr;

	if (trav->current == NULL)
		return 0;

	hmc_strasg(&dest->pw_name, NULL);
	dest->pw_uid     = PDB_NOUID;
	dest->pw_gid     = PDB_NOGID;
	dest->pw_igrp    = NULL;
	hmc_strasg(&dest->pw_real, NULL);
	hmc_strasg(&dest->pw_home, NULL);
	hmc_strasg(&dest->pw_shell, NULL);
	hmc_strasg(&dest->sp_passwd, NULL);
	dest->sp_lastchg = 0;
	dest->sp_min     = PDB_DFL_KEEPMIN;
	dest->sp_max     = PDB_DFL_KEEPMAX;
	dest->sp_warn    = PDB_DFL_WARNAGE;
	dest->sp_expire  = PDB_NO_EXPIRE;
	dest->sp_inact   = PDB_NO_INACTIVE;

	for (attr = ldap_first_attribute(state->conn, trav->current, &ber);
	    attr != NULL;
	    attr = ldap_next_attribute(state->conn, trav->current, ber))
	{
		char **val;
		
		val = ldap_get_values(state->conn, trav->current, attr);
		if (val == NULL)
			continue;
		if (*val == NULL) {
			ldap_value_free(val);
			continue;
		}
		if (strcmp(attr, "uid") == 0)
			hmc_strasg(&dest->pw_name, *val);
		else if (strcmp(attr, "uidNumber") == 0)
			dest->pw_uid = strtol(*val, NULL, 0);
		else if (strcmp(attr, "gidNumber") == 0)
			dest->pw_gid = strtol(*val, NULL, 0);
		else if (strcmp(attr, "gecos") == 0)
			hmc_strasg(&dest->pw_real, *val);
		else if (strcmp(attr, "homeDirectory") == 0)
			hmc_strasg(&dest->pw_home, *val);
		else if (strcmp(attr, "loginShell") == 0)
			hmc_strasg(&dest->pw_shell, *val);
		else if (strcmp(attr, "userPassword") == 0)
			hmc_strasg(&dest->sp_passwd, *val);
		else if (strcmp(attr, "shadowLastChange") == 0)
			dest->sp_lastchg = strtol(*val, NULL, 0);
		else if (strcmp(attr, "shadowMin") == 0)
			dest->sp_min = strtol(*val, NULL, 0);
		else if (strcmp(attr, "shadowMax") == 0)
			dest->sp_max = strtol(*val, NULL, 0);
		else if (strcmp(attr, "shadowWarning") == 0)
			dest->sp_warn = strtol(*val, NULL, 0);
		else if (strcmp(attr, "shadowExpire") == 0)
			dest->sp_expire = strtol(*val, NULL, 0);
		else if (strcmp(attr, "shadowInactive") == 0)
			dest->sp_inact = strtol(*val, NULL, 0);
		else if (strcmp(attr, "vitalnixDeferTimer") == 0)
			dest->vs_defer = strtoul(*val, NULL, 0);
		else if (strcmp(attr, "vitalnixUUID") == 0)
			hmc_strasg(&dest->vs_uuid, *val);
		else if (strcmp(attr, "vitalnixPvgrp") == 0)
			hmc_strasg(&dest->vs_pvgrp, *val);
		ldap_value_free(val);
		ldap_memfree(attr);
	}

	trav->current = ldap_next_entry(state->conn, trav->current);
	return 1;
}

static void vldap_usertrav_free(struct vxpdb_state *vp, void *ptr)
{
	struct ldaptrav *trav = ptr;
	ldap_msgfree(trav->base);
	free(trav);
	return;
}

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

static void *vldap_grouptrav_init(struct vxpdb_state *vp)
{
	struct ldap_state *state = vp->state;
	struct ldaptrav trav;
	int ret;

	ret = ldap_search_ext_s(state->conn, NULL, LDAP_SCOPE_SUBTREE,
	      "(objectClass=posixGroup)", NULL, 0, NULL, NULL,
	      NULL, 0, &trav.base);
	if (ret != LDAP_SUCCESS) {
		errno = 1600 + ret;
		return NULL;
	}

	trav.current = ldap_first_entry(state->conn, trav.base);
	return HX_memdup(&trav, sizeof(trav));
}

static int vldap_grouptrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_group *dest)
{
	struct ldap_state *state = vp->state;
	struct ldaptrav *trav    = ptr;
	BerElement *ber;
	char *attr;

	if (trav->current == NULL)
		return 0;

	hmc_strasg(&dest->gr_name, NULL);
	dest->gr_gid = PDB_NOGID;

	for (attr = ldap_first_attribute(state->conn, trav->current, &ber);
	    attr != NULL;
	    attr = ldap_next_attribute(state->conn, trav->current, ber))
	{
		char **val;

		val = ldap_get_values(state->conn, trav->current, attr);
		if (val == NULL)
			continue;
		if (*val == NULL) {
			ldap_value_free(val);
			continue;
		}
		if (strcmp(attr, "gidNumber") == 0)
			dest->gr_gid = strtol(*val, NULL, 0);
		else if (strcmp(attr, "cn") == 0)
			hmc_strasg(&dest->gr_name, *val);
		ldap_value_free(val);
		ldap_memfree(attr);
	}

	trav->current = ldap_next_entry(state->conn, trav->current);
	return 1;
}

static void vldap_grouptrav_free(struct vxpdb_state *vp, void *ptr)
{
	struct ldaptrav *trav = ptr;
	ldap_msgfree(trav->base);
	free(trav);
	return;
}

static struct vxpdb_driver THIS_MODULE = {
	.name = "LDAP back-end module",
	DRIVER_CB_BASE1(vldap),
	DRIVER_CB_USER(vldap),
	DRIVER_CB_GROUP(vldap),
};

REGISTER_MODULE(ldap, &THIS_MODULE);
