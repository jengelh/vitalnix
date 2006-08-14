/*=============================================================================
Vitalnix User Management Suite
libvxmdsync/proc.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2006
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <libHX.h>
#include "compiler.h"
#include "libvxeds/libvxeds.h"
#include "libvxmdsync/libvxmdsync.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"
#include "libvxpdb/xwfunc.h"
#include "libvxutil/defines.h"
#include "libvxutil/libvxutil.h"

// Definitions
enum {
    ACTION_UPDATE       = 1 << 0, // do not re-add this user
    ACTION_DEFER_START  = 1 << 1, // start deferred deletion
    ACTION_DEFER_WAIT   = 1 << 2, // waits for deletion
    ACTION_DEFER_STOP   = 1 << 3, // cancel deferred deletion
    ACTION_DELETE_NOW   = 1 << 4, // select for final deletion
};

// Functions
static int mdsync_update(struct mdsync_workspace *);
static int mdsync_defer_start(struct mdsync_workspace *);
static int mdsync_defer_stop(struct mdsync_workspace *);

static inline int create_home(const struct mdsync_workspace *, const char *,
    long, long);
static inline char *now_in_ymdhms(char *, size_t);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL void mdsync_compare(struct mdsync_workspace *w)
{
    const struct vxpdb_group *grp = &w->dest_group;
    const struct HXbtree_node *ln_node;
    long users_proc, users_max;
    struct vxpdb_user pwd = {};
    struct vxeds_entry *eds;
    int analyze, decision;
    long defer = w->config.add_opts.defaults.vs_defer;
    void *travp;

    users_proc = 0;
    users_max  = vxpdb_modctl(w->database, PDB_COUNT_USERS);
    travp      = vxpdb_usertrav_init(w->database);

    while(vxpdb_usertrav_walk(w->database, travp, &pwd)) {
        // Record username in @lnlist
        ln_node = HXbtree_add(w->lnlist, pwd.pw_name);
        analyze = (grp->gr_gid != PDB_NOGID && grp->gr_gid == pwd.pw_gid) ||
                  (grp->gr_name != NULL && pwd.pw_igrp != NULL &&
                   strcmp(grp->gr_name, pwd.pw_igrp) == 0);

        if(w->report != NULL)
            w->report(MDREP_COMPARE, w, ++users_proc, users_max);
        if(!analyze)
            continue;

        /*
         * Users we look for (only specified group)
         */
        if(pwd.vs_uuid != NULL && (eds = HXbtree_get(w->add_req, pwd.vs_uuid)) != NULL) {
            // PDB user found in EDS: Keep, and remove from add_req.
            decision = ACTION_UPDATE | ((pwd.vs_defer > 0) ?
                       ACTION_DEFER_STOP : 0);
        } else if(defer > 0) {
            // PDB user not found in EDS
            if(pwd.vs_defer == 0)
                decision = ACTION_DEFER_START;
            else if(vxutil_now_iday() >= pwd.vs_defer + defer)
                // Did not reappear on EDS list within timelimit
                decision = ACTION_DELETE_NOW;
            else
                decision = ACTION_DEFER_WAIT;
        } else {
            decision = ACTION_DELETE_NOW;
        }
        ++w->num_grp;

        /*
         *      Action
         */
        if(decision & ACTION_UPDATE) {
            if(strcmp(eds->pvgrp, pwd.vs_uuid) != 0) {
                struct vxpdb_user *copy = vxpdb_user_dup(&pwd);
                hmc_strasg(&copy->vs_pvgrp, eds->pvgrp);
                HXbtree_add(w->update_req, copy->vs_uuid, copy);
            }
            vxeds_free_entry(HXbtree_del(w->add_req, pwd.vs_uuid));
        }
        if(decision & ACTION_DEFER_START)
            HXdeque_push(w->defer_start, ln_node->key);
        if(decision & ACTION_DEFER_WAIT)
            HXdeque_push(w->defer_wait, ln_node->key);
        if(decision & ACTION_DEFER_STOP)
            HXdeque_push(w->defer_stop, ln_node->key);
        if(decision & ACTION_DELETE_NOW)
            HXdeque_push(w->delete_now, ln_node->key);

        /* username is shared between all these data structures, but to define
        a common point, they are owned by @w->lnlist. (Because this is the
        only structure which keeps _all_ usernames.) */
    }

    vxpdb_usertrav_free(w->database, travp);
    vxpdb_user_free(&pwd, 0);
    if(w->report != NULL)
        w->report(MDREP_COMPARE, w, users_max, users_max);
    return;
}

