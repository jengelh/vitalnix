/*=============================================================================
Vitalnix User Management Suite
drivers/mysql/mysql.c - MYSQL back-end module
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
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <mysql.h>
#include <mysqld_error.h>
#include "vitalnix-config.h"
#include "drivers/proto.h"
#include "drivers/static-build.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxutil/libvxutil.h"

#define CHECK_ACCESS() \
    if((state->sp_table != NULL && !state->perm_shadow) || \
     (state->vs_table != NULL && !state->perm_vxshadow)) \
            return -EACCES;
#define Z_32 sizeof("4294967296")

// Definitions
enum {
    CONFIG_READ_BASE = 1,
    CONFIG_READ_NEST,
    CONFIG_FREE,
    MASK_DELETE = 1 << 0,
    MASK_SET    = 1 << 1,
};

struct mysql_state {
    struct mq_conn {
        MYSQL *handle;
        char *host, *socket, *user, *passwd, *database, *user2, *passwd2;
        unsigned int port;
    } cn;

    // SQL fields
    char *pw_table, *pw_name, *pw_uid, *pw_gid, *pw_real, *pw_home, *pw_shell,
         *sp_table, *sp_user, *sp_passwd, *sp_lastchg, *sp_min, *sp_max,
         *sp_warn, *sp_expire, *sp_inact,
         *gr_table, *gr_name, *gr_gid,
         *gm_table, *gm_user, *gm_group,
         *vs_table, *vs_user, *vs_uuid, *vs_pvgrp, *vs_defer;
    // column selects (SELECT <csl> FROM <tsl>)
    char *csl_passwd, *csl_shadow, *csl_group, *csl_vxshadow;
    // table selects
    hmc_t *tsl_user;
    int perm_shadow, perm_vxshadow;

    // Misc
    long uid_min, uid_max, gid_min, gid_max;
};

struct traverser_state {
    MYSQL_RES *res;
    size_t count;
};

// Functions
DRIVER_PROTO_BASE1(vmysql);
DRIVER_PROTO_USER(vmysql);
DRIVER_PROTO_GROUP(vmysql);
static driver_modctl_t vmysql_modctl;
static int vmysql_xlock_user(struct mysql_state *);
static int vmysql_xlock_group(struct mysql_state *);
static inline void vmysql_xunlock(struct mysql_state *);
static int vmysql_userdel_unlocked(struct mysql_state *,
    const struct vxpdb_user *);

static long count_rows(struct mysql_state *, const char *);
static void export_passwd(struct vxpdb_user *, const MYSQL_ROW);
static void export_shadow(struct vxpdb_user *, const MYSQL_ROW);
static inline void export_vxshadow(struct vxpdb_user *, const MYSQL_ROW);
static inline void export_group(struct vxpdb_group *, const MYSQL_ROW);
static long find_next_id(struct mysql_state *, long);
static inline long ito_expire(long);
static inline long ito_inact(long);
static size_t nd_row_strlen(MYSQL_RES *, MYSQL_ROW, size_t);
static void read_config(struct mysql_state *, unsigned int, const char *);
static void read_config_cb(const struct HXoptcb *);
static char *s_join(const char *, ...);
static hmc_t *sql_groupmask(hmc_t **, const struct mysql_state *,
    const struct vxpdb_group *, unsigned int);
static hmc_t *sql_usermask(hmc_t **, const struct mysql_state *,
    const struct vxpdb_user *, unsigned int);

// Variables
static struct vxpdb_mvtable THIS_MODULE = {
    .name           = "MYSQL back-end module",
    .author         = "Alphagate Systems <support [at] ahn hopto org>, "
                      "2005 - 2006",
    DRIVER_CB_BASE1(vmysql),
    .modctl         = vmysql_modctl,
    DRIVER_CB_USER(vmysql),
    DRIVER_CB_GROUP(vmysql),
};

//-----------------------------------------------------------------------------
REGISTER_MODULE(mysql, &THIS_MODULE);

static int vmysql_init(struct vxpdb_state *vp, const char *config_file) {
    struct mysql_state *st;

    if((st = vp->state = calloc(1, sizeof(struct mysql_state))) == NULL)
        return -errno;

    // ->cn.handle is already freed on mysql_close() in vmysql_close()
    if((st->cn.handle = mysql_init(NULL)) == NULL) {
        free(st);
        return -errno;
    }

    read_config(st, CONFIG_READ_BASE, config_file);

    /* Check that all config fields are present. Only if ->sp_table is given,
    ->sp_* must be there; only if ->vs_table is given, ->vs_* must be there. */
    if(st->pw_table   == NULL || st->pw_name    == NULL   ||
       st->pw_uid     == NULL || st->pw_gid     == NULL   ||
       st->pw_real    == NULL || st->pw_home    == NULL   ||
       st->pw_shell   == NULL || st->gr_table   == NULL   ||
       st->gr_name    == NULL || st->gr_gid     == NULL   ||
      (st->sp_table   != NULL && (st->sp_passwd == NULL   ||
       st->sp_lastchg == NULL || st->sp_min     == NULL   ||
       st->sp_max     == NULL || st->sp_warn    == NULL   ||
       st->sp_expire  == NULL || st->sp_inact   == NULL)) ||
      (st->vs_table   != NULL && (st->vs_user   == NULL   ||
       st->vs_uuid    == NULL || st->vs_pvgrp   == NULL   ||
       st->vs_defer   == NULL)))
            return -1500;

    if(strncmp(st->cn.host, "unix:", 5) == 0) {
        st->cn.socket = HX_strdup(st->cn.host + 5);
        free(st->cn.host);
        st->cn.host = NULL;
    }

    st->csl_passwd   = s_join(",", st->pw_name, st->pw_uid, st->pw_gid,
                       st->pw_real, st->pw_home, st->pw_shell, NULL);
    st->csl_shadow   = s_join(",", st->sp_user, st->sp_passwd, st->sp_lastchg,
                       st->sp_min, st->sp_max, st->sp_warn, st->sp_expire,
                       st->sp_inact, NULL);
    st->csl_group    = s_join(",", st->gr_name, st->gr_gid, NULL);
    st->csl_vxshadow = s_join(",", st->vs_user, st->vs_uuid, st->vs_pvgrp,
                       st->vs_defer, NULL);

    return 1;
}

