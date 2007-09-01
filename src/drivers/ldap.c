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
#define Z_32 sizeof("4294967296")

struct ldap_state {
	/* Connection */
	LDAP *conn;
	char *cn_host, *cn_socket, *cn_user, *cn_passwd, *cn_database,
		 *cn_user2, *cn_passwd2;
	int cn_port;

	char *user_suffix, *group_suffix;

	/* Misc */
	long uid_min, uid_max, gid_min, gid_max;
};

struct ldap_trav {
	LDAPMessage *base, *current;
};

//-----------------------------------------------------------------------------
static int vxldap_init(struct vxpdb_state *vp, const char *config_file)
{
	struct ldap_state *state;

	fprintf(stderr, "Vitalnix LDAP support still incomplete.\n");

	if ((state = vp->state = calloc(1, sizeof(struct ldap_state))) == NULL)
		return -errno;

	state->uid_min      = 1000;
	state->uid_max      = 60000;
	state->user_suffix  = "ou=users,dc=site";
	state->group_suffix = "ou=groups,dc=site";

	return 1;
}

static int vxldap_open(struct vxpdb_state *vp, long flags)
{
	struct ldap_state *state = vp->state;
	int ret;

	ret = ldap_initialize(&state->conn, "ldap://127.0.0.1/");
	if (ret != LDAP_SUCCESS)
		return -ret;

	ret = LDAP_VERSION3;
	ldap_set_option(state->conn, LDAP_OPT_PROTOCOL_VERSION, &ret);

	if (flags & PDB_WRLOCK) {
		ret = ldap_simple_bind_s(state->conn, "cn=root,dc=site", "secret");
		if (ret != LDAP_SUCCESS)
			fprintf(stderr, "Simple bind failed; will use anon\n");
	}

	return 1;
}

static void vxldap_close(struct vxpdb_state *vp)
{
	struct ldap_state *state = vp->state;
	ldap_unbind_ext(state->conn, NULL, NULL);
	return;
}

static void vxldap_exit(struct vxpdb_state *vp)
{
	struct ldap_state *state = vp->state;
	free(state);
	return;
}

static hmc_t *dn_user(const struct ldap_state *state,
    const struct vxpdb_user *rq)
{
	hmc_t *ret;
	if (rq->pw_name == NULL)
		return NULL;
	ret = hmc_sinit("uid=");
	hmc_strcat(&ret, rq->pw_name);
	hmc_strcat(&ret, ",");
	hmc_strcat(&ret, state->user_suffix);
	return ret;
}

