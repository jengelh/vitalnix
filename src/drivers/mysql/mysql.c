/*
 *	mysql.c - MYSQL back-end
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2005 - 2009
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <mysql.h>
#include <mysqld_error.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

#define CHECK_ACCESS() \
	if ((state->names.sp_table != NULL && !state->perm_shadow) || \
	    (state->names.vs_table != NULL && !state->perm_vxshadow)) \
		return -EACCES;

/* Definitions */
enum {
	CONFIG_READ_BASE = 1,
	CONFIG_READ_NEST,
	CONFIG_FREE,
	MASK_DELETE = 1 << 0,
	MASK_SET    = 1 << 1,
};

struct mq_conn {
	MYSQL *handle;
	char *host, *socket, *user, *passwd, *database, *user2, *passwd2;
	unsigned int port;
};

struct mq_names {
	char *pw_table, *pw_name, *pw_uid, *pw_gid, *pw_real, *pw_home;
	char *pw_shell, *sp_table, *sp_user, *sp_passwd, *sp_lastchg, *sp_min;
	char *sp_max, *sp_warn, *sp_expire, *sp_inact, *gr_table, *gr_name;
	char *gr_gid, *gm_table, *gm_user, *gm_group, *vs_table, *vs_user;
	char *vs_uuid, *vs_pvgrp, *vs_defer;

	/* SELECT <csl> FROM <tsl> */
	char *csl_passwd, *csl_shadow, *csl_group, *csl_vxshadow;
	/* table selects */
	hxmc_t *tsl_user;
};

struct mysql_state {
	struct mq_conn  cn;
	struct mq_names names;
	bool perm_shadow, perm_vxshadow;
	/* Misc */
	unsigned int uid_min, uid_max, gid_min, gid_max;
};

struct traverser_state {
	MYSQL_RES *res;
	size_t count;
};

/* Functions */
static int vxmysql_xlock_user(struct mysql_state *);
static int vxmysql_xlock_group(struct mysql_state *);
static inline void vxmysql_xunlock(struct mysql_state *);
static int vxmysql_userdel_unlocked(struct mysql_state *, const char *);

static unsigned int count_rows(struct mysql_state *, const char *);
static void export_passwd(struct vxdb_user *, const MYSQL_ROW);
static void export_shadow(struct vxdb_user *, const MYSQL_ROW);
static inline void export_vxshadow(struct vxdb_user *, const MYSQL_ROW);
static inline void export_group(struct vxdb_group *, const MYSQL_ROW);
static unsigned int find_next_id(struct mysql_state *, unsigned int);
static inline long ito_expire(long);
static inline long ito_inact(long);
static int queryf(MYSQL *, const char *, ...);
static void read_config(struct mysql_state *, unsigned int, const char *);
static void read_config_cb(const struct HXoptcb *);
static char *s_join(const char *, ...);
static hxmc_t *sql_groupmask(hxmc_t **, const struct mysql_state *,
	const struct vxdb_group *, unsigned int);
static hxmc_t *sql_usermask(hxmc_t **, const struct mysql_state *,
	const struct vxdb_user *, unsigned int);

//-----------------------------------------------------------------------------
static int vxmysql_init(struct vxdb_state *vp, const char *config_file)
{
	struct mysql_state *st;
	struct mq_names *nm;

	st = vp->state = calloc(1, sizeof(struct mysql_state));
	if (st == NULL)
		return -errno;

	/* ->cn.handle is already freed on mysql_close() in vxmysql_close() */
	if ((st->cn.handle = mysql_init(NULL)) == NULL) {
		free(st);
		return -errno;
	}

	read_config(st, CONFIG_READ_BASE, config_file);

	/*
	 * Check that all config fields are present. Only if ->sp_table is
	 * given, ->sp_* must be there; only if ->vs_table is given, ->vs_*
	 * must be there.
	 */
	nm = &st->names;
	if (nm->pw_table   == NULL || nm->pw_name    == NULL   ||
	   nm->pw_uid     == NULL || nm->pw_gid     == NULL   ||
	   nm->pw_real    == NULL || nm->pw_home    == NULL   ||
	   nm->pw_shell   == NULL || nm->gr_table   == NULL   ||
	   nm->gr_name    == NULL || nm->gr_gid     == NULL   ||
	  (nm->sp_table   != NULL && (nm->sp_passwd == NULL   ||
	   nm->sp_lastchg == NULL || nm->sp_min     == NULL   ||
	   nm->sp_max     == NULL || nm->sp_warn    == NULL   ||
	   nm->sp_expire  == NULL || nm->sp_inact   == NULL)) ||
	  (nm->vs_table   != NULL && (nm->vs_user   == NULL   ||
	   nm->vs_uuid    == NULL || nm->vs_pvgrp   == NULL   ||
	   nm->vs_defer   == NULL)))
		return -1500;

	if (strncmp(st->cn.host, "unix:", 5) == 0) {
		st->cn.socket = HX_strdup(st->cn.host + 5);
		free(st->cn.host);
		st->cn.host = NULL;
	}

	nm->csl_passwd   = s_join(",", nm->pw_name, nm->pw_uid, nm->pw_gid,
	                   nm->pw_real, nm->pw_home, nm->pw_shell, NULL);
	nm->csl_shadow   = s_join(",", nm->sp_user, nm->sp_passwd,
	                   nm->sp_lastchg, nm->sp_min, nm->sp_max, nm->sp_warn,
	                   nm->sp_expire, nm->sp_inact, NULL);
	nm->csl_group    = s_join(",", nm->gr_name, nm->gr_gid, NULL);
	nm->csl_vxshadow = s_join(",", nm->vs_user, nm->vs_uuid, nm->vs_pvgrp,
	                   nm->vs_defer, NULL);

	return 1;
}