static void vmysql_deinit(struct vxpdb_state *vp) {       
    struct mysql_state *state = vp->state;
    read_config(state, CONFIG_FREE, NULL);
    free(state);
    return;
}

static int vmysql_open(struct vxpdb_state *vp, long flags) {
    struct mysql_state *state = vp->state;
    const struct mq_conn *cn  = &state->cn;
    MYSQL_RES *result;
    void *ret = NULL;   // only used for error checking, so no MYSQL *ret
    char buf[128];

    if(cn->user2 != NULL && cn->passwd2 != NULL)
        ret = mysql_real_connect(cn->handle, cn->host,
              cn->user2, cn->passwd2, cn->database,
              cn->port, cn->socket, 0);

    if(ret == NULL && mysql_real_connect(cn->handle, cn->host, cn->user,
     cn->passwd, cn->database, cn->port, cn->socket, 0) == NULL)
            return -mysql_errno(cn->handle);

    if(state->sp_table != NULL) {
        snprintf(buf, sizeof(buf), "select * from %s limit 0", state->sp_table);
        state->perm_shadow = mysql_query(cn->handle, buf) == 0;
        result = mysql_use_result(cn->handle);
        while(mysql_fetch_row(result) != NULL);
        mysql_free_result(result);
    }
    if(state->vs_table != NULL) {
        snprintf(buf, sizeof(buf), "select * from %s limit 0", state->vs_table);
        state->perm_vxshadow = mysql_query(cn->handle, buf) == 0;
        result = mysql_use_result(cn->handle);
        while(mysql_fetch_row(result) != NULL);
        mysql_free_result(result);
    }

    /* FROM part for user UPDATE */
    {
        hmc_t *s = hmc_minit(NULL, 256);
        hmc_strcat(&s, state->pw_table);
        if(state->perm_shadow) {
            hmc_strcat(&s, " left join ");
            hmc_strcat(&s, state->sp_table);
            hmc_strcat(&s, " on ");
            hmc_strcat(&s, state->pw_name);
            hmc_strcat(&s, "=");
            hmc_strcat(&s, state->sp_user);
        }
        if(state->perm_vxshadow) {
            hmc_strcat(&s, " left join ");
            hmc_strcat(&s, state->vs_table);
            hmc_strcat(&s, " on ");
            hmc_strcat(&s, state->pw_name);
            hmc_strcat(&s, "=");
            hmc_strcat(&s, state->vs_user);
        }
        state->tsl_user = s;
    }

    return 1;
}

static void vmysql_close(struct vxpdb_state *vp) {
    struct mysql_state *state = vp->state;
    mysql_close(state->cn.handle);
    hmc_free(state->tsl_user);
    return;
}

static int vmysql_xlock_user(struct mysql_state *state) {
    char query[128];
    int ret = 1;

    if(!state->perm_vxshadow)
        snprintf(query, sizeof(query), "lock tables %s write, %s write",
                 state->pw_table, state->sp_table);
    else
        snprintf(query, sizeof(query), "lock tables %s write, %s write, %s "
                 "write", state->pw_table, state->sp_table, state->vs_table);

    if(mysql_query(state->cn.handle, query) != 0)
        ret = -mysql_errno(state->cn.handle);

    return ret;
}

