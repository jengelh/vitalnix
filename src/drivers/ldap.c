/*
 *	ldap.c - LDAP back-end
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2009
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ldap.h>
#include <libHX/defs.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxdb/libvxdb.h>

#define F_POSIXACCOUNT "objectClass=posixAccount"
#define F_POSIXGROUP   "objectClass=posixGroup"

struct ldap_attrmap {
	bool posixAccount, shadowAccount, sambaSamAccount;
	bool vitalnixManagedAccount;
	bool cn, userPassword, loginShell, gecos, shadowLastChange, shadowMin;
	bool shadowMax, shadowWarning, shadowInactive, shadowExpire;
	bool sambaNTPassword;
	bool vitalnixUUID, vitalnixGroup, vitalnixDeferTimer;
};

struct ldap_state {
	LDAP *conn;
	char *uri, *root_dn;
	hxmc_t *root_pw;
	char *user_suffix, *group_suffix;
	char *domain_dn, *domain_sid;
	unsigned int domain_algoridbase;
	unsigned int uid_min, uid_max, gid_min, gid_max;
};

struct ldap_trav {
	LDAPMessage *base, *current;
};

/* Functions */
static void vxldap_close(struct vxdb_state *);

/* Variables */
static const char *const no_attrs[] = {"", NULL};

//-----------------------------------------------------------------------------
static int vxldap_errno(int ld_errno)
{
	switch (ld_errno) {
		case LDAP_SUCCESS:			/* 0 */
			return 0;
		case LDAP_OPERATIONS_ERROR:		/* 1 */
		case LDAP_PROTOCOL_ERROR:		/* 2 */
		case LDAP_UNAVAILABLE:			/* 52 */
			return EIO;
		case LDAP_TIMELIMIT_EXCEEDED:		/* 3 */
			return ETIMEDOUT;
		case LDAP_SIZELIMIT_EXCEEDED:		/* 4 */
			return ERANGE;
		case LDAP_AUTH_METHOD_NOT_SUPPORTED:	/* 7 */
		case LDAP_STRONG_AUTH_REQUIRED:		/* 8 */
#ifdef LDAP_CONFIDENTALITY_REQUIRED
		case LDAP_CONFIDENTALITY_REQUIRED:	/* 19 */
#endif
		case LDAP_INAPPROPRIATE_AUTH:		/* 48 */
			return EPERM;
		case LDAP_ADMINLIMIT_EXCEEDED:		/* 11 */
			return EMFILE;
		case LDAP_NO_SUCH_ATTRIBUTE:		/* 16 */
		case LDAP_NO_SUCH_OBJECT:		/* 32 */
			return ENOENT;
		case LDAP_UNDEFINED_TYPE:		/* 17 */
		case LDAP_INAPPROPRIATE_MATCHING:	/* 18 */
		case LDAP_CONSTRAINT_VIOLATION:		/* 19 */
		case LDAP_INVALID_DN_SYNTAX:		/* 34 */
		case LDAP_UNWILLING_TO_PERFORM:		/* 53 */ /* EACCES */
		case LDAP_NAMING_VIOLATION:		/* 64 */
		case LDAP_OBJECT_CLASS_VIOLATION:	/* 65 */
		case LDAP_NOT_ALLOWED_ON_NONLEAF:	/* 66 */
		case LDAP_NOT_ALLOWED_ON_RDN:		/* 67 */
		case LDAP_NO_OBJECT_CLASS_MODS:		/* 69 */
			return EINVAL;
		case LDAP_TYPE_OR_VALUE_EXISTS:		/* 20 */
		case LDAP_ALREADY_EXISTS:		/* 68 */
			return EEXIST;
		case LDAP_ALIAS_DEREF_PROBLEM:		/* 36 */
		case LDAP_INVALID_CREDENTIALS:		/* 49 */
		case LDAP_INSUFFICIENT_ACCESS:		/* 50 */
			return EACCES;
		case LDAP_BUSY:				/* 51 */
			return EBUSY;
		case LDAP_LOOP_DETECT:			/* 54 */
			return ELOOP;
		default:
			fprintf(stderr, "LDAP error %d: %s\n", ld_errno,
			        ldap_err2string(ld_errno));
			return 1600 + ld_errno;
	}
}

static int vxldap_errno_sp(int ld_errno, const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	fprintf(stderr, ": %s\n", ldap_err2string(ld_errno));
	va_end(argp);
	return -(errno = vxldap_errno(ld_errno));
}

static void vxldap_read_ldap_secret(const struct HXoptcb *cbi)
{
	hxmc_t **pw = cbi->current->uptr;
	FILE *fp;
	if ((fp = fopen(cbi->data, "r")) == NULL)
		return;
	*pw = HXmc_meminit(NULL, 0);
	HX_getl(pw, fp);
	HX_chomp(*pw);
}

static void vxldap_read_config(struct ldap_state *state, const char *file,
    bool deallocate)
{
	struct HXoption autouid_table[] = {
		{.ln = "UID_MIN", .type = HXTYPE_UINT, .ptr = &state->uid_min},
		{.ln = "UID_MAX", .type = HXTYPE_UINT, .ptr = &state->uid_max},
		{.ln = "GID_MIN", .type = HXTYPE_UINT, .ptr = &state->gid_min},
		{.ln = "GID_MAX", .type = HXTYPE_UINT, .ptr = &state->gid_max},
		HXOPT_TABLEEND,
	};
	struct HXoption options_table[] = {
		{.ln = "URI",          .type = HXTYPE_STRING, .ptr = &state->uri},
		{.ln = "USER_SUFFIX",  .type = HXTYPE_STRING, .ptr = &state->user_suffix},
		{.ln = "GROUP_SUFFIX", .type = HXTYPE_STRING, .ptr = &state->group_suffix},
		{.ln = "ROOT_DN",      .type = HXTYPE_STRING, .ptr = &state->root_dn},
		{.ln = "ROOT_PWFILE",  .type = HXTYPE_STRING, .cb  = vxldap_read_ldap_secret, .uptr = &state->root_pw},
		{.ln = "DOMAIN_DN",    .type = HXTYPE_STRING, .ptr = &state->domain_dn},
		HXOPT_TABLEEND,
	};

	if (deallocate) {
		HX_shconfig_free(options_table);
		return;
	}

	state->uid_min      = state->gid_min = 1000;
	state->uid_max      = state->gid_max = 60000;
	HX_shconfig(CONFIG_SYSCONFDIR "/autouid.conf", autouid_table);
	HX_shconfig(file, options_table);
}