static void vxmysql_exit(struct vxdb_state *vp)
{
	struct mysql_state *state = vp->state;
	read_config(state, CONFIG_FREE, NULL);
	free(state);
}

static int vxmysql_open(struct vxdb_state *vp, unsigned int flags)
{
	struct mysql_state *state  = vp->state;
	const struct mq_conn *conn = &state->cn;
	struct mq_names *names     = &state->names;
	MYSQL_RES *result;
	void *ret = NULL;	/* only used for error checking, so no MYSQL *ret */

	if (conn->user2 != NULL && conn->passwd2 != NULL)
		ret = mysql_real_connect(conn->handle, conn->host,
		      conn->user2, conn->passwd2, conn->database,
		      conn->port, conn->socket, 0);

	if (ret == NULL && mysql_real_connect(conn->handle, conn->host,
	    conn->user, conn->passwd, conn->database, conn->port,
	    conn->socket, 0) == NULL)
		return -mysql_errno(conn->handle);

	if (names->sp_table != NULL) {
		state->perm_shadow = queryf(conn->handle,
		                     "select * from %s limit 0",
		                     names->sp_table) == 0;
		result = mysql_use_result(conn->handle);
		while (mysql_fetch_row(result) != NULL)
			/* noop */;
		mysql_free_result(result);
	}
	if (names->vs_table != NULL) {
		state->perm_vxshadow = queryf(conn->handle,
		                       "select * from %s limit 0",
		                       names->vs_table) == 0;
		result = mysql_use_result(conn->handle);
		while (mysql_fetch_row(result) != NULL)
			/* noop */;
		mysql_free_result(result);
	}

	/* FROM part for user UPDATE */
	{
		hxmc_t *s = HXmc_meminit(NULL, 256);
		HXmc_strcat(&s, names->pw_table);
		if (state->perm_shadow) {
			HXmc_strcat(&s, " left join ");
			HXmc_strcat(&s, names->sp_table);
			HXmc_strcat(&s, " on ");
			HXmc_strcat(&s, names->pw_name);
			HXmc_strcat(&s, "=");
			HXmc_strcat(&s, names->sp_user);
		}
		if (state->perm_vxshadow) {
			HXmc_strcat(&s, " left join ");
			HXmc_strcat(&s, names->vs_table);
			HXmc_strcat(&s, " on ");
			HXmc_strcat(&s, names->pw_name);
			HXmc_strcat(&s, "=");
			HXmc_strcat(&s, names->vs_user);
		}
		names->tsl_user = s;
	}

	return 1;
}

static void vxmysql_close(struct vxdb_state *vp)
{
	struct mysql_state *state = vp->state;
	mysql_close(state->cn.handle);
	HXmc_free(state->names.tsl_user);
}

static int vxmysql_xlock_user(struct mysql_state *state)
{
	const struct mq_names *names = &state->names;
	MYSQL *c = state->cn.handle;
	int ret = 1;

	if (!state->perm_vxshadow)
		ret = queryf(c, "lock tables %s write, %s write",
		      names->pw_table, names->sp_table);
	else
		ret = queryf(c, "lock tables %s write, %s write, %s write",
		      names->pw_table, names->sp_table, names->vs_table);

	if (ret != 0)
		ret = -mysql_errno(c);

	return ret;
}

static int vxmysql_xlock_group(struct mysql_state *state)
{
	return (queryf(state->cn.handle, "lock tables %s write",
	       state->names.gr_table) == 0) ? 1 :
	       -mysql_errno(state->cn.handle);
}

static inline void vxmysql_xunlock(struct mysql_state *state)
{
	mysql_query(state->cn.handle, "unlock tables");
	/* FIXME: does it require fetch_result? */
	MYSQL_RES *r = mysql_store_result(state->cn.handle);
	if (r != NULL)
		mysql_free_result(r);
}