static int vmysql_xlock_group(struct mysql_state *state) {
    char query[64];
    snprintf(query, sizeof(query), "lock tables %s write", state->gr_table);
    if(mysql_query(state->cn.handle, query) != 0)
        return -mysql_errno(state->cn.handle);
    return 1;
}

static inline void vmysql_xunlock(struct mysql_state *state) {
    mysql_query(state->cn.handle, "unlock tables");
    // FIXME: does it require fetch_result?
    return;
}

static long vmysql_modctl(struct vxpdb_state *vp, long command, ...) {
    struct mysql_state *state = vp->state;
    errno = 0;

    switch(command) {
        case PDB_COUNT_USERS:
            return count_rows(state, state->pw_table);
        case PDB_COUNT_GROUPS:
            return count_rows(state, state->gr_table);
        case PDB_NEXTUID_SYS:
        case PDB_NEXTUID:
        case PDB_NEXTGID_SYS:
        case PDB_NEXTGID:
            return find_next_id(state, command);
    }

    return -ENOSYS;
}


//-----------------------------------------------------------------------------
static int vmysql_useradd(struct vxpdb_state *vp,
  const struct vxpdb_user *rq)
{
    struct mysql_state *state = vp->state;
    char query[1024], *esc_name, *free_me[4];
    int ret = 0;

    if(rq->pw_name == NULL)
        return -EINVAL;
    CHECK_ACCESS();
    if((ret = vmysql_xlock_user(state)) <= 0)
        return ret;

    esc_name = vxutil_quote(rq->pw_name, 0, &free_me[0]);

    // shadow part
    if(state->sp_table != NULL) {
        snprintf(query, sizeof(query),
            "insert into %s (%s) values ('%s','%s',%ld,%ld,%ld,%ld,%ld,%ld)",
            state->sp_table, state->csl_shadow, esc_name,
            vxutil_quote(rq->sp_passwd, 0, &free_me[1]), rq->sp_lastchg,
            rq->sp_min, rq->sp_max, rq->sp_warn, rq->sp_expire, rq->sp_inact);
        free(free_me[1]);
        if(mysql_query(state->cn.handle, query) != 0) {
            if((ret = -mysql_errno(state->cn.handle)) == -ER_DUP_ENTRY)
                ret = -EEXIST;
            goto out;
        }
    }

    // vxshadow part
    if(state->vs_table != NULL) {
        snprintf(query, sizeof(query),
            "insert into %s (%s) values ('%s','%s','%s',%ld)",
            state->vs_table, state->csl_vxshadow, esc_name,
            vxutil_quote(rq->vs_uuid, 0, &free_me[1]),
            vxutil_quote(rq->vs_pvgrp, 0, &free_me[2]),
            rq->vs_defer
        );
        free(free_me[1]);
        free(free_me[2]);
        if(mysql_query(state->cn.handle, query) != 0) {
            if((ret = -mysql_errno(state->cn.handle)) == -ER_DUP_ENTRY)
                ret = -EEXIST;
            goto out;
        }
    }

    /* passwd part -- comes last, because this is THE entry which makes a user
    really "visible" in the system. */
    snprintf(query, sizeof(query),
        "insert into %s (%s,%s,%s,%s,%s,%s) "
        "values ('%s','%ld','%ld','%s','%s','%s')",
        state->pw_table, state->pw_name, state->pw_uid, state->pw_gid,
        state->pw_real, state->pw_home, state->pw_shell, esc_name, rq->pw_uid,
        rq->pw_gid, vxutil_quote(rq->pw_real, 0, &free_me[1]),
        vxutil_quote(rq->pw_home, 0, &free_me[2]),
        vxutil_quote(rq->pw_shell, 0, &free_me[3])
    );
    free(free_me[0]);
    free(free_me[1]);
    free(free_me[2]);
    free(free_me[3]);
    if(mysql_query(state->cn.handle, query) != 0) {
        if((ret = -mysql_errno(state->cn.handle)) == -ER_DUP_ENTRY)
            ret = -EEXIST;
        goto out;
    }

    vmysql_xunlock(state);
    return 1;

 out:
    vmysql_userdel_unlocked(state, rq);
    vmysql_xunlock(state);
    return -ret;
}