static int vxldap_useradd(struct vxpdb_state *vp, const struct vxpdb_user *rq)
{
	struct ldap_state *state = vp->state;

	char s_pw_uid[Z_32], s_pw_gid[Z_32], s_sp_min[Z_32], s_sp_max[Z_32];
	char s_sp_warn[Z_32], s_sp_expire[Z_32], s_sp_inact[Z_32];
	char s_vs_defer[Z_32];
	LDAPMod attr[17], *attr_ptrs[18];
	unsigned int a = 0, i;
	hmc_t *dn;
	int ret;

	if ((dn = dn_user(state, rq)) == NULL)
		return -EINVAL;

	if (rq->sp_min != PDB_DFL_KEEPMIN || rq->sp_max != PDB_DFL_KEEPMAX ||
	    rq->sp_warn != PDB_DFL_WARNAGE || rq->sp_expire != PDB_NO_EXPIRE ||
	    rq->sp_inact != PDB_NO_INACTIVE)
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "objectClass",
			.mod_values = (char *[]){"account", "posixAccount",
			              "shadowAccount", NULL},
		};
	else
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "objectClass",
			.mod_values = (char *[]){"account", "posixAccount", NULL},
		};

	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "uid",
		.mod_values = (char *[]){rq->pw_name, NULL},
	};
	snprintf(s_pw_uid, sizeof(s_pw_uid), "%lu", rq->pw_uid);
	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "uidNumber",
		.mod_values = (char *[]){s_pw_uid, NULL},
	};
	snprintf(s_pw_gid, sizeof(s_pw_gid), "%lu", rq->pw_gid);
	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "gidNumber",
		.mod_values = (char *[]){s_pw_gid, NULL},
	};
	if (rq->pw_real != NULL) {
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "gecos",
			.mod_values = (char *[]){rq->pw_real, NULL},
		};
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "cn",
			.mod_values = (char *[]){rq->pw_real, NULL},
		};
	} else {
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "cn",
			.mod_values = (char *[]){rq->pw_name, NULL},
		};
	}
	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "homeDirectory",
		.mod_values = (char *[]){rq->pw_home, NULL},
	};
	if (rq->pw_shell != NULL)
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "loginShell",
			.mod_values = (char *[]){rq->pw_shell, NULL},
		};

	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "userPassword",
		.mod_values = (char *[]){(rq->sp_passwd == NULL) ? "" :
		                         rq->sp_passwd, NULL},
	};

	/* shadow */
	if (rq->sp_min != PDB_DFL_KEEPMIN) {
		snprintf(s_sp_min, sizeof(s_sp_min), "%lu", rq->sp_min);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowMin",
			.mod_values = (char *[]){s_sp_min, NULL},
		};
	}
	if (rq->sp_max != PDB_DFL_KEEPMAX) {
		snprintf(s_sp_max, sizeof(s_sp_max), "%lu", rq->sp_max);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowMax",
			.mod_values = (char *[]){s_sp_max, NULL},
		};
	}
	if (rq->sp_warn != PDB_DFL_WARNAGE) {
		snprintf(s_sp_warn, sizeof(s_sp_warn), "%lu", rq->sp_warn);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowWarning",
			.mod_values = (char *[]){s_sp_warn, NULL},
		};
	}
	if (rq->sp_expire != PDB_NO_EXPIRE) {
		snprintf(s_sp_expire, sizeof(s_sp_expire), "%lu", rq->sp_expire);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowExpire",
			.mod_values = (char *[]){s_sp_expire, NULL},
		};
	}
	if (rq->sp_inact != PDB_NO_INACTIVE) {
		snprintf(s_sp_inact, sizeof(s_sp_inact), "%lu", rq->sp_inact);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowInactive",
			.mod_values = (char *[]){s_sp_inact, NULL},
		};
	}

	/* vitalnix-specific */
	if (rq->vs_pvgrp != NULL) {
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "vitalnixGroup",
			.mod_values = (char *[]){rq->vs_pvgrp, NULL},
		};
	}
	if (rq->vs_uuid != NULL) {
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "vitalnixUUID",
			.mod_values = (char *[]){rq->vs_pvgrp, NULL},
		};
	}
	if (rq->vs_defer > 0) {
		snprintf(s_vs_defer, sizeof(s_vs_defer), "%lu", rq->vs_defer);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "vitalnixDeferTimer",
			.mod_values = (char *[]){s_vs_defer, NULL},
		};
	}

	fprintf(stderr, "dn: %s\n", dn);
	for (i = 0; i < a; ++i) {
		int j;
		attr_ptrs[i] = &attr[i];
		for (j = 0; attr[i].mod_values[j] != NULL; ++j)
			fprintf(stderr, "%s: %s\n", attr[i].mod_type, attr[i].mod_values[j]);
	}
	attr_ptrs[i] = NULL;

	ret = ldap_add_ext_s(state->conn, dn, attr_ptrs, NULL, NULL);

	hmc_free(dn);
	if (ret != LDAP_SUCCESS)
		return -(errno = 1600 + ret);

	return 1;
}

static int vxldap_usermod(struct vxpdb_state *vp,
    const struct vxpdb_user *sr_mask, const struct vxpdb_user *mod_mask)
{
	return -ENOENT;
}

static int vxldap_userdel(struct vxpdb_state *vp,
    const struct vxpdb_user *sr_mask)
{
	struct ldap_state *state = vp->state;
	hmc_t *dn;
	int ret;

	if ((dn = dn_user(state, sr_mask)) == NULL)
		return -EINVAL;

	ret = ldap_delete_ext_s(state->conn, dn, NULL, NULL);
	hmc_free(dn);
	if (ret != LDAP_SUCCESS)
		return -(errno = 1600 + ret);
	
	return 1;
}