EXPORT_SYMBOL void mdsync_compare_simple(struct mdsync_workspace *w)
{
    void *travp = vxpdb_usertrav_init(w->database);
    struct vxpdb_user pwd = {};
    int count, count_max;

    count     = 0;
    count_max = vxpdb_modctl(w->database, PDB_COUNT_USERS);

    if(travp != NULL)
        while(vxpdb_usertrav_walk(w->database, travp, &pwd)) {
            // No invalid references later, thanks to HXBT_CDATA
            HXbtree_add(w->lnlist, pwd.pw_name);
            if(w->report != NULL)
                w->report(MDREP_COMPARE, w, ++count, count_max);
        }

    vxpdb_usertrav_free(w->database, travp);
    vxpdb_user_free(&pwd, 0);
    return;
}

EXPORT_SYMBOL int mdsync_add(struct mdsync_workspace *w)
{
    struct mdsync_config *c = &w->config;
    char home_path[MAXFNLEN], plain_pw[64];
    unsigned long users_proc, users_max;
    const struct HXbtree_node *node;
    void *travp;
    int ret;
    struct HXoption master_catalog[] = {
        {.sh = 'n', .type = HXTYPE_ULONG, .ptr = &users_max},
        HXOPT_TABLEEND,
    };

    users_proc = 0;
    users_max  = w->add_req->itemcount;

    if(users_max == 0) // Nothing to do
        return 1;

    if(c->add_opts.master_preadd != NULL)
        vxutil_replace_run(c->add_opts.master_preadd, master_catalog);
    if(c->new_pw_length >= sizeof(plain_pw))
        c->new_pw_length = sizeof(plain_pw) - 1;

    travp = HXbtrav_init(w->add_req, NULL);
    while((node = HXbtraverse(travp)) != NULL) {
        struct vxeds_entry *in = node->data;
        struct vxpdb_user out    = {};
        struct HXoption user_catalog[] = {
            {.sh = 'l', .type = HXTYPE_STRP, .ptr = &out.pw_name},
            {.sh = 'n', .type = HXTYPE_STRP, .ptr = &out.pw_real},
            {.sh = 'u', .type = HXTYPE_LONG, .ptr = &out.pw_uid},
            {.sh = 'g', .type = HXTYPE_LONG, .ptr = &out.pw_gid},
            {.sh = 'G', .type = HXTYPE_STRP, .ptr = &out.pw_igrp},
            {.sh = 'S', .type = HXTYPE_STRP, .ptr = &out.pw_sgrp},
            {.sh = 'h', .type = HXTYPE_STRP, .ptr = &out.pw_home},
            {.sh = 's', .type = HXTYPE_STRP, .ptr = &out.pw_shell},
            HXOPT_TABLEEND,
        };

        out.pw_name  = in->username;
        out.pw_uid   = PDB_AUTOUID;
        out.pw_gid   = w->dest_group.gr_gid;
        out.pw_igrp  = w->dest_group.gr_name;
        out.pw_real  = in->full_name;
        out.pw_home  = vxutil_propose_home(home_path, sizeof(home_path),
                       c->add_opts.home_base, in->username,
                       c->add_opts.split_level);
        out.pw_shell = c->add_opts.defaults.pw_shell;
        out.pw_sgrp  = NULL;
        out.vs_uuid  = in->uuid;
        out.vs_pvgrp = in->pvgrp;

        if(c->new_pw_length < 0) {
            out.sp_passwd  = "!";
            out.sp_lastchg = 0;
        } else if(c->new_pw_length == 0) {
            out.sp_passwd  = "";
            out.sp_lastchg = vxutil_now_iday();
        } else {
            vxutil_genpw(plain_pw, c->new_pw_length, GENPW_O1CASE |
                         GENPW_O1DIGIT | c->genpw_type);
            vxutil_cryptpw(plain_pw, NULL, c->crypw_type, &out.sp_passwd);
            out.sp_lastchg = vxutil_now_iday();
        }

        out.sp_min    = c->add_opts.defaults.sp_min;
        out.sp_max    = c->add_opts.defaults.sp_max;
        out.sp_warn   = c->add_opts.defaults.sp_warn;
        out.sp_expire = c->add_opts.defaults.sp_expire;
        out.sp_inact  = c->add_opts.defaults.sp_inact;

        if(c->add_opts.user_preadd != NULL)
            vxutil_replace_run(c->add_opts.user_preadd, user_catalog);

        if((ret = vxpdb_useradd(w->database, &out)) <= 0) {
            memset(plain_pw, 0, sizeof(plain_pw));
            HXbtrav_free(travp);
            return ret;
        }

        if(c->postadd_flush && c->add_opts.user_postadd != NULL)
            vxpdb_modctl(w->database, PDB_FLUSH);

        if(w->logfile == NULL) {
            hmc_strcat(&w->output_data, out.pw_name);
            hmc_strcat(&w->output_data, ":");
            hmc_strcat(&w->output_data, plain_pw);
            hmc_strcat(&w->output_data, "\n");
        } else {
            fprintf(w->logfile, "+:%s:%s:%s:%s:%s\n", in->pvgrp, in->surname,
                    in->first_name, out.pw_name, plain_pw);
        }

        if(c->new_pw_length > 0 && out.sp_passwd != NULL) {
            memset(out.sp_passwd, 0, strlen(out.sp_passwd));
            free(out.sp_passwd);
        }

        // Create home directory and optionally copy skeleton dir
        if(!create_home(w, out.pw_home, out.pw_uid, out.pw_gid)) {
            printf("\n");
            fprintf(stderr, "Warning: Could not create home directory %s: %s\n",
                    out.pw_home, strerror(errno));
        }

        if(c->add_opts.user_postadd != NULL)
            vxutil_replace_run(c->add_opts.user_postadd, user_catalog);
        if(w->report != NULL)
            w->report(MDREP_ADD, w, ++users_proc, users_max);
    }

    memset(plain_pw, 0, sizeof(plain_pw));
    HXbtrav_free(travp);
    vxpdb_modctl(w->database, PDB_FLUSH);

    if(c->add_opts.master_postadd != NULL)
        vxutil_replace_run(c->add_opts.master_postadd, master_catalog);

    return 1;
}