static int vmysql_usermod(struct vxpdb_state *vp,
  const struct vxpdb_user *sr_mask, const struct vxpdb_user *mod_mask)
{
/*    struct mysql_state *state = vp->state;
    where_usermask(state, sr_mask, mod2whmask(mod_mask));
    char query[4096], srk_mask[2048], mod_mask[2048];
    mask(srk, sp, srk_mask, sizeof(srk_mask));
    mask(mod, sp, mod_mask, sizeof(mod_mask));
    snprintf(query, sizeof(query), "update %s set %s where %s",
        TABLE_DESC, modmask2sql(), srmask2sql()); */
    return 0;
}

static int vmysql_userdel(struct vxpdb_state *vp,
  const struct vxpdb_user *sr_mask)
{
    struct mysql_state *state = vp->state;
    int ret;

    CHECK_ACCESS();
    if(sr_mask->pw_name == NULL && sr_mask->pw_uid == PDB_NOUID)
        return -EINVAL;
    if((ret = vmysql_xlock_user(state)) <= 0)
        return ret;
    ret = vmysql_userdel_unlocked(state, sr_mask);
    vmysql_xunlock(state);
    return ret;
}

static int vmysql_userdel_unlocked(struct mysql_state *state,
  const struct vxpdb_user *sr_mask)
{
/*    hmc_t *wtable_pw = where_pw_u(state, sr_mask, WHERE_PW_NAME | WHERE_PW_UID);
    char query[1024];
    int ret = 1;
    size_t n;

    n = snprintf(query, sizeof(query), "delete from %s where %s",
                 state->pw_table, where);
    if(n >= sizeof(query) - 1)
        goto toolong;
    if(mysql_query(state->cn.handle, query) != 0)
        ret = -mysql_errno(state->cn.handle);

    n = snprintf(query, sizeof(query), "delete from %s where %s",
                 state->sp_table, where);
    if(n >= sizeof(query) - 1)
        goto toolong;
    if(mysql_query(state->cn.handle, query) != 0 && ret == 0)
        ret = -mysql_errno(state->cn.handle);

    if(state->have_vxshadow) {
        snprintf(query, sizeof(query), "delete from %s where %s='%s'",
                 state->vs_table, where);
        if(n >= sizeof(query) - 1)
            goto toolong;
        if(mysql_query(state->cn.handle, query) != 0 && ret == 0)
            ret = -mysql_errno(state->cn.handle);
    }

    hmc_free(where);
    return ret;

 toolong:
    hmc_free(where);*/
    return -E2BIG;
}

static void *vmysql_usertrav_init(struct vxpdb_state *vp) {
    struct mysql_state *state = vp->state;
    hmc_t *query = hmc_minit(NULL, 1024);
    struct traverser_state trav;

    // Build query
    hmc_strasg(&query, "select ");
    hmc_strcat(&query, state->csl_passwd);
    if(state->perm_shadow) {
        hmc_strcat(&query, ",");
        hmc_strcat(&query, state->csl_shadow);
    }
    if(state->perm_vxshadow) {
        hmc_strcat(&query, ",");
        hmc_strcat(&query, state->csl_vxshadow);
    }

    hmc_strcat(&query, " from ");
    hmc_strcat(&query, state->tsl_user);

    // Do the query
    if(mysql_query(state->cn.handle, query) != 0) {
        hmc_free(query);
        errno = mysql_errno(state->cn.handle);
        return NULL;
    }

    hmc_free(query);
    if((trav.res = mysql_store_result(state->cn.handle)) == NULL) {
        errno = mysql_errno(state->cn.handle);
        return NULL;
    }

    trav.count = mysql_num_rows(trav.res);
    return HX_memdup(&trav, sizeof(struct traverser_state));
}

static int vmysql_usertrav_walk(struct vxpdb_state *vp, void *ptr,
  struct vxpdb_user *res)
{
    struct mysql_state *state = vp->state;
    struct traverser_state *trav = ptr;
    size_t columns, colstart = 0, rem;
    MYSQL_ROW row;
    char *wp;
    int ret;

    // -- part 0 --
    if((row = mysql_fetch_row(trav->res)) == NULL)
        return 0;
    if((ret = mysql_errno(state->cn.handle)) != 0)
        return -ret;
    columns = mysql_num_fields(trav->res);
    vxpdb_user_clean(res);
    rem = nd_row_strlen(trav->res, row, columns);
    wp  = vxpdb_user_alloc(res, rem);

    // -- part 1 --
    if(columns < 6)
        fprintf(stderr, "%s: columns < 6, crash imminent\n", __FUNCTION__);
    export_passwd(res, &row[colstart]);
    colstart += 6;
    columns  -= 6;

    // -- part 2 --
    if(state->perm_shadow) {
        if(columns < 8)
            fprintf(stderr, "%s: columns < 8, crash imminent\n", __FUNCTION__);
        export_shadow(res, &row[colstart]);
        colstart += 8;
        columns  -= 8;
    }