static void vxldap_copy_user(struct vxpdb_user *dest, LDAP *conn,
    LDAPMessage *entry)
{
	BerElement *ber;
	char *attr;

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

	for (attr = ldap_first_attribute(conn, entry, &ber); attr != NULL;
	    attr = ldap_next_attribute(conn, entry, ber))
	{
		char **val;
		
		val = ldap_get_values(conn, entry, attr);
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
		else if (strcmp(attr, "vitalnixGroup") == 0)
			hmc_strasg(&dest->vs_pvgrp, *val);
		else if (strcmp(attr, "vitalnixDeferTimer") == 0)
			dest->vs_defer = strtoul(*val, NULL, 0);
		ldap_value_free(val);
		ldap_memfree(attr);
	}
	return;
}

static int vxldap_getpwx(struct ldap_state *state, const char *filter,
    struct vxpdb_user *dest)
{
	LDAPMessage *result;
	int ret;

	ret = ldap_search_ext_s(state->conn, state->user_suffix,
	      LDAP_SCOPE_SUBTREE, filter, NULL, false,
	      NULL, NULL, NULL, 1, &result);
	if (ret != LDAP_SUCCESS || result == NULL)
		return -(errno = 1600 + ret);

	vxldap_copy_user(dest, state->conn, result);
	ldap_msgfree(result);
	return 1;
}

static int vxldap_getpwuid(struct vxpdb_state *vp, long uid,
    struct vxpdb_user *dest)
{
	char filter[48+Z_32];
	snprintf(filter, sizeof(filter),
	         "(&(objectClass=posixAccount)(uidNumber=%lu))", uid);
	return vxldap_getpwx(vp->state, filter, dest);
}

static int vxldap_getpwnam(struct vxpdb_state *vp, const char *user,
    struct vxpdb_user *dest)
{
	hmc_t *filter;
	int ret;

	filter = hmc_sinit("(&(objectClass=posixAccount)(uid=" /* )) */);
	hmc_strcat(&filter, user);
	hmc_strcat(&filter, /* (( */ "))");
	ret = vxldap_getpwx(vp->state, filter, dest);
	hmc_free(filter);
	return ret;
}

static void *vxldap_usertrav_init(struct vxpdb_state *vp)
{
	struct ldap_state *state = vp->state;
	struct ldap_trav trav;
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

static int vxldap_usertrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_user *dest)
{
	struct ldap_state *state = vp->state;
	struct ldap_trav *trav   = ptr;

	if (trav->current == NULL)
		return 0;

	vxldap_copy_user(dest, state->conn, trav->current);
	trav->current = ldap_next_entry(state->conn, trav->current);
	return 1;
}

static void vxldap_usertrav_free(struct vxpdb_state *vp, void *ptr)
{
	struct ldap_trav *trav = ptr;
	ldap_msgfree(trav->base);
	free(trav);
	return;
}

static hmc_t *dn_group(const struct ldap_state *state,
    const struct vxpdb_group *group)
{
	hmc_t *ret = hmc_sinit("cn=");
	if (group->gr_name == NULL)
		return ret;
	hmc_strcat(&ret, group->gr_name);
	hmc_strcat(&ret, ",");
	hmc_strcat(&ret, state->group_suffix);
	return ret;
}

static int vxldap_groupadd(struct vxpdb_state *vp,
    const struct vxpdb_group *rq)
{
	struct ldap_state *state = vp->state;
	LDAPMod attr[14], *attr_ptrs[15];
	hmc_t *dn;
	int ret;

	if ((dn = dn_group(state, rq)) == NULL)
		return -ENOMEM;

	ret = ldap_add_ext_s(state->conn, dn, attr_ptrs, NULL, NULL);
	if (ret != LDAP_SUCCESS)
		return -(errno = 1600 + ret);

	return 0;
}

static int vxldap_groupmod(struct vxpdb_state *vp,
    const struct vxpdb_group *sr_mask, const struct vxpdb_group *mod_mask)
{
	return 0;
}