static int vxldap_init(struct vxdb_state *vp, const char *config_file)
{
	struct ldap_state *state;

	fprintf(stderr, "[ Experimental Vitalnix LDAP support. ]\n");

	if ((state = vp->state = calloc(1, sizeof(struct ldap_state))) == NULL)
		return -errno;

	state->domain_algoridbase = 1000;
	vxldap_read_config(state, config_file, false);
	return 1;
}

/**
 * vxldap_get_rid - get Samba RID number
 * @state:	vxldap internal structure
 *
 * Gets the domain SID and the RID base value and stores it in @state. On
 * success, when both attributes have been retrieved, returns positive
 * non-zero. If that is not the case, returns zero. On failure, returns the
 * libldap error code.
 */
static int vxldap_get_rid(struct ldap_state *state)
{
	static const char *const attrs[] =
		{"sambaSID", "sambaAlgorithmicRidBase", NULL};
	LDAPMessage *result, *entry;
	char *attr, **val;
	BerElement *ber;
	int ret;

	ret = ldap_search_ext_s(state->conn, state->domain_dn,
	      LDAP_SCOPE_BASE, NULL, const_cast2(char **, attrs),
	      false, NULL, NULL, NULL, 1, &result);
	if (ret == LDAP_NO_SUCH_OBJECT)
		return 0;
	if (ret != LDAP_SUCCESS)
		return ret;

	entry = ldap_first_entry(state->conn, result);
	if (entry == NULL) {
		ldap_msgfree(result);
		return 0;
	}

	for (attr = ldap_first_attribute(state->conn, entry, &ber);
	    attr != NULL; attr = ldap_next_attribute(state->conn, entry, ber))
	{
		val = ldap_get_values(state->conn, entry, attr);
		if (val == NULL)
			continue;
		if (*val == NULL) {
			ldap_value_free(val);
			ldap_memfree(attr);
			continue;
		}
		if (strcmp(attr, "sambaSID") == 0)
			HXmc_strcpy(&state->domain_sid, *val);
		else if (strcmp(attr, "sambaAlgorithmicRidBase") == 0)
			state->domain_algoridbase = strtoul(*val, NULL, 0);
		ldap_value_free(val);
		ldap_memfree(attr);
	}

	if (ber != NULL)
		ber_free(ber, false);
	ldap_msgfree(result);
	return state->domain_sid != NULL && state->domain_algoridbase != 0;
}

static int vxldap_open(struct vxdb_state *vp, unsigned int flags)
{
	struct ldap_state *state = vp->state;
	int ret;

	if (state->uri == NULL)
		return -EINVAL;

	ret = ldap_initialize(&state->conn, state->uri);
	if (ret != LDAP_SUCCESS)
		return vxldap_errno_sp(ret, "ldap_initialize");

	ret = LDAP_VERSION3;
	ldap_set_option(state->conn, LDAP_OPT_PROTOCOL_VERSION, &ret);
	ret = LDAP_MAXINT;
	ret = ldap_set_option(state->conn, LDAP_OPT_SIZELIMIT, &ret);

	if (flags & (VXDB_ADMIN | VXDB_WRLOCK)) {
		if (ret != LDAP_SUCCESS) {
			ldap_perror(state->conn, "Could not raise LDAP search "
			            "limit to maximum, but we need it!\n");
			vxldap_close(vp);
			return vxldap_errno_sp(ret, "ldap_set_option("
			       "LDAP_OPT_SIZELIMIT)");
		}
		ldap_start_tls_s(state->conn, NULL, NULL);
		ret = ldap_simple_bind_s(state->conn, state->root_dn,
		      state->root_pw);
		if (ret != LDAP_SUCCESS)
			ldap_perror(state->conn, "Simple bind to ldaproot "
			            "account failed, continuing");

		if (state->domain_dn != NULL) {
			ret = vxldap_get_rid(state);
			if (ret == 0)
				/*
				 * Samba usually creates the sambaDomain object
				 * with both fields.
				 */
				fprintf(stderr, "Warning: Not all required "
				        "domain attributes have been found! "
				        "Using defaults.\n");
			else if (ret < 0)
				ldap_perror(state->conn, "Problem retrieving "
				            "domain attributes");
		}
	}

	return 1;
}

static void vxldap_close(struct vxdb_state *vp)
{
	struct ldap_state *state = vp->state;
	ldap_unbind_ext(state->conn, NULL, NULL);
}

static void vxldap_exit(struct vxdb_state *vp)
{
	struct ldap_state *state = vp->state;
	vxldap_read_config(state, NULL, true);
	HXmc_free(state->root_pw);
	free(state);
}

/**
 * vxldap_count - count results of a search
 * @conn:	libldap structure
 * @base:	basedn to start search at
 * @filter:	object filter
 *
 * Returns the number of results on success, or the libldap error code as
 * negative value on failure.
 */
static unsigned int vxldap_count(LDAP *conn, const char *base,
    const char *filter)
{
	LDAPMessage *result, *entry;
	unsigned int count = 0;
	int ret;

	ret = ldap_search_ext_s(conn, base, LDAP_SCOPE_SUBTREE, filter,
	      const_cast2(char **, no_attrs), true, NULL, NULL,
	      NULL, LDAP_MAXINT, &result);
	if (ret != LDAP_SUCCESS || result == NULL)
		return -ret;

	for (entry = ldap_first_entry(conn, result); entry != NULL;
	    entry = ldap_next_entry(conn, entry))
		++count;

	ldap_msgfree(result);
	return count;
}

static long vxldap_modctl(struct vxdb_state *vp, unsigned int command, ...)
{
	struct ldap_state *state = vp->state;
	int ret;
	errno = 0;
	switch (command) {
		case VXDB_COUNT_USERS:
			ret = vxldap_count(state->conn, state->user_suffix,
			      F_POSIXACCOUNT);
			break;
		case VXDB_COUNT_GROUPS:
			ret = vxldap_count(state->conn, state->group_suffix,
			      F_POSIXGROUP);
			break;
		default:
			return -ENOSYS;
	}
	if (ret < 0)
		return vxldap_errno_sp(-ret, "vxldap_count");
	return ret;
}

static hxmc_t *dn_user(const struct ldap_state *state, const char *name)
{
	hxmc_t *ret;
	if (name == NULL)
		return NULL;
	ret = HXmc_strinit("uid=");
	HXmc_strcat(&ret, name);
	HXmc_strcat(&ret, ",");
	HXmc_strcat(&ret, state->user_suffix);
	return ret;
}