static long vxmysql_modctl(struct vxdb_state *vp, unsigned int command, ...)
{
	struct mysql_state *state = vp->state;

	errno = 0;
	switch (command) {
		case VXDB_COUNT_USERS:
			return count_rows(state, state->names.pw_table);
		case VXDB_COUNT_GROUPS:
			return count_rows(state, state->names.gr_table);
		case VXDB_NEXTUID_SYS:
		case VXDB_NEXTUID:
		case VXDB_NEXTGID_SYS:
		case VXDB_NEXTGID:
			return find_next_id(state, command);
	}

	return -ENOSYS;
}

//-----------------------------------------------------------------------------
static int vxmysql_useradd(struct vxdb_state *vp,
    const struct vxdb_user *rq)
{
	struct mysql_state *state    = vp->state;
	const struct mq_names *names = &state->names;
	int ret = 0;

	if (rq->pw_name == NULL)
		return -EINVAL;
	CHECK_ACCESS();
	if ((ret = vxmysql_xlock_user(state)) <= 0)
		return ret;

	/* shadow part */
	if (names->sp_table != NULL) {
		ret = queryf(state->cn.handle, "insert into %s (%s) values "
		      "('%S','%S',%ld,%ld,%ld,%ld,%ld,%ld)",
		      names->sp_table, names->csl_shadow, rq->pw_name,
		      rq->sp_passwd, rq->sp_lastchg, rq->sp_min, rq->sp_max,
		      rq->sp_warn, rq->sp_expire, rq->sp_inact);
		if (ret != 0) {
			ret = -mysql_errno(state->cn.handle);
			if (ret == -ER_DUP_ENTRY)
				ret = -EEXIST;
			goto out;
		}
	}

	/* vxshadow part */
	if (names->vs_table != NULL) {
		ret = queryf(state->cn.handle, "insert into %s (%s) values "
		      "('%S','%S','%s',%u)", names->vs_table,
		      names->csl_vxshadow, rq->pw_name, rq->vs_uuid,
		      rq->vs_pvgrp, rq->vs_defer);
		if (ret != 0) {
			ret = -mysql_errno(state->cn.handle);
			if (ret == -ER_DUP_ENTRY)
				ret = -EEXIST;
			goto out;
		}
	}

	/*
	 * passwd part -- comes last, because this is THE entry which makes a
	 * user really "visible" in the system.
	 */
	ret = queryf(state->cn.handle,
	      "insert into %s (%s,%s,%s,%s,%s,%s) "
	      "values ('%S',%u,%u,'%s','%s','%s')",
	      names->pw_table, names->pw_name, names->pw_uid, names->pw_gid,
	      names->pw_real, names->pw_home, names->pw_shell, rq->pw_name,
	      rq->pw_uid, rq->pw_gid, rq->pw_real, rq->pw_home, rq->pw_shell);
	if (ret != 0) {
		if ((ret = -mysql_errno(state->cn.handle)) == -ER_DUP_ENTRY)
			ret = -EEXIST;
		goto out;
	}

	vxmysql_xunlock(state);
	return 1;

 out:
	vxmysql_userdel_unlocked(state, rq->pw_name);
	vxmysql_xunlock(state);
	return -ret;
}

static int vxmysql_usermod(struct vxdb_state *vp, const char *name,
    const struct vxdb_user *param)
{
	/*
	struct mysql_state *state = vp->state;
	where_usermask(state, sr_mask, mod2whmask(mod_mask));
	char query[4096], srk_mask[2048], mod_mask[2048];
	mask(srk, sp, srk_mask, sizeof(srk_mask));
	mask(mod, sp, mod_mask, sizeof(mod_mask));
	snprintf(query, sizeof(query), "update %s set %s where %s",
		TABLE_DESC, modmask2sql(), srmask2sql());
	*/
	return 0;
}

static int vxmysql_userdel(struct vxdb_state *vp, const char *name)
{
	struct mysql_state *state = vp->state;
	int ret;

	CHECK_ACCESS();
	if (name == NULL)
		return -EINVAL;
	if ((ret = vxmysql_xlock_user(state)) <= 0)
		return ret;
	ret = vxmysql_userdel_unlocked(state, name);
	vxmysql_xunlock(state);
	return ret;
}

static int vxmysql_userdel_unlocked(struct mysql_state *state, const char *name)
{
	/*
	hxmc_t *wtable_pw = where_pw_u(state, sr_mask, WHERE_PW_NAME | WHERE_PW_UID);
	char query[1024];
	int ret = 1;
	size_t n;

	n = snprintf(query, sizeof(query), "delete from %s where %s",
	             state->pw_table, where);
	if (n >= sizeof(query) - 1)
		goto toolong;
	if (mysql_query(state->cn.handle, query) != 0)
		ret = -mysql_errno(state->cn.handle);

	n = snprintf(query, sizeof(query), "delete from %s where %s",
	             state->sp_table, where);
	if (n >= sizeof(query) - 1)
		goto toolong;
	if (mysql_query(state->cn.handle, query) != 0 && ret == 0)
		ret = -mysql_errno(state->cn.handle);

	if (state->have_vxshadow) {
		snprintf(query, sizeof(query), "delete from %s where %s='%s'",
		         state->vs_table, where);
		if (n >= sizeof(query) - 1)
			goto toolong;
		if (mysql_query(state->cn.handle, query) != 0 && ret == 0)
			ret = -mysql_errno(state->cn.handle);
	}

	HXmc_free(where);
	return ret;

 toolong:
	HXmc_free(where);
	*/
	return -E2BIG;
}