EXPORT_SYMBOL int mdsync_mod(struct mdsync_workspace *w)
{
    // update defer timers and pvgrp
    int ret = 0;

    // FIXME: postmod not implemented

    if(w->update_req->itemcount > 0 && (ret = mdsync_update(w)) <= 0)
        return ret;
    if(w->defer_start->itemcount > 0 && (ret = mdsync_defer_start(w)) <= 0)
        return ret;
    if(w->defer_stop->itemcount > 0 && (ret = mdsync_defer_stop(w)) <= 0)
        return ret;

    return 1;
}

EXPORT_SYMBOL int mdsync_del(struct mdsync_workspace *w)
{
    // Deleting the old users
    const struct mdsync_config *c = &w->config;
    unsigned long users_proc, users_max;
    char current_date[MAXSNLEN];
    struct HXdeque_node *travp;
    struct vxpdb_user res = {};
    int ret;
    struct HXoption master_catalog[] = {
        {.sh = 'd', .type = HXTYPE_STRING, .ptr = current_date},
        {.sh = 'n', .type = HXTYPE_ULONG,  .ptr = &users_max},
        HXOPT_TABLEEND,
    };

    users_proc = 0;
    users_max  = w->delete_now->itemcount;

    if(users_max == 0)
        return 1;

    /* We generate a time stamp here so that a possible backup operation will
    write into the same directory for all users to be deleted. */
    now_in_ymdhms(current_date, sizeof(current_date));

    if(c->del_opts.master_predel != NULL)
        vxutil_replace_run(c->del_opts.master_predel, master_catalog);

    for(travp = w->delete_now->first; travp != NULL; travp = travp->Next) {
        struct HXoption user_catalog[] = {
            {.sh = 'l', .type = HXTYPE_STRING, .ptr = res.pw_name},
            {.sh = 'u', .type = HXTYPE_LONG,   .ptr = &res.pw_uid},
            {.sh = 'g', .type = HXTYPE_LONG,   .ptr = &res.pw_gid},
            {.sh = 'G', .type = HXTYPE_STRING, .ptr = res.pw_igrp},
            {.sh = 'h', .type = HXTYPE_STRING, .ptr = res.pw_home},
            {.sh = 'd', .type = HXTYPE_STRING, .ptr = current_date},
            HXOPT_TABLEEND,
        };

        if((ret = vxpdb_getpwnam(w->database, travp->ptr, &res)) < 0) {
            fprintf(stderr, "%s()+pdb_getpwnam(): %s\n", __FUNCTION__,
                    strerror(errno));
            return ret;
        } else if(ret == 0) {
            fprintf(stderr, "Warning: Someone deleted \"%s\" before "
                    "we did.\n", res.pw_name);
            continue;
        }

        if(c->del_opts.user_predel != NULL)
            vxutil_replace_run(c->del_opts.user_predel, user_catalog);

        HX_rrmdir(res.pw_home);

        if((ret = vxpdb_userdel(w->database, &res)) < 0) {
            fprintf(stderr, "%s()+pdb_userdel(): %s\n", __FUNCTION__,
                    strerror(errno));
            return ret;
        }

        if(w->logfile != NULL)
            fprintf(w->logfile, "-:%s\n", res.pw_name);
        if(c->del_opts.user_postdel != NULL)
            vxutil_replace_run(c->del_opts.user_postdel, user_catalog);
        if(w->report != NULL)
            w->report(MDREP_DELETE, w, ++users_proc, users_max);
    }

    if(c->del_opts.master_postdel != NULL)
        vxutil_replace_run(c->del_opts.master_postdel, master_catalog);

    vxpdb_user_free(&res, 0);
    return 1;
}