    // -- part 3 --
    if(state->perm_vxshadow) {
        if(columns < 4)
            fprintf(stderr, "%s: columns < 4, crash immiment\n", __FUNCTION__);
        export_vxshadow(res, &row[colstart]);
        colstart += 4;
        columns  -= 4;
    }

    if(columns != 0)
        fprintf(stderr, "Warning: %s: columns != 0\n", __FUNCTION__);
    return 1;
}

static void vmysql_usertrav_free(struct vxpdb_state *vp, void *ptr) {
    struct traverser_state *trav = ptr;
    mysql_free_result(trav->res);
    free(trav);
    return;
}

static int vmysql_userinfo(struct vxpdb_state *vp,
  const struct vxpdb_user *msk, struct vxpdb_user *result, size_t size)
{
/*    hmc_t *query = hmc_minit(NULL, 0);
    hmc_strcat(&query, "select * from ");
    hmc_strcat(&query, st->ctable
    char query[1024];
    snprintf(query, sizeof(query),
        "select * from users,shadow where users.username=shadow.username"*/
    return 0;
}

//-----------------------------------------------------------------------------
static int vmysql_groupadd(struct vxpdb_state *vp,
  const struct vxpdb_group *rq)
{
    struct mysql_state *state = vp->state;
    int ret;

    CHECK_ACCESS();
    if((ret = vmysql_xlock_group(vp->state)) <= 0)
        return ret;
    return 0;
}

static int vmysql_groupmod(struct vxpdb_state *vp,
  const struct vxpdb_group *msk, const struct vxpdb_group *mod)
{
    return 0;
}

static int vmysql_groupdel(struct vxpdb_state *vp,
  const struct vxpdb_group *sr_mask)
{
/*    struct mysql_state *state = vp->state;
    char query[1024], qtmp[512];
    hmc_t *where;
    int ret;

    if(!state->have_shadow)
        return -EACCES;
    if(sr_mask->gr_name == NULL && sr_mask->gr_gid == PDB_NOGID)
        return -EINVAL;
    if((ret = vmysql_xlock_group(state)) <= 0)
        return ret;

    where = where_groupmask(state, sr_mask, WHERE_GR_NAME | WHERE_GR_GID);
    snprintf(query, sizeof(query), "delete from %s where %s",
      state->gr_table, where);
    if(mysql_query(state->cn.handle, query) != 0)
        ret = -mysql_errno(state->cn.handle);

    vmysql_xunlock(state);
    hmc_free(where); */
    return 0; // (ret == 0) ? 1 : ret;
}

void *vmysql_grouptrav_init(struct vxpdb_state *vp) {
    struct mysql_state *st = vp->state;
    struct traverser_state trav;
    char query[1024];

    snprintf(query, sizeof(query), "select %s from %s",
      st->csl_group, st->gr_table);

    if(mysql_query(st->cn.handle, query) != 0 ||
     (trav.res = mysql_store_result(st->cn.handle)) == NULL) {
        errno = mysql_errno(st->cn.handle);
        return NULL;
    }

    trav.count = mysql_num_rows(trav.res);
    return HX_memdup(&trav, sizeof(struct traverser_state));
}

int vmysql_grouptrav_walk(struct vxpdb_state *vp, void *ptr,
  struct vxpdb_group *res)
{
    struct mysql_state *st = vp->state;
    struct traverser_state *trav = ptr;
    size_t columns, rem;
    MYSQL_ROW row;
    char *wp;
    int ret;

    if((row = mysql_fetch_row(trav->res)) == NULL)
        return 0;
    if((ret = mysql_errno(st->cn.handle)) != 0)
        return -ret;
    if((columns = mysql_num_fields(trav->res)) != 2)
        fprintf(stderr, "%s: columns != 2, crash imminent\n", __FUNCTION__);
    rem = nd_row_strlen(trav->res, row, columns);
    wp  = vxpdb_group_alloc(res, rem);
    export_group(res, row);
    return 1;
}

void vmysql_grouptrav_free(struct vxpdb_state *vp, void *ptr) {
    struct traverser_state *trav = ptr;
    mysql_free_result(trav->res);
    free(trav);
    return;
}

int vmysql_groupinfo(struct vxpdb_state *vp, const struct vxpdb_group *srk,
  struct vxpdb_group *res, size_t rsize)
{
    return 0;
}

//-----------------------------------------------------------------------------
static long count_rows(struct mysql_state *state, const char *table) {
    char query[256];
    MYSQL_RES *res;
    MYSQL_ROW row;
    long ret = 0;

    snprintf(query, sizeof(query), "select count(*) from %s", table);
    if(mysql_query(state->cn.handle, query) != 0 ||
     (res = mysql_store_result(state->cn.handle)) == NULL)
        return -mysql_errno(state->cn.handle);

    if((row = mysql_fetch_row(res)) == NULL || *row == NULL)
        goto out;

    ret = strtol(*row, NULL, 0);

 out:
    mysql_free_result(res);
    return ret;
}