static void *vxmysql_usertrav_init(struct vxdb_state *vp)
{
	struct mysql_state *state    = vp->state;
	const struct mq_names *names = &state->names;
	hxmc_t *query = HXmc_meminit(NULL, 1024);
	struct traverser_state trav;

	/* Build query */
	HXmc_strcpy(&query, "select ");
	HXmc_strcat(&query, names->csl_passwd);
	if (state->perm_shadow) {
		HXmc_strcat(&query, ",");
		HXmc_strcat(&query, names->csl_shadow);
	}
	if (state->perm_vxshadow) {
		HXmc_strcat(&query, ",");
		HXmc_strcat(&query, names->csl_vxshadow);
	}

	HXmc_strcat(&query, " from ");
	HXmc_strcat(&query, names->tsl_user);

	/* Do the query */
	if (mysql_query(state->cn.handle, query) != 0) {
		HXmc_free(query);
		errno = mysql_errno(state->cn.handle);
		return NULL;
	}

	HXmc_free(query);
	if ((trav.res = mysql_store_result(state->cn.handle)) == NULL) {
		errno = mysql_errno(state->cn.handle);
		return NULL;
	}

	trav.count = mysql_num_rows(trav.res);
	return HX_memdup(&trav, sizeof(struct traverser_state));
}

static int vxmysql_usertrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_user *res)
{
	struct mysql_state *state = vp->state;
	struct traverser_state *trav = ptr;
	size_t columns, colstart = 0;
	MYSQL_ROW row;
	int ret;

	/* -- part 0 -- */
	if ((row = mysql_fetch_row(trav->res)) == NULL)
		return 0;
	if ((ret = mysql_errno(state->cn.handle)) != 0)
		return -ret;
	columns = mysql_num_fields(trav->res);
	vxdb_user_clean(res);

	/* -- part 1 -- */
	if (columns < 6)
		fprintf(stderr, "%s: columns < 6, crash imminent\n", __func__);
	export_passwd(res, &row[colstart]);
	colstart += 6;
	columns  -= 6;

	/* -- part 2 -- */
	if (state->perm_shadow) {
		if (columns < 8)
			fprintf(stderr, "%s: columns < 8, crash imminent\n", __func__);
		export_shadow(res, &row[colstart]);
		colstart += 8;
		columns  -= 8;
	}

	/* -- part 3 -- */
	if (state->perm_vxshadow) {
		if (columns < 4)
			fprintf(stderr, "%s: columns < 4, crash immiment\n", __func__);
		export_vxshadow(res, &row[colstart]);
		colstart += 4;
		columns  -= 4;
	}

	if (columns != 0)
		fprintf(stderr, "Warning: %s: columns != 0\n", __func__);
	return 1;
}

static void vxmysql_usertrav_free(struct vxdb_state *vp, void *ptr)
{
	struct traverser_state *trav = ptr;
	mysql_free_result(trav->res);
	free(trav);
}

static int vxmysql_groupadd(struct vxdb_state *vp,
    const struct vxdb_group *rq)
{
	struct mysql_state *state = vp->state;
	int ret;

	CHECK_ACCESS();
	if ((ret = vxmysql_xlock_group(vp->state)) <= 0)
		return ret;
	return 0;
}

static int vxmysql_groupmod(struct vxdb_state *vp, const char *name,
    const struct vxdb_group *param)
{
	return 0;
}

static int vxmysql_groupdel(struct vxdb_state *vp, const char *name)
{
	/*
	struct mysql_state *state = vp->state;
	char query[1024], qtmp[512];
	hxmc_t *where;
	int ret;

	if (!state->have_shadow)
		return -EACCES;
	if (sr_mask->gr_name == NULL && sr_mask->gr_gid == VXDB_NOGID)
		return -EINVAL;
	if ((ret = vxmysql_xlock_group(state)) <= 0)
		return ret;

	where = where_groupmask(state, sr_mask, WHERE_GR_NAME | WHERE_GR_GID);
	snprintf(query, sizeof(query), "delete from %s where %s",
	         state->gr_table, where);
	if (mysql_query(state->cn.handle, query) != 0)
		ret = -mysql_errno(state->cn.handle);

	vxmysql_xunlock(state);
	HXmc_free(where);
	*/
	return 0; /* (ret == 0) ? 1 : ret; */
}