static inline int vxldap_uid_to_sid(struct ldap_state *state, char *sid,
    size_t sid_size, unsigned int uid)
{
	/* samba-3.0.25/source/include/rpc_misc.h */
	enum {
		RID_MULTIPLIER = 2,
		USER_RID_TYPE  = 0,
	};
	return snprintf(sid, sid_size, "%s-%u", state->domain_sid,
	       (uid * RID_MULTIPLIER + state->domain_algoridbase) |
	       USER_RID_TYPE);
}

static int vxldap_useradd(struct vxdb_state *vp, const struct vxdb_user *rq)
{
	struct ldap_state *state = vp->state;
	char s_pw_uid[HXSIZEOF_Z32], s_pw_gid[HXSIZEOF_Z32], s_sp_last[HXSIZEOF_Z32];
	char s_sp_min[HXSIZEOF_Z32], s_sp_max[HXSIZEOF_Z32], s_sp_warn[HXSIZEOF_Z32];
	char s_sp_expire[HXSIZEOF_Z32], s_sp_inact[HXSIZEOF_Z32], s_vs_defer[HXSIZEOF_Z32];
	char s_sid[256], s_smblastchg[HXSIZEOF_Z32];
	LDAPMod attr[21], *attr_ptrs[22];
	const char *object_classes[6];
	unsigned int a = 0, i, o = 0;
	hxmc_t *dn, *password = NULL;
	int ret;

	if ((dn = dn_user(state, rq->pw_name)) == NULL)
		return -ENOMEM;

	object_classes[o++] = "account";
	object_classes[o++] = "posixAccount";
	if (rq->sp_lastchg > 0 || rq->sp_min != VXDB_DFL_KEEPMIN ||
	    rq->sp_max != VXDB_DFL_KEEPMAX || rq->sp_warn != VXDB_DFL_WARNAGE ||
	    rq->sp_expire != VXDB_NO_EXPIRE || rq->sp_inact != VXDB_NO_INACTIVE)
		object_classes[o++] = "shadowAccount";
	if (rq->vs_uuid != NULL || rq->vs_pvgrp != NULL || rq->vs_defer != 0)
		object_classes[o++] = "vitalnixManagedAccount";
	if (rq->sp_ntpasswd != NULL && state->domain_sid != NULL)
		object_classes[o++] = "sambaSamAccount";
	object_classes[o] = NULL;

	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "objectClass",
		.mod_values = const_cast2(char **, object_classes),
	};
	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "uid",
		.mod_values = (char *[]){rq->pw_name, NULL},
	};

	/* posixAccount */
	/* FIXME: Handle VXDB_AUTOUID */
	snprintf(s_pw_uid, sizeof(s_pw_uid), "%u",
	         static_cast(unsigned int, rq->pw_uid));
	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "uidNumber",
		.mod_values = (char *[]){s_pw_uid, NULL},
	};
	snprintf(s_pw_gid, sizeof(s_pw_gid), "%u", rq->pw_gid);
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

	if (rq->sp_passwd != NULL) {
		password = HXmc_strinit(rq->sp_passwd);
		HXmc_strpcat(&password, "{crypt}");
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "userPassword",
			.mod_values = (char *[]){password, NULL},
		};
	}

	/* shadowAccount */
	if (rq->sp_lastchg > 0) {
		snprintf(s_sp_last, sizeof(s_sp_last), "%lu", rq->sp_lastchg);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowLastChange",
			.mod_values = (char *[]){s_sp_last, NULL},
		};
	}
	if (rq->sp_min != VXDB_DFL_KEEPMIN) {
		snprintf(s_sp_min, sizeof(s_sp_min), "%lu", rq->sp_min);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowMin",
			.mod_values = (char *[]){s_sp_min, NULL},
		};
	}
	if (rq->sp_max != VXDB_DFL_KEEPMAX) {
		snprintf(s_sp_max, sizeof(s_sp_max), "%lu", rq->sp_max);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowMax",
			.mod_values = (char *[]){s_sp_max, NULL},
		};
	}
	if (rq->sp_warn != VXDB_DFL_WARNAGE) {
		snprintf(s_sp_warn, sizeof(s_sp_warn), "%lu", rq->sp_warn);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowWarning",
			.mod_values = (char *[]){s_sp_warn, NULL},
		};
	}
	if (rq->sp_expire != VXDB_NO_EXPIRE) {
		snprintf(s_sp_expire, sizeof(s_sp_expire), "%lu", rq->sp_expire);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowExpire",
			.mod_values = (char *[]){s_sp_expire, NULL},
		};
	}
	if (rq->sp_inact != VXDB_NO_INACTIVE) {
		snprintf(s_sp_inact, sizeof(s_sp_inact), "%lu", rq->sp_inact);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "shadowInactive",
			.mod_values = (char *[]){s_sp_inact, NULL},
		};
	}

	/* sambaSamAccount */
	if (rq->sp_ntpasswd != NULL && state->domain_sid != NULL &&
	    vxldap_uid_to_sid(state, s_sid, sizeof(s_sid), rq->pw_uid) > 0) {
	    	if (rq->sp_lastchg > 0)
			snprintf(s_smblastchg, sizeof(s_smblastchg),
			         "%lu", rq->sp_lastchg * 86400);
		else
			snprintf(s_smblastchg, sizeof(s_smblastchg),
			         "%lu", time(NULL));
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "sambaSID",
			.mod_values = (char *[]){s_sid, NULL},
		};
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "sambaNTPassword",
			.mod_values = (char *[]){rq->sp_ntpasswd, NULL},
		};
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "sambaPwdLastSet",
			.mod_values = (char *[]){s_smblastchg, NULL},
		};
	}

	/* vitalnixManagedAccount */
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
			.mod_values = (char *[]){rq->vs_uuid, NULL},
		};
	}
	if (rq->vs_defer != 0) {
		snprintf(s_vs_defer, sizeof(s_vs_defer), "%u", rq->vs_defer);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "vitalnixDeferTimer",
			.mod_values = (char *[]){s_vs_defer, NULL},
		};
	}

	for (i = 0; i < a; ++i)
		attr_ptrs[i] = &attr[i];
	attr_ptrs[i] = NULL;
	ret = ldap_add_ext_s(state->conn, dn, attr_ptrs, NULL, NULL);

	HXmc_free(dn);
	HXmc_free(password);
	if (ret != LDAP_SUCCESS)
		return vxldap_errno_sp(ret, "vxldap_useradd");

	return 1;
}