static void export_passwd(struct vxpdb_user *dest, const MYSQL_ROW in) {
    HX_strclone(&dest->pw_name, in[0]);
    if(in[1] != NULL) dest->pw_uid = strtol(in[1], NULL, 0);
    if(in[2] != NULL) dest->pw_gid = strtol(in[2], NULL, 0);
    HX_strclone(&dest->pw_real, in[3]);
    HX_strclone(&dest->pw_home, in[4]);
    HX_strclone(&dest->pw_shell, in[5]);
    return;
}

static void export_shadow(struct vxpdb_user *dest, const MYSQL_ROW in) {
    // in[0] is username, we do not need that
    HX_strclone(&dest->sp_passwd, in[1]);
    if(in[2] != NULL) dest->sp_lastchg = strtol(in[2], NULL, 0);
    if(in[3] != NULL) dest->sp_min     = strtol(in[3], NULL, 0);
    if(in[4] != NULL) dest->sp_max     = strtol(in[4], NULL, 0);
    if(in[5] != NULL) dest->sp_warn    = strtol(in[5], NULL, 0);
    if(in[6] != NULL) dest->sp_expire  = ito_expire(strtol(in[6], NULL, 0));
    if(in[7] != NULL) dest->sp_inact   = ito_inact(strtol(in[7], NULL, 0));
    return;
}

static inline void export_vxshadow(struct vxpdb_user *dest,
  const MYSQL_ROW in)
{
    // in[0] is username
    HX_strclone(&dest->vs_uuid, in[1]);
    HX_strclone(&dest->vs_pvgrp, in[2]);
    if(in[3] != NULL)
        dest->vs_defer = strtol(in[3], NULL, 0);
    return;
}

static inline void export_group(struct vxpdb_group *dest, const MYSQL_ROW in) {
    HX_strclone(&dest->gr_name, in[0]);
    // FIXME misses gid
    return;
}

static long find_next_id(struct mysql_state *state, long command) {
    const char *field, *table;
    long start, end;
    char query[256];

    if(command == PDB_NEXTGID_SYS || command == PDB_NEXTGID) {
        field = state->gr_gid;
        table = state->gr_table;
        start = state->gid_min;
        end   = state->gid_max;
    } else {
        field = state->pw_uid;
        table = state->pw_table;
        start = state->uid_min;
        end   = state->uid_max;
    }
    if(command == PDB_NEXTGID_SYS || command == PDB_NEXTUID_SYS) {
        end   = start - 1;
        start = 1;
    }

    snprintf(query, sizeof(query), "select %s from %s where %s=%ld",
     field, table, field, start);

//    if(empty)
//        return start;

/*
mysql> select min(uid)+1 as i from users as u1 where not exists
(select uid from users as u2 where u2.uid=u1.uid+1) and
uid between 1500 and 1509;
*/

    snprintf(query, sizeof(query), "select min(uq0.uid+1) from %s as uq0 "
      "where not exists (select %s from %s as uq1 where "
      "uq1.uid=uq0.uid+1) and uq0.uid >= %ld and uq0.uid <= %ld",
      table, field, table, start, end);

    return 0;
}

static inline long ito_expire(long d) {
    return (d == -1) ? PDB_NO_EXPIRE : d;
}

static inline long ito_inact(long d) {
    return (d == -1) ? PDB_NO_INACTIVE : d;
}

static size_t nd_row_strlen(MYSQL_RES *res, MYSQL_ROW row, size_t col) {
    MYSQL_FIELD *f = mysql_fetch_fields(res);
    size_t size = 0;

    while(col--) {
        if(!IS_NUM(f[col].type) && row[col] != NULL)
            size += strlen(row[col]) + 1;
    }

    return size;
}

static void quote_append(hmc_t **dest, const char *s) {
    size_t orig_len;
    char *ret, *tmp, *fm;

    orig_len = strlen(s) * 2 + 1;
    tmp      = malloc(orig_len);
    if(tmp == NULL) {
        perror("malloc in quote_append in mysql.c");
        abort(); // FIXME: ugly
    }
    if((ret = vxutil_quote(s, 0, &fm)) != NULL) {
        hmc_strcat(dest, "='");
        hmc_strcat(dest, ret);
        hmc_strcat(dest, "'");
    }
    free(fm);
    return;
}