static int vxldap_groupdel(struct vxpdb_state *vp,
    const struct vxpdb_group *sr_mask)
{
	struct ldap_state *state = vp->state;
	hmc_t *dn;
	int ret;

	dn  = dn_group(state, sr_mask);
	ret = ldap_delete_ext_s(state->conn, dn, NULL, NULL);
	if (ret != LDAP_SUCCESS) {
		hmc_free(dn);
		return -(errno = 1600 + ret);
	}
	hmc_free(dn);
	return 1;
}

static void vxldap_copy_group(struct vxpdb_group *dest, LDAP *conn,
    LDAPMessage *entry)
{
	BerElement *ber;
	char *attr;

	hmc_strasg(&dest->gr_name, NULL);
	dest->gr_gid = PDB_NOGID;

	for (attr = ldap_first_attribute(conn, entry, &ber); attr != NULL;
	    attr = ldap_next_attribute(conn, entry, ber))
	{
		char **val;

		val = ldap_get_values(conn, entry, attr);
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
	return;
}

static int vxldap_getgrx(struct ldap_state *state, const char *filter,
    struct vxpdb_group *dest)
{
	LDAPMessage *result;
	int ret;

	ret = ldap_search_ext_s(state->conn, state->group_suffix,
	      LDAP_SCOPE_SUBTREE, filter, NULL, false,
	      NULL, NULL, NULL, 1, &result);
	if (ret != LDAP_SUCCESS || result == NULL)
		return -(errno = 1600 + ret);

	vxldap_copy_group(dest, state->conn, result);
	ldap_msgfree(result);
	return 1;
}

static int vxldap_getgrgid(struct vxpdb_state *vp, long gid,
    struct vxpdb_group *dest)
{
	char filter[48+Z_32];
	snprintf(filter, sizeof(filter),
	         "(&(objectClass=posixGroup)(gidNumber=%lu))", gid);
	return vxldap_getgrx(vp->state, filter, dest);
}

static int vxldap_getgrnam(struct vxpdb_state *vp, const char *user,
    struct vxpdb_group *dest)
{
	hmc_t *filter;
	int ret;

	filter = hmc_sinit("(&(objectClass=posixGroup)(cn=" /* )) */);
	hmc_strcat(&filter, user);
	hmc_strcat(&filter, /* (( */ "))");
	ret = vxldap_getgrx(vp->state, filter, dest);
	hmc_free(filter);
	return ret;
}

static void *vxldap_grouptrav_init(struct vxpdb_state *vp)
{
	struct ldap_state *state = vp->state;
	struct ldap_trav trav;
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

static int vxldap_grouptrav_walk(struct vxpdb_state *vp, void *ptr,
    struct vxpdb_group *dest)
{
	struct ldap_state *state = vp->state;
	struct ldap_trav *trav   = ptr;

	if (trav->current == NULL)
		return 0;

	vxldap_copy_group(dest, state->conn, trav->current);
	trav->current = ldap_next_entry(state->conn, trav->current);
	return 1;
}

static void vxldap_grouptrav_free(struct vxpdb_state *vp, void *ptr)
{
	struct ldap_trav *trav = ptr;
	ldap_msgfree(trav->base);
	free(trav);
	return;
}

static struct vxpdb_driver THIS_MODULE = {
	.name           = "LDAP back-end module",
	.init           = vxldap_init,
	.open           = vxldap_open,
	.close          = vxldap_close,
	.exit           = vxldap_exit,
	.useradd        = vxldap_useradd,
	.usermod        = vxldap_usermod,
	.userdel        = vxldap_userdel,
	.getpwuid       = vxldap_getpwuid,
	.getpwnam       = vxldap_getpwnam,
	.usertrav_init  = vxldap_usertrav_init,
	.usertrav_walk  = vxldap_usertrav_walk,
	.usertrav_free  = vxldap_usertrav_free,
	.groupadd       = vxldap_groupadd,
	.groupmod       = vxldap_groupmod,
	.groupdel       = vxldap_groupdel,
	.getgrgid       = vxldap_getgrgid,
	.getgrnam       = vxldap_getgrnam,
	.grouptrav_init = vxldap_grouptrav_init,
	.grouptrav_walk = vxldap_grouptrav_walk,
	.grouptrav_free = vxldap_grouptrav_free,
};

REGISTER_MODULE(ldap, &THIS_MODULE);