static void vxldap_getattr(struct ldap_state *state, const char *dn,
    struct ldap_attrmap *xa)
{
	static const char *const attrs[] =
		{"objectClass", "userPassword", "loginShell", "gecos",
		"shadowLastChange", "shadowMin", "shadowMax", "shadowWarning",
		"shadowInactive", "shadowExpire", "sambaNTPassword",
		"vitalnixUUID", "vitalnixGroup", "vitalnixDeferTimer", NULL};
	LDAPMessage *result, *entry;
	char *attr, **vals, **val_ptr;
	BerElement *ber;
	int ret;

	ret = ldap_search_ext_s(state->conn, dn, LDAP_SCOPE_BASE, NULL,
	      const_cast2(char **, attrs), false, NULL, NULL, NULL, 1, &result);
	if (ret != LDAP_SUCCESS)
		return;

	entry = ldap_first_entry(state->conn, result);
	if (entry == NULL) {
		ldap_msgfree(result);
		return;
	}

	for (attr = ldap_first_attribute(state->conn, entry, &ber);
	    attr != NULL; attr = ldap_next_attribute(state->conn, entry, ber))
	{
		if (strcmp(attr, "cn") == 0)
			xa->cn = true;
		else if (strcmp(attr, "userPassword") == 0)
			xa->userPassword = true;
		else if (strcmp(attr, "loginShell") == 0)
			xa->loginShell = true;
		else if (strcmp(attr, "gecos") == 0)
			xa->gecos = true;
		else if (strcmp(attr, "shadowLastChange") == 0)
			xa->shadowLastChange = true;
		else if (strcmp(attr, "shadowMin") == 0)
			xa->shadowMin = true;
		else if (strcmp(attr, "shadowMax") == 0)
			xa->shadowMax = true;
		else if (strcmp(attr, "shadowWarning") == 0)
			xa->shadowWarning = true;
		else if (strcmp(attr, "shadowExpire") == 0)
			xa->shadowExpire = true;
		else if (strcmp(attr, "shadowInactive") == 0)
			xa->shadowInactive = true;
		else if (strcmp(attr, "sambaNTPassword") == 0)
			xa->sambaNTPassword = true;
		else if (strcmp(attr, "vitalnixUUID") == 0)
			xa->vitalnixUUID = true;
		else if (strcmp(attr, "vitalnixGroup") == 0)
			xa->vitalnixGroup = true;
		else if (strcmp(attr, "vitalnixDeferTimer") == 0)
			xa->vitalnixDeferTimer = true;
		
		if (strcmp(attr, "objectClass") != 0) {
			ldap_memfree(attr);
			continue;
		}

		vals = ldap_get_values(state->conn, entry, attr);
		if (vals == NULL) {
			ldap_memfree(attr);
			continue;
		}

		for (val_ptr = vals; *val_ptr != NULL; ++val_ptr)
			if (strcmp(*val_ptr, "posixAccount") == 0)
				xa->posixAccount = true;
			else if (strcmp(*val_ptr, "shadowAccount") == 0)
				xa->shadowAccount = true;
			else if (strcmp(*val_ptr, "sambaSamAccount") == 0)
				xa->sambaSamAccount = true;
			else if (strcmp(*val_ptr, "vitalnixManagedAccount") == 0)
				xa->vitalnixManagedAccount = true;

		ldap_value_free(vals);
		ldap_memfree(attr);
	}
	if (ber != NULL)
		ber_free(ber, false);
	ldap_msgfree(result);
}

/**
 * vxldap_usermod2 - handle DN rename
 * @state:	vxldap internal structure
 * @old_dn:	old/current DN
 * @new_name:	new username
 */
static int vxldap_usermod2(struct ldap_state *state, hxmc_t *old_dn,
    const char *new_name)
{
	hxmc_t *new_rdn;
	int ret;

	new_rdn = HXmc_strinit("uid=");
	HXmc_strcat(&new_rdn, new_name);
	ret = ldap_rename_s(state->conn, old_dn, new_rdn,
	      NULL, false, NULL, NULL);
	HXmc_free(new_rdn);
	if (ret != LDAP_SUCCESS) {
		ldap_perror(state->conn, "vxldap_usermod2: The DN was not "
		            "modified. You NEED to fix this up!\n");
		return -(errno = vxldap_errno(ret));
	}

	return 1;
}

#define repl_add(x) ((x) ? LDAP_MOD_REPLACE : LDAP_MOD_ADD)