static void *vxmysql_grouptrav_init(struct vxdb_state *vp)
{
	struct mysql_state *state    = vp->state;
	const struct mq_names *names = &state->names;
	struct traverser_state trav;
	int ret;

	ret = queryf(state->cn.handle, "select %s from %s",
	             names->csl_group, names->gr_table);
	if (ret != 0 ||
	    (trav.res = mysql_store_result(state->cn.handle)) == NULL) {
		errno = mysql_errno(state->cn.handle);
		return NULL;
	}

	trav.count = mysql_num_rows(trav.res);
	return HX_memdup(&trav, sizeof(struct traverser_state));
}

static int vxmysql_grouptrav_walk(struct vxdb_state *vp, void *ptr,
    struct vxdb_group *res)
{
	struct mysql_state *st = vp->state;
	struct traverser_state *trav = ptr;
	size_t columns;
	MYSQL_ROW row;
	int ret;

	if ((row = mysql_fetch_row(trav->res)) == NULL)
		return 0;
	if ((ret = mysql_errno(st->cn.handle)) != 0)
		return -ret;
	if ((columns = mysql_num_fields(trav->res)) != 2)
		fprintf(stderr, "%s: columns != 2, crash imminent\n", __func__);

	export_group(res, row);
	return 1;
}

static void vxmysql_grouptrav_free(struct vxdb_state *vp, void *ptr)
{
	struct traverser_state *trav = ptr;
	mysql_free_result(trav->res);
	free(trav);
}

static unsigned int count_rows(struct mysql_state *state, const char *table)
{
	unsigned int ret = 0;
	char query[256];
	MYSQL_RES *res;
	MYSQL_ROW row;

	snprintf(query, sizeof(query), "select count(*) from %s", table);
	if (mysql_query(state->cn.handle, query) != 0 ||
	    (res = mysql_store_result(state->cn.handle)) == NULL)
		return -mysql_errno(state->cn.handle);

	if ((row = mysql_fetch_row(res)) == NULL || *row == NULL)
		goto out;

	ret = strtol(*row, NULL, 0);

 out:
	mysql_free_result(res);
	return ret;
}

static void export_passwd(struct vxdb_user *dest, const MYSQL_ROW in)
{
	HXmc_strcpy(&dest->pw_name, in[0]);
	if (in[1] != NULL)
		dest->pw_uid = strtoul(in[1], NULL, 0);
	if (in[2] != NULL)
		dest->pw_gid = strtoul(in[2], NULL, 0);
	HXmc_strcpy(&dest->pw_real, in[3]);
	HXmc_strcpy(&dest->pw_home, in[4]);
	HXmc_strcpy(&dest->pw_shell, in[5]);
}

static void export_shadow(struct vxdb_user *dest, const MYSQL_ROW in)
{
	/* in[0] is username, we do not to copy it again */
	HXmc_strcpy(&dest->sp_passwd, in[1]);
	if (in[2] != NULL)
		dest->sp_lastchg = strtol(in[2], NULL, 0);
	if (in[3] != NULL)
		dest->sp_min     = strtol(in[3], NULL, 0);
	if (in[4] != NULL)
		dest->sp_max     = strtol(in[4], NULL, 0);
	if (in[5] != NULL)
		dest->sp_warn    = strtol(in[5], NULL, 0);
	if (in[6] != NULL)
		dest->sp_expire  = ito_expire(strtol(in[6], NULL, 0));
	if (in[7] != NULL)
		dest->sp_inact   = ito_inact(strtol(in[7], NULL, 0));
}

static inline void export_vxshadow(struct vxdb_user *dest,
    const MYSQL_ROW in)
{
	HXmc_strcpy(&dest->vs_uuid, in[1]);
	HXmc_strcpy(&dest->vs_pvgrp, in[2]);
	if (in[3] != NULL)
		dest->vs_defer = strtoul(in[3], NULL, 0);
}

static inline void export_group(struct vxdb_group *dest, const MYSQL_ROW in)
{
	HXmc_strcpy(&dest->gr_name, in[0]);
	if (in[1] != NULL)
		dest->gr_gid = strtol(in[1], NULL, 0);
}