static void read_config(struct mysql_state *state, unsigned int action,
  const char *file)
{
    struct HXoption options_table[] = {
        {.ln = "SOURCE_PRI",            .type = HXTYPE_STRING, .cb = read_config_cb, .uptr = state},
        {.ln = "SOURCE_SEC",            .type = HXTYPE_STRING, .cb = read_config_cb, .uptr = state},
        {.ln = "users.host",            .type = HXTYPE_STRING, .ptr = &state->cn.host},
        {.ln = "users.database",        .type = HXTYPE_STRING, .ptr = &state->cn.database},
        {.ln = "users.db_user",         .type = HXTYPE_STRING, .ptr = &state->cn.user},
        {.ln = "users.db_password",     .type = HXTYPE_STRING, .ptr = &state->cn.passwd},
        {.ln = "users.table",           .type = HXTYPE_STRING, .ptr = &state->pw_table},
        //{.ln = "users.where_clause",    .type = HXTYPE_STRING, .ptr = NULL},
        {.ln = "users.user_column",     .type = HXTYPE_STRING, .ptr = &state->pw_name},
        //{.ln = "users.password_column", .type = HXTYPE_STRING, .ptr = NULL},
        //{.ln = "users.userid_column",   .type = HXTYPE_STRING, .ptr = NULL},
        {.ln = "users.uid_column",      .type = HXTYPE_STRING, .ptr = &state->pw_uid},
        {.ln = "users.gid_column",      .type = HXTYPE_STRING, .ptr = &state->pw_gid},
        {.ln = "users.realname_column", .type = HXTYPE_STRING, .ptr = &state->pw_real},
        {.ln = "users.homedir_column",  .type = HXTYPE_STRING, .ptr = &state->pw_home},
        {.ln = "users.shell_column",    .type = HXTYPE_STRING, .ptr = &state->pw_shell},

        //{.ln = "shadow.host",              .type = HXTYPE_STRING, .ptr = NULL},
        {.ln = "shadow.db_user",           .type = HXTYPE_STRING, .ptr = &state->cn.user2},
        {.ln = "shadow.db_password",       .type = HXTYPE_STRING, .ptr = &state->cn.passwd2},
        {.ln = "shadow.table",             .type = HXTYPE_STRING, .ptr = &state->sp_table},
        //{.ln = "shadow.userid_column",     .type = HXTYPE_STRING, .ptr = NULL},
        //{.ln = "shadow.user_column",       .type = HXTYPE_STRING, .ptr = NULL},
        {.ln = "shadow.password_column",   .type = HXTYPE_STRING, .ptr = &state->sp_passwd},
        {.ln = "shadow.lastchange_column", .type = HXTYPE_STRING, .ptr = &state->sp_lastchg},
        {.ln = "shadow.min_column",        .type = HXTYPE_STRING, .ptr = &state->sp_min},
        {.ln = "shadow.max_column",        .type = HXTYPE_STRING, .ptr = &state->sp_max},
        {.ln = "shadow.warn_column",       .type = HXTYPE_STRING, .ptr = &state->sp_warn},
        {.ln = "shadow.expire_column",     .type = HXTYPE_STRING, .ptr = &state->sp_expire},
        {.ln = "shadow.inact_column",      .type = HXTYPE_STRING, .ptr = &state->sp_inact},

        {.ln = "groups.group_info_table",  .type = HXTYPE_STRING, .ptr = &state->gr_table},
        //{.ln = "groups.where_clause",      .type = HXTYPE_STRING, .ptr = NULL},
        {.ln = "groups.group_name_column", .type = HXTYPE_STRING, .ptr = &state->gr_name},
        //{.ln = "groups.groupid_column",    .type = HXTYPE_STRING, .ptr = NULL},
        {.ln = "groups.gid_column",        .type = HXTYPE_STRING, .ptr = &state->gr_gid},
        //{.ln = "groups.password_column",   .type = HXTYPE_STRING, .ptr = NULL}, // no SGRP

        //{.ln = "groups.members_table",     .type = HXTYPE_STRING, .ptr = &state->gm_table}, // no SGRP
        //{.ln = "groups.member_uid_column", .type = HXTYPE_STRING, .ptr = &state->gm_user},
        //{.ln = "groups.member_gid_column", .type = HXTYPE_STRING, .ptr = &state->gm_group},

        {.ln = "vxshadow.table_name", .type = HXTYPE_STRING, .ptr = &state->vs_table},
        {.ln = "vxshadow.user_col",   .type = HXTYPE_STRING, .ptr = &state->vs_user},
        {.ln = "vxshadow.uuid_col",   .type = HXTYPE_STRING, .ptr = &state->vs_uuid},
        {.ln = "vxshadow.pvgrp_col",  .type = HXTYPE_STRING, .ptr = &state->vs_pvgrp},
        {.ln = "vxshadow.defer_col",  .type = HXTYPE_STRING, .ptr = &state->vs_defer},
        HXOPT_TABLEEND,
    };

    if(action == CONFIG_READ_BASE) {
        struct HXoption autouid_table[] = {
            {.ln = "UID_MIN", .type = HXTYPE_LONG, .ptr = &state->uid_min},
            {.ln = "UID_MAX", .type = HXTYPE_LONG, .ptr = &state->uid_max},
            {.ln = "GID_MIN", .type = HXTYPE_LONG, .ptr = &state->gid_min},
            {.ln = "GID_MAX", .type = HXTYPE_LONG, .ptr = &state->gid_max},
            HXOPT_TABLEEND,
        };
        state->uid_min = state->gid_min = 1000;
        state->uid_max = state->gid_max = 60000;
        HX_shconfig(CONFIG_ETC_VITALNIX "/autouid.conf", autouid_table);
        HX_shconfig(file, options_table);
    } else if(action == CONFIG_READ_NEST) {
        HX_shconfig(file, options_table);
    } else if(action == CONFIG_FREE) {
        HX_shconfig_free(options_table);
    }

    return;
}