static int vxldap_usermod(struct vxdb_state *vp, const char *name,
    const struct vxdb_user *param)
{
	struct ldap_state *state = vp->state;
	char s_pw_uid[HXSIZEOF_Z32], s_pw_gid[HXSIZEOF_Z32], s_sp_last[HXSIZEOF_Z32];
	char s_sp_min[HXSIZEOF_Z32], s_sp_max[HXSIZEOF_Z32], s_sp_warn[HXSIZEOF_Z32];
	char s_sp_expire[HXSIZEOF_Z32], s_sp_inact[HXSIZEOF_Z32], s_vs_defer[HXSIZEOF_Z32];
	struct ldap_attrmap attr_map = {};
	LDAPMod attr[20], *attr_ptrs[21];
	hxmc_t *dn, *password = NULL;
	unsigned int a = 0, i;
	int ret;

	if ((dn = dn_user(state, name)) == NULL)
		return -ENOMEM;

	vxldap_getattr(state, dn, &attr_map);
	if (!attr_map.posixAccount)
		return -ENOENT;

	/* posixAccount */
	if (param->pw_uid != VXDB_NO_CHANGE) {
		snprintf(s_pw_uid, sizeof(s_pw_uid), "%u", param->pw_uid);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_REPLACE,
			.mod_type   = "uidNumber",
			.mod_values = (char *[]){s_pw_uid, NULL},
		};
	}
	if (param->pw_gid != VXDB_NO_CHANGE) {
		snprintf(s_pw_gid, sizeof(s_pw_gid), "%u", param->pw_gid);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_REPLACE,
			.mod_type   = "gidNumber",
			.mod_values = (char *[]){s_pw_gid, NULL},
		};
	}
	if (param->pw_real != NULL) {
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.gecos),
			.mod_type   = "gecos",
			.mod_values = (char *[]){param->pw_real, NULL},
		};
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.cn),
			.mod_type   = "cn",
			.mod_values = (char *[]){param->pw_real, NULL},
		};
	}
	if (param->pw_home != NULL)
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_REPLACE,
			.mod_type   = "homeDirectory",
			.mod_values = (char *[]){param->pw_home, NULL},
		};
	if (param->pw_shell != NULL)
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.loginShell),
			.mod_type   = "loginShell",
			.mod_values = (char *[]){param->pw_shell, NULL},
		};
	if (param->sp_passwd != NULL) {
		password = HXmc_strinit(param->sp_passwd);
		HXmc_strpcat(&password, "{crypt}");
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.userPassword),
			.mod_type   = "userPassword",
			.mod_values = (char *[]){password, NULL},
		};
	}

	/* shadowAccount */
	if (!attr_map.shadowAccount && (param->sp_lastchg != VXDB_NO_CHANGE ||
	    param->sp_min != VXDB_NO_CHANGE ||
	    param->sp_max != VXDB_NO_CHANGE ||
	    param->sp_warn != VXDB_NO_CHANGE ||
	    param->sp_expire != VXDB_NO_CHANGE ||
	    param->sp_inact != VXDB_NO_CHANGE))
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "objectClass",
			.mod_values = (char *[]){"shadowAccount", NULL},
		};
	if (param->sp_lastchg != VXDB_NO_CHANGE) {
		snprintf(s_sp_last, sizeof(s_sp_last), "%lu", param->sp_lastchg);
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.shadowLastChange),
			.mod_type   = "shadowLastChange",
			.mod_values = (char *[]){s_sp_last, NULL},
		};
	}
	if (param->sp_min != VXDB_NO_CHANGE) {
		snprintf(s_sp_min, sizeof(s_sp_min), "%lu", param->sp_min);
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.shadowMin),
			.mod_type   = "shadowMin",
			.mod_values = (char *[]){s_sp_min, NULL},
		};
	}
	if (param->sp_max != VXDB_NO_CHANGE) {
		snprintf(s_sp_max, sizeof(s_sp_max), "%lu", param->sp_max);
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.shadowMax),
			.mod_type   = "shadowMax",
			.mod_values = (char *[]){s_sp_max, NULL},
		};
	}
	if (param->sp_warn != VXDB_NO_CHANGE) {
		snprintf(s_sp_warn, sizeof(s_sp_warn), "%lu", param->sp_warn);
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.shadowWarning),
			.mod_type   = "shadowWarning",
			.mod_values = (char *[]){s_sp_warn, NULL},
		};
	}
	if (param->sp_expire != VXDB_NO_CHANGE) {
		snprintf(s_sp_expire, sizeof(s_sp_expire),
		         "%lu", param->sp_expire);
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.shadowExpire),
			.mod_type   = "shadowExpire",
			.mod_values = (char *[]){s_sp_expire, NULL},
		};
	}
	if (param->sp_inact != VXDB_NO_CHANGE) {
		snprintf(s_sp_inact, sizeof(s_sp_inact),
		         "%lu", param->sp_inact);
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_REPLACE,
			.mod_type   = "shadowInactive",
			.mod_values = (char *[]){s_sp_inact, NULL},
		};
	}

	/* sambaSamAccount */
	if (!attr_map.sambaSamAccount && param->sp_ntpasswd != NULL)
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "objectClass",
			.mod_values = (char *[]){"sambaSamAccount", NULL},
		};
	if (param->sp_ntpasswd != NULL)
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.sambaNTPassword),
			.mod_type   = "sambaNTPassword",
			.mod_values = (char *[]){param->sp_ntpasswd, NULL},
		};

	/* vitalnixManagedAccount */
	if (!attr_map.vitalnixManagedAccount && (param->vs_uuid != NULL ||
	    param->vs_pvgrp != NULL || param->vs_defer != VXDB_NO_CHANGE))
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_ADD,
			.mod_type   = "objectClass",
			.mod_values = (char *[]){"vitalnixManagedAccount", NULL},
		};
	if (param->vs_uuid != NULL)
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.vitalnixUUID),
			.mod_type   = "vitalnixUUID",
			.mod_values = (char *[]){param->vs_uuid, NULL},
		};
	if (param->vs_pvgrp != NULL)
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.vitalnixGroup),
			.mod_type   = "vitalnixGroup",
			.mod_values = (char *[]){param->vs_pvgrp, NULL},
		};
	if (param->vs_defer != VXDB_NO_CHANGE) {
		snprintf(s_vs_defer, sizeof(s_vs_defer),
		         "%u", param->vs_defer);
		attr[a++] = (LDAPMod){
			.mod_op     = repl_add(attr_map.vitalnixDeferTimer),
			.mod_type   = "vitalnixDeferTimer",
			.mod_values = (char *[]){s_vs_defer, NULL},
		};
	}

	for (i = 0; i < a; ++i)
		attr_ptrs[i] = &attr[i];
	attr_ptrs[i] = NULL;

	ret = ldap_modify_ext_s(state->conn, dn, attr_ptrs, NULL, NULL);
	if (ret == LDAP_NO_SUCH_OBJECT) {
		return -ENOENT;
	} else if (ret != LDAP_SUCCESS) {
		HXmc_free(dn);
		return vxldap_errno_sp(ret, "vxldap_usermod");
	}

	if (param->pw_name == NULL) {
		HXmc_free(dn);
		return 1;
	}

	/* Rename requested. */
	return vxldap_usermod2(state, dn, param->pw_name);
}

static int vxldap_userdel(struct vxdb_state *vp, const char *name)
{
	struct ldap_state *state = vp->state;
	hxmc_t *dn;
	int ret;

	if ((dn = dn_user(state, name)) == NULL)
		return -ENOMEM;

	ret = ldap_delete_ext_s(state->conn, dn, NULL, NULL);
	HXmc_free(dn);
	if (ret == LDAP_NO_SUCH_OBJECT)
		return 0;
	else if (ret != LDAP_SUCCESS)
		return vxldap_errno_sp(ret, "vxldap_userdel");

	return 1;
}