static unsigned int find_next_id(struct mysql_state *state,
    unsigned int command)
{
	struct mq_names *names = &state->names;
	const char *field, *table;
	unsigned int start, end;
	char query[256];

	if (command == VXDB_NEXTGID_SYS || command == VXDB_NEXTGID) {
		field = names->gr_gid;
		table = names->gr_table;
		start = state->gid_min;
		end   = state->gid_max;
	} else {
		field = names->pw_uid;
		table = names->pw_table;
		start = state->uid_min;
		end   = state->uid_max;
	}
	if (command == VXDB_NEXTGID_SYS || command == VXDB_NEXTUID_SYS) {
		end   = start - 1;
		start = 1;
	}

	snprintf(query, sizeof(query), "select %s from %s where %s=%u",
	         field, table, field, start);

/*
	if (empty)
		return start;
*/

/*
mysql> select min(uid)+1 as i from users as u1 where not exists
(select uid from users as u2 where u2.uid=u1.uid+1) and
uid between 1500 and 1509;
*/

	snprintf(query, sizeof(query), "select min(uq0.uid+1) from %s as uq0 "
	  "where not exists (select %s from %s as uq1 where "
	  "uq1.uid=uq0.uid+1) and uq0.uid >= %u and uq0.uid <= %u",
	  table, field, table, start, end);

	return 0;
}

static inline long ito_expire(long d)
{
	return (d == -1) ? VXDB_NO_EXPIRE : d;
}

static inline long ito_inact(long d)
{
	return (d == -1) ? VXDB_NO_INACTIVE : d;
}

static int queryf(MYSQL *handle, const char *fmt, ...)
{
	const char *last_ptr, *next_ptr;
	va_list argp;
	hxmc_t *query;
	char *quote;
	int ret;

	query = HXmc_meminit(NULL, 256);
	va_start(argp, fmt);
	last_ptr = fmt;

	while ((next_ptr = strchr(last_ptr, '%')) != NULL) {
		HXmc_memcat(&query, last_ptr, next_ptr - last_ptr);
		if (strncmp(next_ptr, "%s", 2) == 0) {
			HXmc_strcat(&query, va_arg(argp, const char *));
			last_ptr += 2;
		} else if (strncmp(next_ptr, "%S", 2) == 0) {
			vxutil_quote(va_arg(argp, const char *),
			             VXQUOTE_SINGLE, &quote);
			HXmc_strcat(&query, "'");
			HXmc_strcat(&query, quote);
			HXmc_strcat(&query, "'");
			last_ptr += 2;
		} else if (strncmp(next_ptr, "%ld", 3) == 0) {
			char buf[HXSIZEOF_Z64];
			snprintf(buf, sizeof(buf), "%ld", va_arg(argp, long));
			HXmc_strcat(&query, buf);
			last_ptr += 3;
		} else if (strncmp(next_ptr, "%lu", 3) == 0) {
			char buf[HXSIZEOF_Z64];
			snprintf(buf, sizeof(buf), "%lu", va_arg(argp, unsigned long));
			HXmc_strcat(&query, buf);
			last_ptr += 3;
		} else if (strncmp(next_ptr, "%d", 2) == 0) {
			char buf[HXSIZEOF_Z32];
			snprintf(buf, sizeof(buf), "%d", va_arg(argp, int));
			HXmc_strcat(&query, buf);
			last_ptr += 3;
		} else if (strncmp(next_ptr, "%u", 2) == 0) {
			char buf[HXSIZEOF_Z32];
			snprintf(buf, sizeof(buf), "%d", va_arg(argp, unsigned int));
			HXmc_strcat(&query, buf);
			last_ptr += 3;
		} else {
			last_ptr = next_ptr;
		}
	}

	HXmc_memcat(&query, last_ptr, strlen(last_ptr));
	ret = mysql_query(handle, query);
	HXmc_free(query);
	free(quote);
	va_end(argp);
	return ret;
}

static void quote_append(hxmc_t **dest, const char *s)
{
	size_t orig_len;
	char *ret, *tmp, *fm;

	orig_len = strlen(s) * 2 + 1;
	tmp      = malloc(orig_len);
	if (tmp == NULL) {
		perror("malloc in quote_append in mysql.c");
		abort(); /* FIXME: ugly */
	}
	if ((ret = vxutil_quote(s, 0, &fm)) != NULL) {
		HXmc_strcat(dest, "='");
		HXmc_strcat(dest, ret);
		HXmc_strcat(dest, "'");
	}
	free(fm);
}