static void read_config_cb(const struct HXoptcb *cbi) {
    read_config(cbi->current->uptr, CONFIG_READ_NEST, cbi->s);
    return;
}

static char *s_join(const char *delim, ...) {
    size_t max = 0, dl = 0;
    int not_first = 0;
    va_list vp, wp;
    const char *s;
    char *ret;

    if(delim != NULL)
        dl = strlen(delim);

    va_start(vp, delim);
    va_copy(wp, vp);

    // Find length
    while((s = va_arg(wp, const char *)) != NULL)
        max += strlen(s) + dl;
    va_end(wp);

    // Allocate
    if((ret = malloc(max)) == NULL)
        return NULL;
    *ret = '\0';

    // Compose string
    while((s = va_arg(vp, const char *)) != NULL) {
        if(not_first++ && dl > 0)
            strcat(ret, delim);
        strcat(ret, s);
    }
    va_end(vp);
    return ret;
}

#define PUT_I(SF, MF) \
    do { \
        if(n) hmc_strcat(s, " and "); \
        hmc_strcat(s, (SF)); \
        snprintf(tmp, sizeof(tmp), "=%ld", (MF)); \
        hmc_strcat(s, tmp); \
        n = 1; \
    } while(0)
#define PUT_S(SF, MF) \
    do { \
        if(n) hmc_strcat(s, " and "); \
        hmc_strcat(s, (SF)); \
        quote_append(s, (MF)); \
        n = 1; \
    } while(0)
static hmc_t *sql_groupmask(hmc_t **s, const struct mysql_state *state,
  const struct vxpdb_group *mask, unsigned int flags)
{
    char tmp[Z_32];
    int n = 0;
    if(*s == NULL)
        fprintf(stderr, "Error: %s called with *s == NULL\n", __FUNCTION__);
    if(mask->gr_name != NULL)
        PUT_S(state->gr_name, mask->gr_name);
    if(mask->gr_gid != PDB_NOGID)
        PUT_I(state->gr_gid, mask->gr_gid);
    return *s;
}

/*
flags can contain:
    MASK_DELETE = only put username and uid into sql query
    MASK_SET    = add extra fields to set username in ->sp_table and ->vs_table
*/

static hmc_t *sql_usermask(hmc_t **s, const struct mysql_state *state,
  const struct vxpdb_user *mask, unsigned int flags)
{
    char tmp[Z_32];
    int n = 0;

    if(*s == NULL)
        fprintf(stderr, "Error: %s called with *s == NULL\n", __FUNCTION__);

    if(mask->pw_name != NULL) {
        PUT_S(state->pw_name, mask->pw_name);
        if((flags & MASK_SET) && state->perm_shadow)
            PUT_S(state->sp_user, mask->pw_name);
        if((flags & MASK_SET) && state->perm_vxshadow)
            PUT_S(state->vs_user, mask->pw_name);
    }
    if(mask->pw_uid != PDB_NOUID)
        PUT_I(state->pw_uid, mask->pw_uid);
    if(flags & MASK_DELETE)
        return *s;
    if(mask->pw_gid   != PDB_NOGID) PUT_I(state->pw_gid,   mask->pw_gid);
    if(mask->pw_real  != NULL)      PUT_S(state->pw_real,  mask->pw_real);
    if(mask->pw_home  != NULL)      PUT_S(state->pw_home,  mask->pw_home);
    if(mask->pw_shell != NULL)      PUT_S(state->pw_shell, mask->pw_shell);
    return *s;
}
#undef PUT_I
#undef PUT_S

//=============================================================================