static void vxldap_copy_user(struct vxdb_user *dest, LDAP *conn,
    LDAPMessage *entry)
{
	char *attr, **val;
	BerElement *ber;

	vxdb_user_clean(dest);
	for (attr = ldap_first_attribute(conn, entry, &ber); attr != NULL;
	    attr = ldap_next_attribute(conn, entry, ber))
	{
		val = ldap_get_values(conn, entry, attr);
		if (val == NULL)
			continue;
		if (*val == NULL) {
			ldap_value_free(val);
			ldap_memfree(attr);
			continue;
		}
		if (strcmp(attr, "uid") == 0)
			HXmc_strcpy(&dest->pw_name, *val);
		else if (strcmp(attr, "uidNumber") == 0)
			dest->pw_uid = strtoul(*val, NULL, 0);
		else if (strcmp(attr, "gidNumber") == 0)
			dest->pw_gid = strtoul(*val, NULL, 0);
		else if (strcmp(attr, "gecos") == 0)
			HXmc_strcpy(&dest->pw_real, *val);
		else if (strcmp(attr, "homeDirectory") == 0)
			HXmc_strcpy(&dest->pw_home, *val);
		else if (strcmp(attr, "loginShell") == 0)
			HXmc_strcpy(&dest->pw_shell, *val);
		else if (strcmp(attr, "userPassword") == 0)
			HXmc_strcpy(&dest->sp_passwd, *val);
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
			HXmc_strcpy(&dest->vs_uuid, *val);
		else if (strcmp(attr, "vitalnixGroup") == 0)
			HXmc_strcpy(&dest->vs_pvgrp, *val);
		ldap_value_free(val);
		ldap_memfree(attr);
	}

	if (ber != NULL)
		ber_free(ber, false);
}

static int vxldap_getpwx(struct ldap_state *state, const char *filter,
    struct vxdb_user *dest)
{
	LDAPMessage *result, *entry;
	int ret;

	ret = ldap_search_ext_s(state->conn, state->user_suffix,
	      LDAP_SCOPE_SUBTREE, filter, NULL, false, NULL, NULL, NULL, 1,
	      &result);
	if (ret != LDAP_SUCCESS || result == NULL)
		return -(errno = vxldap_errno(ret));
	entry = ldap_first_entry(state->conn, result);
	if (entry == NULL) {
		ldap_msgfree(result);
		return -ENOENT;
	}
	if (dest != NULL)
		vxldap_copy_user(dest, state->conn, result);
	ldap_msgfree(result);
	return 1;
}

static int vxldap_getpwuid(struct vxdb_state *vp, unsigned int uid,
    struct vxdb_user *dest)
{
	char filter[48+HXSIZEOF_Z32];
	int ret;
	snprintf(filter, sizeof(filter),
	         "(&(" F_POSIXACCOUNT ")(uidNumber=%u))", uid);
	ret = vxldap_getpwx(vp->state, filter, dest);
	return (ret == -ENOENT) ? 0 : ret;
}

static int vxldap_getpwnam(struct vxdb_state *vp, const char *user,
    struct vxdb_user *dest)
{
	hxmc_t *filter;
	int ret;

	filter = HXmc_strinit("(&(" F_POSIXACCOUNT ")(uid=" /* )) */);
	HXmc_strcat(&filter, user);
	HXmc_strcat(&filter, /* (( */ "))");
	ret = vxldap_getpwx(vp->state, filter, dest);
	HXmc_free(filter);
	return (ret == -ENOENT) ? 0 : ret;
}

static void *vxldap_usertrav_init(struct vxdb_state *vp)
{
	static const char *const attrs[] = {
		"uid", "uidNumber", "gidNumber", "gecos", "homeDirectory",
		"loginShell", "userPassword", "shadowLastChange",
		"shadowMin", "shadowMax", "shadowWarning", "shadowExpire",
		"shadowInactive", "vitalnixDeferTimer", "vitalnixUUID",
		"vitalnixGroup", NULL,
	};
	struct ldap_state *state = vp->state;
	struct ldap_trav trav;
	int ret;

	ret = ldap_search_ext_s(state->conn, state->user_suffix,
	      LDAP_SCOPE_SUBTREE, F_POSIXACCOUNT, const_cast2(char **, attrs),
	      false, NULL, NULL, NULL, LDAP_MAXINT, &trav.base);
	if (ret != LDAP_SUCCESS) {
		vxldap_errno_sp(ret, "vxldap_usertrav_init");
		return NULL;
	}

	trav.current = ldap_first_entry(state->conn, trav.base);
	return HX_memdup(&trav, sizeof(trav));
}

static int vxldap_usertrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_user *dest)
{
	struct ldap_state *state = vp->state;
	struct ldap_trav *trav   = ptr;

	if (trav->current == NULL)
		return 0;

	vxldap_copy_user(dest, state->conn, trav->current);
	trav->current = ldap_next_entry(state->conn, trav->current);
	return 1;
}

static void vxldap_usertrav_free(struct vxdb_state *vp, void *ptr)
{
	struct ldap_trav *trav = ptr;
	ldap_msgfree(trav->base);
	free(trav);
}

static hxmc_t *dn_group(const struct ldap_state *state, const char *name)
{
	hxmc_t *ret = HXmc_strinit("cn=");
	if (name == NULL)
		return ret;
	HXmc_strcat(&ret, name);
	HXmc_strcat(&ret, ",");
	HXmc_strcat(&ret, state->group_suffix);
	return ret;
}

static int vxldap_groupadd(struct vxdb_state *vp, const struct vxdb_group *rq)
{
	struct ldap_state *state = vp->state;
	LDAPMod attr[4], *attr_ptrs[5];
	unsigned int a = 0, i;
	char s_gr_gid[HXSIZEOF_Z32];
	hxmc_t *dn;
	int ret;

	if ((dn = dn_group(state, rq->gr_name)) == NULL)
		return -ENOMEM;

	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "objectClass",
		.mod_values = (char *[]){"groupOfNames", "posixGroup", NULL},
	};
	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "cn",
		.mod_values = (char *[]){rq->gr_name, NULL},
	};

	/* FIXME: Handle VXDB_AUTOGID */
	snprintf(s_gr_gid, sizeof(s_gr_gid), "%u",
	         static_cast(unsigned int, rq->gr_gid));
	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "gidNumber",
		.mod_values = (char *[]){s_gr_gid, NULL},
	};
	/*
	 * this dummy attribute seems required because groupOfNames has a
	 * "MUST member" in its spec, yet we need to have this group
	 * appear empty. (Resolving the "member" attribute will not yield
	 * a posixAccount.)
	 */
	attr[a++] = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "member",
		.mod_values = (char *[]){dn, NULL},
	};

	for (i = 0; i < a; ++i)
		attr_ptrs[i] = &attr[i];
	attr_ptrs[i] = NULL;
	ret = ldap_add_ext_s(state->conn, dn, attr_ptrs, NULL, NULL);

	HXmc_free(dn);
	if (ret != LDAP_SUCCESS)
		return vxldap_errno_sp(ret, "vxldap_groupadd");

	return 1;
}