static void read_config(struct mysql_state *state, unsigned int action,
    const char *file)
{
	struct mq_names *names = &state->names;
	struct HXoption options_table[] = {
		{.ln = "SOURCE_PRI",            .type = HXTYPE_STRING, .cb = read_config_cb, .uptr = state},
		{.ln = "SOURCE_SEC",            .type = HXTYPE_STRING, .cb = read_config_cb, .uptr = state},
		{.ln = "users.host",            .type = HXTYPE_STRING, .ptr = &state->cn.host},
		{.ln = "users.database",        .type = HXTYPE_STRING, .ptr = &state->cn.database},
		{.ln = "users.db_user",         .type = HXTYPE_STRING, .ptr = &state->cn.user},
		{.ln = "users.db_password",     .type = HXTYPE_STRING, .ptr = &state->cn.passwd},
		{.ln = "users.table",           .type = HXTYPE_STRING, .ptr = &names->pw_table},
		//{.ln = "users.where_clause",    .type = HXTYPE_STRING, .ptr = NULL},
		{.ln = "users.user_column",     .type = HXTYPE_STRING, .ptr = &names->pw_name},
		//{.ln = "users.password_column", .type = HXTYPE_STRING, .ptr = NULL},
		//{.ln = "users.userid_column",   .type = HXTYPE_STRING, .ptr = NULL},
		{.ln = "users.uid_column",      .type = HXTYPE_STRING, .ptr = &names->pw_uid},
		{.ln = "users.gid_column",      .type = HXTYPE_STRING, .ptr = &names->pw_gid},
		{.ln = "users.realname_column", .type = HXTYPE_STRING, .ptr = &names->pw_real},
		{.ln = "users.homedir_column",  .type = HXTYPE_STRING, .ptr = &names->pw_home},
		{.ln = "users.shell_column",    .type = HXTYPE_STRING, .ptr = &names->pw_shell},

		//{.ln = "shadow.host",              .type = HXTYPE_STRING, .ptr = NULL},
		{.ln = "shadow.db_user",           .type = HXTYPE_STRING, .ptr = &state->cn.user2},
		{.ln = "shadow.db_password",       .type = HXTYPE_STRING, .ptr = &state->cn.passwd2},
		{.ln = "shadow.table",             .type = HXTYPE_STRING, .ptr = &names->sp_table},
		//{.ln = "shadow.userid_column",     .type = HXTYPE_STRING, .ptr = NULL},
		//{.ln = "shadow.user_column",       .type = HXTYPE_STRING, .ptr = NULL},
		{.ln = "shadow.password_column",   .type = HXTYPE_STRING, .ptr = &names->sp_passwd},
		{.ln = "shadow.lastchange_column", .type = HXTYPE_STRING, .ptr = &names->sp_lastchg},
		{.ln = "shadow.min_column",        .type = HXTYPE_STRING, .ptr = &names->sp_min},
		{.ln = "shadow.max_column",        .type = HXTYPE_STRING, .ptr = &names->sp_max},
		{.ln = "shadow.warn_column",       .type = HXTYPE_STRING, .ptr = &names->sp_warn},
		{.ln = "shadow.expire_column",     .type = HXTYPE_STRING, .ptr = &names->sp_expire},
		{.ln = "shadow.inact_column",      .type = HXTYPE_STRING, .ptr = &names->sp_inact},

		{.ln = "groups.group_info_table",  .type = HXTYPE_STRING, .ptr = &names->gr_table},
		//{.ln = "groups.where_clause",      .type = HXTYPE_STRING, .ptr = NULL},
		{.ln = "groups.group_name_column", .type = HXTYPE_STRING, .ptr = &names->gr_name},
		//{.ln = "groups.groupid_column",    .type = HXTYPE_STRING, .ptr = NULL},
		{.ln = "groups.gid_column",        .type = HXTYPE_STRING, .ptr = &names->gr_gid},
		//{.ln = "groups.password_column",   .type = HXTYPE_STRING, .ptr = NULL}, // no SGRP

		//{.ln = "groups.members_table",     .type = HXTYPE_STRING, .ptr = &names->gm_table}, // no SGRP
		//{.ln = "groups.member_uid_column", .type = HXTYPE_STRING, .ptr = &names->gm_user},
		//{.ln = "groups.member_gid_column", .type = HXTYPE_STRING, .ptr = &names->gm_group},

		{.ln = "vxshadow.table_name", .type = HXTYPE_STRING, .ptr = &names->vs_table},
		{.ln = "vxshadow.user_col",   .type = HXTYPE_STRING, .ptr = &names->vs_user},
		{.ln = "vxshadow.uuid_col",   .type = HXTYPE_STRING, .ptr = &names->vs_uuid},
		{.ln = "vxshadow.pvgrp_col",  .type = HXTYPE_STRING, .ptr = &names->vs_pvgrp},
		{.ln = "vxshadow.defer_col",  .type = HXTYPE_STRING, .ptr = &names->vs_defer},
		HXOPT_TABLEEND,
	};

	if (action == CONFIG_READ_BASE) {
		struct HXoption autouid_table[] = {
			{.ln = "UID_MIN", .type = HXTYPE_UINT, .ptr = &state->uid_min},
			{.ln = "UID_MAX", .type = HXTYPE_UINT, .ptr = &state->uid_max},
			{.ln = "GID_MIN", .type = HXTYPE_UINT, .ptr = &state->gid_min},
			{.ln = "GID_MAX", .type = HXTYPE_UINT, .ptr = &state->gid_max},
			HXOPT_TABLEEND,
		};
		state->uid_min = state->gid_min = 1000;
		state->uid_max = state->gid_max = 60000;
		HX_shconfig(CONFIG_SYSCONFDIR "/autouid.conf", autouid_table);
		HX_shconfig(file, options_table);
	} else if (action == CONFIG_READ_NEST) {
		HX_shconfig(file, options_table);
	} else if (action == CONFIG_FREE) {
		HX_shconfig_free(options_table);
	}
}