//-----------------------------------------------------------------------------
static int mdsync_update(struct mdsync_workspace *w) {
    long users_proc = 0, users_max      = w->update_req->itemcount;
    void *travp = HXbtrav_init(w->update_req, NULL);
    struct vxpdb_user search_rq, mod_rq;
    const struct HXbtree_node *node;
    const struct vxpdb_user *act;
    int ret;

    while((node = HXbtraverse(travp)) != NULL) {
        vxpdb_user_clean(&search_rq);
        vxpdb_user_nomodify(&mod_rq);
        act = node->data;
        search_rq.pw_name = act->pw_name;
        mod_rq.vs_pvgrp   = act->vs_pvgrp;

        if((ret = vxpdb_usermod(w->database, &search_rq, &mod_rq)) <= 0)
            return ret;
        if(w->report != NULL)
            w->report(MDREP_UPDATE, w, ++users_proc, users_max);
    }

    return 1;
}

static int mdsync_defer_start(struct mdsync_workspace *w) {
    long users_proc = 0, users_max      = w->defer_start->itemcount;
    struct vxpdb_user search_rq, mod_rq;
    const struct HXdeque_node *node;
    long today                          = vxutil_now_iday();
    int ret;

    for(node = w->defer_start->first; node != NULL; node = node->Next) {
        vxpdb_user_clean(&search_rq);
        vxpdb_user_nomodify(&mod_rq);
        search_rq.pw_name = node->ptr;
        mod_rq.vs_defer   = today;

        if((ret = vxpdb_usermod(w->database, &search_rq, &mod_rq)) <= 0)
            return ret;
        if(w->report != NULL)
            w->report(MDREP_DSTART, w, ++users_proc, users_max);
    }

    return 1;
}

static int mdsync_defer_stop(struct mdsync_workspace *w) {
    long users_proc = 0, users_max      = w->defer_stop->itemcount;
    struct vxpdb_user search_rq, mod_rq;
    const struct HXdeque_node *node;
    int ret;

    for(node = w->defer_stop->first; node != NULL; node = node->Next) {
        vxpdb_user_clean(&search_rq);
        vxpdb_user_nomodify(&mod_rq);
        search_rq.pw_name = node->ptr;
        mod_rq.vs_defer   = 0;

        if((ret = vxpdb_usermod(w->database, &search_rq, &mod_rq)) <= 0)
            return ret;
        if(w->report != NULL)
            w->report(MDREP_DSTOP, w, ++users_proc, users_max);
    }

    return 1;
}

//-----------------------------------------------------------------------------
static inline int create_home(const struct mdsync_workspace *w, const char *d,
 long uid, long gid)
{
    const struct mdsync_config *c = &w->config;

    if(HX_mkdir(d) <= 0)
        return 0;
    lchown(d, uid, gid);
    chmod(d, (S_IRWXU | S_IRWXG | S_IRWXO) & ~c->add_opts.umask);
    if(c->add_opts.skel_dir != NULL)
        HX_copy_dir(c->add_opts.skel_dir, d, HXF_UID | HXF_GID | HXF_KEEP,
         uid, gid);
    return 1;
}

static inline char *now_in_ymdhms(char *buf, size_t count) {
    // Write the current time as YYMMDD-HHMMSS into "buf"
    time_t now = time(NULL);
    struct tm res;
    strftime(buf, MAXSNLEN, "%Y%m%d-%H%M%S", localtime_r(&now, &res));
    return buf;
}

//=============================================================================