static int vxldap_groupmod(struct vxdb_state *vp, const char *name,
    const struct vxdb_group *param)
{
	struct ldap_state *state = vp->state;
	char s_gr_gid[HXSIZEOF_Z32];
	LDAPMod attr[2], *attr_ptrs[3];
	unsigned int a = 0, i;
	hxmc_t *dn;
	int ret;

	if ((dn = dn_group(state, name)) == NULL)
		return -ENOMEM;

	if (param->gr_name != NULL)
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_REPLACE,
			.mod_type   = "cn",
			.mod_values = (char *[]){param->gr_name, NULL},
		};
	if (param->gr_gid != VXDB_NOGID) {
		snprintf(s_gr_gid, sizeof(s_gr_gid), "%u",
		         static_cast(unsigned int, param->gr_gid));
		attr[a++] = (LDAPMod){
			.mod_op     = LDAP_MOD_REPLACE,
			.mod_type   = "gidNumber",
			.mod_values = (char *[]){s_gr_gid, NULL},
		};
	}

	for (i = 0; i < a; ++i)
		attr_ptrs[i] = &attr[i];
	attr_ptrs[i] = NULL;
	ret = ldap_modify_ext_s(state->conn, dn, attr_ptrs, NULL, NULL);

	HXmc_free(dn);
	if (ret != LDAP_SUCCESS)
		return vxldap_errno_sp(ret, "vxldap_groupmod");

	return 1;
}

static int vxldap_groupdel(struct vxdb_state *vp, const char *name)
{
	struct ldap_state *state = vp->state;
	hxmc_t *dn;
	int ret;

	dn  = dn_group(state, name);
	ret = ldap_delete_ext_s(state->conn, dn, NULL, NULL);
	HXmc_free(dn);
	if (ret == LDAP_NO_SUCH_OBJECT)
		return 0;
	else if (ret != LDAP_SUCCESS)
		return vxldap_errno_sp(ret, "vxldap_groupdel");
	return 1;
}

static void vxldap_copy_group(struct vxdb_group *dest, LDAP *conn,
    LDAPMessage *entry)
{
	char *attr, **val;
	BerElement *ber;

	vxdb_group_clean(dest);
	for (attr = ldap_first_attribute(conn, entry, &ber); attr != NULL;
	    attr = ldap_next_attribute(conn, entry, ber))
	{
		val = ldap_get_values(conn, entry, attr);
		if (val == NULL)
			continue;
		if (*val == NULL) {
			ldap_value_free(val);
			ldap_memfree(attr);
			continue;
		}
		if (strcmp(attr, "gidNumber") == 0)
			dest->gr_gid = strtol(*val, NULL, 0);
		else if (strcmp(attr, "cn") == 0)
			HXmc_strcpy(&dest->gr_name, *val);
		ldap_value_free(val);
		ldap_memfree(attr);
	}

	if (ber != NULL)
		ber_free(ber, false);
}

static int vxldap_getgrx(struct ldap_state *state, const char *filter,
    struct vxdb_group *dest)
{
	LDAPMessage *result, *entry;
	int ret;

	ret = ldap_search_ext_s(state->conn, state->group_suffix,
	      LDAP_SCOPE_SUBTREE, filter, NULL, false,
	      NULL, NULL, NULL, 1, &result);
	if (ret != LDAP_SUCCESS || result == NULL)
		return -(errno = vxldap_errno(ret));
	entry = ldap_first_entry(state->conn, result);
	if (entry == NULL) {
		ldap_msgfree(result);
		return -ENOENT;
	}
	if (dest != NULL)
		vxldap_copy_group(dest, state->conn, result);
	ldap_msgfree(result);
	return 1;
}

static int vxldap_getgrgid(struct vxdb_state *vp, unsigned int gid,
    struct vxdb_group *dest)
{
	char filter[48+HXSIZEOF_Z32];
	int ret;
	snprintf(filter, sizeof(filter),
	         "(&(" F_POSIXGROUP ")(gidNumber=%u))", gid);
	ret = vxldap_getgrx(vp->state, filter, dest);
	return (ret == -ENOENT) ? 0 : ret;
}

static int vxldap_getgrnam(struct vxdb_state *vp, const char *user,
    struct vxdb_group *dest)
{
	hxmc_t *filter;
	int ret;

	filter = HXmc_strinit("(&(" F_POSIXGROUP ")(cn=" /* )) */);
	HXmc_strcat(&filter, user);
	HXmc_strcat(&filter, /* (( */ "))");
	ret = vxldap_getgrx(vp->state, filter, dest);
	HXmc_free(filter);
	return (ret == -ENOENT) ? 0 : ret;
}

static void *vxldap_grouptrav_init(struct vxdb_state *vp)
{
	static const char *const attrs[] = {
		"cn", "gidNumber", "member", "memberUid", NULL,
	};
	struct ldap_state *state = vp->state;
	struct ldap_trav trav;
	int ret;

	ret = ldap_search_ext_s(state->conn, state->group_suffix,
	      LDAP_SCOPE_SUBTREE, F_POSIXGROUP, const_cast2(char **, attrs),
	      false, NULL, NULL, NULL, LDAP_MAXINT, &trav.base);
	if (ret != LDAP_SUCCESS) {
		vxldap_errno_sp(ret, "vxldap_grouptrav_init");
		return NULL;
	}

	trav.current = ldap_first_entry(state->conn, trav.base);
	return HX_memdup(&trav, sizeof(trav));
}

static int vxldap_grouptrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_group *dest)
{
	struct ldap_state *state = vp->state;
	struct ldap_trav *trav   = ptr;

	if (trav->current == NULL)
		return 0;

	vxldap_copy_group(dest, state->conn, trav->current);
	trav->current = ldap_next_entry(state->conn, trav->current);
	return 1;
}

static void vxldap_grouptrav_free(struct vxdb_state *vp, void *ptr)
{
	struct ldap_trav *trav = ptr;
	ldap_msgfree(trav->base);
	free(trav);
}