static void read_config_cb(const struct HXoptcb *cbi)
{
	read_config(cbi->current->uptr, CONFIG_READ_NEST, cbi->data);
}

static char *s_join(const char *delim, ...)
{
	size_t max = 0, dl = 0;
	int not_first = 0;
	va_list vp, wp;
	const char *s;
	char *ret;

	if (delim != NULL)
		dl = strlen(delim);

	va_start(vp, delim);
	va_copy(wp, vp);

	/* Find length */
	while ((s = va_arg(wp, const char *)) != NULL)
		max += strlen(s) + dl;
	va_end(wp);

	/* Allocate */
	if ((ret = malloc(max)) == NULL)
		return NULL;
	*ret = '\0';

	/* Compose string */
	while ((s = va_arg(vp, const char *)) != NULL) {
		if (not_first++ && dl > 0)
			strcat(ret, delim);
		strcat(ret, s);
	}
	va_end(vp);
	return ret;
}

#define PUT_I(SF, MF) \
	do { \
		if (n) HXmc_strcat(s, " and "); \
		HXmc_strcat(s, (SF)); \
		snprintf(tmp, sizeof(tmp), "=%u", (MF)); \
		HXmc_strcat(s, tmp); \
		n = 1; \
	} while (0)
#define PUT_S(SF, MF) \
	do { \
		if (n) HXmc_strcat(s, " and "); \
		HXmc_strcat(s, (SF)); \
		quote_append(s, (MF)); \
		n = 1; \
	} while (0)
static hxmc_t *sql_groupmask(hxmc_t **s, const struct mysql_state *state,
    const struct vxdb_group *mask, unsigned int flags)
{
	const struct mq_names *names = &state->names;
	char tmp[HXSIZEOF_Z32];
	int n = 0;

	if (*s == NULL)
		fprintf(stderr, "Error: %s called with *s == NULL\n", __func__);
	if (mask->gr_name != NULL)
		PUT_S(names->gr_name, mask->gr_name);
	if (mask->gr_gid != VXDB_NOGID)
		PUT_I(names->gr_gid, mask->gr_gid);
	return *s;
}

/*
flags can contain:
	MASK_DELETE = only put username and uid into sql query
	MASK_SET    = add extra fields to set username in ->sp_table and ->vs_table
*/

static hxmc_t *sql_usermask(hxmc_t **s, const struct mysql_state *state,
    const struct vxdb_user *mask, unsigned int flags)
{
	const struct mq_names *names = &state->names;
	char tmp[HXSIZEOF_Z32];
	int n = 0;

	if (*s == NULL)
		fprintf(stderr, "Error: %s called with *s == NULL\n", __func__);

	if (mask->pw_name != NULL) {
		PUT_S(names->pw_name, mask->pw_name);
		if ((flags & MASK_SET) && state->perm_shadow)
			PUT_S(names->sp_user, mask->pw_name);
		if ((flags & MASK_SET) && state->perm_vxshadow)
			PUT_S(names->vs_user, mask->pw_name);
	}
	if (mask->pw_uid != VXDB_NOUID)
		PUT_I(names->pw_uid, mask->pw_uid);
	if (flags & MASK_DELETE)
		return *s;
	if (mask->pw_gid   != VXDB_NOGID)
		PUT_I(names->pw_gid,   mask->pw_gid);
	if (mask->pw_real  != NULL)
		PUT_S(names->pw_real,  mask->pw_real);
	if (mask->pw_home  != NULL)
		PUT_S(names->pw_home,  mask->pw_home);
	if (mask->pw_shell != NULL)
		PUT_S(names->pw_shell, mask->pw_shell);
	return *s;
}
#undef PUT_I
#undef PUT_S

EXPORT_SYMBOL struct vxdb_driver THIS_MODULE = {
	.name           = "MYSQL back-end module",
	.init           = vxmysql_init,
	.open           = vxmysql_open,
	.close          = vxmysql_close,
	.exit           = vxmysql_exit,
	.modctl         = vxmysql_modctl,
	.useradd        = vxmysql_useradd,
	.usermod        = vxmysql_usermod,
	.userdel        = vxmysql_userdel,
	.usertrav_init  = vxmysql_usertrav_init,
	.usertrav_walk  = vxmysql_usertrav_walk,
	.usertrav_free  = vxmysql_usertrav_free,
	.groupadd       = vxmysql_groupadd,
	.groupmod       = vxmysql_groupmod,
	.groupdel       = vxmysql_groupdel,
	.grouptrav_init = vxmysql_grouptrav_init,
	.grouptrav_walk = vxmysql_grouptrav_walk,
	.grouptrav_free = vxmysql_grouptrav_free,
};