static int vxldap_sgmapadd(struct vxdb_state *vp, const char *user,
    const char *group)
{
	struct ldap_state *state = vp->state;
	hxmc_t *userdn = NULL, *groupdn = NULL;
	LDAPMod attr, *attr_ptrs[2];
	LDAPMessage *result;
	int ret, ldret;

	ret     = -ENOMEM;
	userdn  = dn_user(state, user);
	groupdn = dn_group(state, group);
	if (userdn == NULL || groupdn == NULL)
		goto out;

	/* Verify user exists */
	ret    = -ENOENT;
	ldret  = ldap_search_ext_s(state->conn, userdn, LDAP_SCOPE_BASE,
	         NULL, const_cast2(char **, no_attrs), true, NULL, NULL,
	         NULL, 1, &result);
	if (ldret == LDAP_NO_SUCH_OBJECT || result == NULL)
		goto out;

	attr = (LDAPMod){
		.mod_op     = LDAP_MOD_ADD,
		.mod_type   = "member",
		.mod_values = (char *[]){userdn, NULL},
	};
	attr_ptrs[0] = &attr;
	attr_ptrs[1] = NULL;

	ldret = ldap_modify_ext_s(state->conn, groupdn, attr_ptrs, NULL, NULL);
	if (ldret == LDAP_SUCCESS)
		ret = 1;
	else
		ret = vxldap_errno_sp(ret, "vxldap_sgmapadd");

 out:
	HXmc_free(userdn);
	HXmc_free(groupdn);
	return ret;
}

/**
 * vxldap_member_filter -
 * @state:	vxldap control structure
 * @user:	username
 *
 * Builds the LDAP filter for searching @user in all groups, taking care of
 * encoding @user as per RFC 4515.
 */
static hxmc_t *vxldap_member_filter(const struct ldap_state *state,
    const char *user)
{
	static const char *const hexmap = "0123456789ABCDEF";
	const char *s = state->user_suffix;
	char buf[64], *ptr;
	hxmc_t *filter;

	filter = HXmc_strinit("(&(" F_POSIXGROUP ")(member=uid\\3D"); /* )) */
	HXmc_strcat(&filter, user);
	HXmc_strcat(&filter, ",");

	while (*s != '\0') {
		for (ptr = buf; ptr < buf + sizeof(buf) - 1; ++s) {
			if (*s == '\0')
				break;
			if (*s == '=' || *s == '(' || *s == ')' || *s == '*') {
				if (ptr + 3 >= buf + sizeof(buf))
					break;
				*ptr++ = '\\';
				*ptr++ = hexmap[*s / 16];
				*ptr++ = hexmap[*s % 16];
				continue;
			}
			*ptr++ = *s;
		}
		*ptr = '\0';
		HXmc_strcat(&filter, buf);
	}

	HXmc_strcat(&filter, /* (( */ "))");
	return filter;
}

static void vxldap_sgmapget2(LDAP *conn, LDAPMessage *ldres,
    char **out, int entries)
{
	LDAPMessage *entry;
	char *attr, **val;
	BerElement *ber;

	for (entry = ldap_first_entry(conn, ldres);
	    entry != NULL && entries-- > 0;
	    entry = ldap_next_entry(conn, ldres))
	{
		attr = ldap_first_attribute(conn, entry, &ber);
		if (attr == NULL)
			/* should not happen, but whatever... */
			continue;
		val = ldap_get_values(conn, entry, attr);
		if (val != NULL) {
			if (*val != NULL)
				*out++ = HX_strdup(*val);
			ldap_value_free(val);
		}
		ldap_memfree(attr);
	}

	*out = NULL;
	ldap_msgfree(ldres);
}

static int vxldap_sgmapget(struct vxdb_state *vp, const char *user,
    char ***data)
{
	static const char *const attrs[] = {"cn", NULL};
	struct ldap_state *state = vp->state;
	LDAPMessage *result;
	hxmc_t *filter;
	int ret;

	filter = vxldap_member_filter(state, user);
	ret    = ldap_search_ext_s(state->conn, state->group_suffix,
	         LDAP_SCOPE_SUBTREE, filter, const_cast2(char **, attrs),
	         false, NULL, NULL, NULL, LDAP_MAXINT, &result);
	HXmc_free(filter);
	if (ret == LDAP_NO_SUCH_OBJECT)
		return -ENOENT;
	else if (ret != LDAP_SUCCESS)
		return vxldap_errno_sp(ret, "vxldap_sgmapget");

	ret = ldap_count_entries(state->conn, result);
	if (ret < 0) {
		ldap_perror(state->conn, "vxldap_sgmapget/ldap_count_entries");
		ldap_msgfree(result);
		return -(errno = 1600);
	}

	if (ret == 0 || data == NULL) {
		ldap_msgfree(result);
		return ret;
	}

	*data = malloc(sizeof(char *) * (ret + 1));
	if (*data == NULL) {
		ldap_msgfree(result);
		return -ENOMEM;
	}

	vxldap_sgmapget2(state->conn, result, *data, ret);
	return ret;
}

static int vxldap_sgmapdel(struct vxdb_state *vp, const char *user,
    const char *group)
{
	struct ldap_state *state = vp->state;
	LDAPMod attr, *attr_ptrs[2];
	hxmc_t *userdn, *groupdn;
	int ret;

	ret     = -ENOMEM;
	userdn  = dn_user(state, user);
	groupdn = dn_group(state, user);
	if (userdn == NULL || groupdn == NULL)
		goto out;

	attr = (LDAPMod){
		.mod_op     = LDAP_MOD_DELETE,
		.mod_type   = "member",
		.mod_values = (char *[]){userdn, NULL},
	};
	attr_ptrs[0] = &attr;
	attr_ptrs[1] = NULL;

	ret = ldap_modify_ext_s(state->conn, groupdn, attr_ptrs, NULL, NULL);
	if (ret == LDAP_NO_SUCH_OBJECT || ret == LDAP_NO_SUCH_ATTRIBUTE)
		return -ENOENT;
	else if (ret != LDAP_SUCCESS)
		return vxldap_errno_sp(ret, "vxldap_sgmapdel");
	else
		ret = 1;

 out:
	HXmc_free(userdn);
	HXmc_free(groupdn);
	return ret;
}

EXPORT_SYMBOL struct vxdb_driver THIS_MODULE = {
	.name           = "LDAP back-end module",
	.init           = vxldap_init,
	.open           = vxldap_open,
	.close          = vxldap_close,
	.exit           = vxldap_exit,
	.modctl         = vxldap_modctl,
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
	.sgmapadd       = vxldap_sgmapadd,
	.sgmapget       = vxldap_sgmapget,
	.sgmapdel       = vxldap_sgmapdel,
};
