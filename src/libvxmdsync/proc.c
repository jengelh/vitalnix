/*
 *	libvxmdsync/proc.c
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <libHX/arbtree.h>
#include <libHX/deque.h>
#include <libHX/misc.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include <vitalnix/libvxmdsync/libvxmdsync.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
enum {
	ACTION_UPDATE       = 1 << 0, /* do not re-add this user */
	ACTION_DEFER_START  = 1 << 1, /* start deferred deletion */
	ACTION_DEFER_WAIT   = 1 << 2, /* waits for deletion */
	ACTION_DEFER_STOP   = 1 << 3, /* cancel deferred deletion */
	ACTION_DELETE_NOW   = 1 << 4, /* select for final deletion */
};

/* Functions */
static int mdsync_update(struct mdsync_workspace *);
static int mdsync_defer_start(struct mdsync_workspace *);
static int mdsync_defer_stop(struct mdsync_workspace *);

static inline bool create_home(const struct mdsync_workspace *, const char *,
	unsigned int, unsigned int);
static inline char *now_in_ymdhms(char *, size_t);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL void mdsync_compare(struct mdsync_workspace *w)
{
	const struct vxdb_group *grp = &w->dest_group;
	const struct HXbtree_node *ln_node;
	unsigned int users_proc, users_max, decision;
	struct vxdb_user pwd = {};
	struct vxeds_entry *eds;
	bool analyze;
	const unsigned int defer = w->config.add_opts.defaults.vs_defer;
	void *travp;

	users_proc = 0;
	users_max  = vxdb_modctl(w->database, VXDB_COUNT_USERS);
	travp      = vxdb_usertrav_init(w->database);

	while (vxdb_usertrav_walk(w->database, travp, &pwd)) {
		/* Record username in @lnlist */
		ln_node = HXbtree_add(w->lnlist, pwd.pw_name);
		analyze = (grp->gr_gid != VXDB_NOGID && grp->gr_gid == pwd.pw_gid) ||
		          (grp->gr_name != NULL && pwd.pw_igrp != NULL &&
		          strcmp(grp->gr_name, pwd.pw_igrp) == 0);

		if (w->report != NULL)
			w->report(MDREP_COMPARE, w, ++users_proc, users_max);
		if (!analyze)
			continue;

		/*
		 * Users we look for (only specified group)
		 */
		if (pwd.vs_uuid != NULL &&
		    (eds = HXbtree_get(w->add_req, pwd.vs_uuid)) != NULL) {
			/*
			 * VXDB user found in EDS:
			 * Keep, and remove from add_req
			 */
			decision = ACTION_UPDATE | ((pwd.vs_defer != 0) ?
			           ACTION_DEFER_STOP : 0);
		} else if (defer != 0) {
			/* VXDB user not found in EDS */
			if (pwd.vs_defer == 0)
				decision = ACTION_DEFER_START;
			else if (vxutil_now_iday() >= pwd.vs_defer + defer)
				/*
				 * Did not reappear on EDS list within timelimit
				 */
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
		if (decision & ACTION_UPDATE) {
			if (strcmp(eds->pvgrp, pwd.vs_uuid) != 0) {
				struct vxdb_user *copy = vxdb_user_dup(&pwd);
				HXmc_strcpy(&copy->vs_pvgrp, eds->pvgrp);
				HXbtree_add(w->update_req, copy->vs_uuid, copy);
			}
			vxeds_free_entry(HXbtree_del(w->add_req, pwd.vs_uuid));
		}
		if (decision & ACTION_DEFER_START)
			HXdeque_push(w->defer_start, ln_node->key);
		if (decision & ACTION_DEFER_WAIT)
			HXdeque_push(w->defer_wait, ln_node->key);
		if (decision & ACTION_DEFER_STOP)
			HXdeque_push(w->defer_stop, ln_node->key);
		if (decision & ACTION_DELETE_NOW)
			HXdeque_push(w->delete_now, ln_node->key);

		/*
		 * username is shared between all these data structures, but to
		 * define a common point, they are owned by @w->lnlist.
		 * (Because this is the only structure which keeps _all_
		 * usernames.)
		 */
	}

	vxdb_usertrav_free(w->database, travp);
	vxdb_user_free(&pwd, false);
	if (w->report != NULL)
		w->report(MDREP_COMPARE, w, users_max, users_max);
}

EXPORT_SYMBOL void mdsync_compare_simple(struct mdsync_workspace *w)
{
	void *travp = vxdb_usertrav_init(w->database);
	struct vxdb_user pwd = {};
	int count, count_max;

	count     = 0;
	count_max = vxdb_modctl(w->database, VXDB_COUNT_USERS);

	if (travp != NULL)
		while (vxdb_usertrav_walk(w->database, travp, &pwd)) {
			/* No invalid references later, thanks to %HXBT_CDATA */
			HXbtree_add(w->lnlist, pwd.pw_name);
			if (w->report != NULL)
				w->report(MDREP_COMPARE, w, ++count, count_max);
		}

	vxdb_usertrav_free(w->database, travp);
	vxdb_user_free(&pwd, false);
}

EXPORT_SYMBOL int mdsync_add(struct mdsync_workspace *w)
{
	struct HXbtree *master_catalog = NULL, *user_catalog = NULL;
	struct mdsync_config *c = &w->config;
	char home_path[MAXFNLEN], plain_pw[64];
	unsigned int users_proc, users_max;
	const struct HXbtree_node *node;
	struct vxdb_user chk = {};
	struct vxdb_user out;
	void *travp;
	int ret = 1;

	users_proc = 0;
	users_max  = w->add_req->items;
	if (users_max == 0)
		return 1;

	if (c->add_opts.master_preadd != NULL ||
	    c->add_opts.master_postadd != NULL) {
		master_catalog = HXformat_init();
		HXformat_add(master_catalog, "USERS", &users_max, HXTYPE_UINT);
	}
	if (c->add_opts.user_preadd != NULL ||
	    c->add_opts.user_postadd != NULL) {
		user_catalog = HXformat_init();
		HXformat_add(user_catalog, "USERNAME", out.pw_name,  HXTYPE_STRING);
		HXformat_add(user_catalog, "REALNAME", out.pw_real,  HXTYPE_STRING);
		HXformat_add(user_catalog, "UID",     &out.pw_uid,   HXTYPE_UINT);
		HXformat_add(user_catalog, "GID",     &out.pw_gid,   HXTYPE_UINT);
		/* igrp, sgrp unimplemented in Vitalnix */
		HXformat_add(user_catalog, "GROUP",    "",           HXTYPE_STRING);
		HXformat_add(user_catalog, "SGRP",     "",           HXTYPE_STRING);
		HXformat_add(user_catalog, "HOME",     out.pw_home,  HXTYPE_STRING);
		HXformat_add(user_catalog, "SHELL",    out.pw_shell, HXTYPE_STRING);
	}

	if (c->add_opts.master_preadd != NULL)
		vxutil_replace_run(c->add_opts.master_preadd, master_catalog);
	if (c->new_pw_length >= sizeof(plain_pw))
		c->new_pw_length = sizeof(plain_pw) - 1;

	travp = HXbtrav_init(w->add_req);
	while ((node = HXbtraverse(travp)) != NULL) {
		struct vxeds_entry *in = node->data;

		vxdb_user_clean(&out);
		out.pw_name  = in->username;
		out.pw_uid   = VXDB_AUTOUID;
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

		if (c->new_pw_length < 0) {
			out.sp_passwd  = "!";
			out.sp_lastchg = 0;
		} else if (c->new_pw_length == 0) {
			out.sp_passwd  = "";
			out.sp_lastchg = vxutil_now_iday();
		} else {
			vxutil_genpw(plain_pw, c->new_pw_length, GENPW_O1CASE |
			             GENPW_O1DIGIT | c->genpw_type);
			vxutil_phash(plain_pw, NULL, c->phash_type,
			             &out.sp_passwd);
			vxutil_phash(plain_pw, NULL, VXPHASH_SMBNT,
			             &out.sp_ntpasswd);
			out.sp_lastchg = vxutil_now_iday();
		}

		out.sp_min    = c->add_opts.defaults.sp_min;
		out.sp_max    = c->add_opts.defaults.sp_max;
		out.sp_warn   = c->add_opts.defaults.sp_warn;
		out.sp_expire = c->add_opts.defaults.sp_expire;
		out.sp_inact  = c->add_opts.defaults.sp_inact;

		if (c->add_opts.user_preadd != NULL)
			vxutil_replace_run(c->add_opts.user_preadd,
			                   user_catalog);

		if ((ret = vxdb_useradd(w->database, &out)) <= 0 ||
		    (ret = vxdb_getpwnam(w->database, out.pw_name, &chk)) <= 0)
			break;

		if (c->postadd_flush && c->add_opts.user_postadd != NULL)
			vxdb_modctl(w->database, VXDB_FLUSH);

		if (w->logfile == NULL) {
			HXmc_strcat(&w->output_data, out.pw_name);
			HXmc_strcat(&w->output_data, ":");
			HXmc_strcat(&w->output_data, plain_pw);
			HXmc_strcat(&w->output_data, "\n");
		} else {
			fprintf(w->logfile, "+:%s:%s:%s:%s:%s\n", in->pvgrp, in->surname,
			        in->first_name, out.pw_name, plain_pw);
		}

		if (c->new_pw_length > 0 && out.sp_passwd != NULL) {
			memset(out.sp_passwd, 0, strlen(out.sp_passwd));
			free(out.sp_passwd);
		}

		/* Create home directory and optionally copy skeleton dir */
		if (!create_home(w, chk.pw_home, chk.pw_uid, chk.pw_gid)) {
			printf("\n");
			fprintf(stderr, "Warning: Could not create home directory %s: %s\n",
			        out.pw_home, strerror(errno));
		}

		if (c->add_opts.user_postadd != NULL)
			vxutil_replace_run(c->add_opts.user_postadd, user_catalog);
		if (w->report != NULL)
			w->report(MDREP_ADD, w, ++users_proc, users_max);
	}

	memset(plain_pw, 0, sizeof(plain_pw));
	HXbtrav_free(travp);
	vxdb_modctl(w->database, VXDB_FLUSH);
	vxdb_user_free(&chk, false);

	if (ret > 0 && c->add_opts.master_postadd != NULL)
		vxutil_replace_run(c->add_opts.master_postadd, master_catalog);

	if (master_catalog != NULL)
		HXformat_free(master_catalog);
	if (user_catalog != NULL)
		HXformat_free(user_catalog);

	return ret;
}

EXPORT_SYMBOL int mdsync_mod(struct mdsync_workspace *w)
{
	/* update defer timers and pvgrp */
	int ret = 0;

	/* FIXME: postmod not implemented */

	if (w->update_req->items > 0 && (ret = mdsync_update(w)) <= 0)
		return ret;
	if (w->defer_start->items > 0 && (ret = mdsync_defer_start(w)) <= 0)
		return ret;
	if (w->defer_stop->items > 0 && (ret = mdsync_defer_stop(w)) <= 0)
		return ret;

	return 1;
}

EXPORT_SYMBOL int mdsync_del(struct mdsync_workspace *w)
{
	/* Deleting the old users */
	struct HXbtree *master_catalog = NULL, *user_catalog = NULL;
	const struct mdsync_config *c = &w->config;
	unsigned int users_proc, users_max;
	char current_date[MAXSNLEN];
	struct HXdeque_node *travp;
	struct vxdb_user res = {};
	int ret;

	users_proc = 0;
	users_max  = w->delete_now->items;

	if (users_max == 0)
		return 1;

	/*
	 * We generate a time stamp here so that a possible backup operation
	 * will write into the same directory for all users to be deleted.
	 */
	now_in_ymdhms(current_date, sizeof(current_date));

	if (c->del_opts.master_predel != NULL ||
	    c->del_opts.master_postdel != NULL) {
		master_catalog = HXformat_init();
		HXformat_add(master_catalog, "DATE",  current_date, HXTYPE_STRING);
		HXformat_add(master_catalog, "USERS", &users_max,   HXTYPE_UINT);
	}

	if (c->del_opts.user_predel != NULL || c->del_opts.user_postdel != NULL) {
		user_catalog = HXformat_init();
		HXformat_add(user_catalog, "USERNAME", &res.pw_name, HXTYPE_STRP);
		HXformat_add(user_catalog, "UID",      &res.pw_uid,  HXTYPE_UINT);
		HXformat_add(user_catalog, "GID",      &res.pw_gid,  HXTYPE_UINT);
		HXformat_add(user_catalog, "GROUP",    &res.pw_igrp, HXTYPE_STRP);
		HXformat_add(user_catalog, "HOME",     &res.pw_home, HXTYPE_STRP);
		HXformat_add(user_catalog, "DATE",     current_date, HXTYPE_STRING);
	}

	if (c->del_opts.master_predel != NULL)
		vxutil_replace_run(c->del_opts.master_predel, master_catalog);

	for (travp = w->delete_now->first;
	    travp != NULL; travp = travp->next) {
		if ((ret = vxdb_getpwnam(w->database, travp->ptr, &res)) < 0) {
			fprintf(stderr, "%s()+vxdb_getpwnam(): %s\n",
			        __func__, strerror(errno));
			return ret;
		} else if (ret == 0) {
			fprintf(stderr, "Warning: Someone deleted \"%s\" "
			        "before we did.\n", res.pw_name);
			continue;
		}

		if (c->del_opts.user_predel != NULL)
			vxutil_replace_run(c->del_opts.user_predel, user_catalog);

		HX_rrmdir(res.pw_home);

		if ((ret = vxdb_userdel(w->database, res.pw_name)) < 0) {
			fprintf(stderr, "%s()+vxdb_userdel(): %s\n",
			        __func__, strerror(errno));
			return ret;
		}

		if (w->logfile != NULL)
			fprintf(w->logfile, "-:%s\n", res.pw_name);
		if (c->del_opts.user_postdel != NULL)
			vxutil_replace_run(c->del_opts.user_postdel, user_catalog);
		if (w->report != NULL)
			w->report(MDREP_DELETE, w, ++users_proc, users_max);
	}

	if (c->del_opts.master_postdel != NULL)
		vxutil_replace_run(c->del_opts.master_postdel, master_catalog);

	if (master_catalog != NULL)
		HXformat_free(master_catalog);
	if (user_catalog != NULL)
		HXformat_free(user_catalog);

	vxdb_user_free(&res, false);
	return 1;
}

//-----------------------------------------------------------------------------
static int mdsync_update(struct mdsync_workspace *w)
{
	unsigned int users_proc = 0, users_max = w->update_req->items;
	void *travp = HXbtrav_init(w->update_req);
	struct vxdb_user mod_rq;
	const struct HXbtree_node *node;
	const struct vxdb_user *act;
	int ret;

	while ((node = HXbtraverse(travp)) != NULL) {
		vxdb_user_nomodify(&mod_rq);
		act = node->data;
		mod_rq.vs_pvgrp   = act->vs_pvgrp;

		ret = vxdb_usermod(w->database, act->pw_name, &mod_rq);
		if (ret <= 0)
			return ret;
		if (w->report != NULL)
			w->report(MDREP_UPDATE, w, ++users_proc, users_max);
	}

	return 1;
}

static int mdsync_defer_start(struct mdsync_workspace *w)
{
	unsigned int users_proc = 0, users_max = w->defer_start->items;
	struct vxdb_user mod_rq;
	const struct HXdeque_node *node;
	long today = vxutil_now_iday();
	int ret;

	for (node = w->defer_start->first; node != NULL; node = node->next) {
		vxdb_user_nomodify(&mod_rq);
		mod_rq.vs_defer   = today;

		ret = vxdb_usermod(w->database, node->ptr, &mod_rq);
		if (ret <= 0)
			return ret;
		if (w->report != NULL)
			w->report(MDREP_DSTART, w, ++users_proc, users_max);
	}

	return 1;
}

static int mdsync_defer_stop(struct mdsync_workspace *w)
{
	unsigned int users_proc = 0, users_max = w->defer_stop->items;
	struct vxdb_user mod_rq;
	const struct HXdeque_node *node;
	int ret;

	for (node = w->defer_stop->first; node != NULL; node = node->next) {
		vxdb_user_nomodify(&mod_rq);
		mod_rq.vs_defer   = 0;

		ret = vxdb_usermod(w->database, node->ptr, &mod_rq);
		if (ret <= 0)
			return ret;
		if (w->report != NULL)
			w->report(MDREP_DSTOP, w, ++users_proc, users_max);
	}

	return 1;
}

//-----------------------------------------------------------------------------
static inline bool create_home(const struct mdsync_workspace *w, const char *d,
    unsigned int uid, unsigned int gid)
{
	const struct mdsync_config *c = &w->config;

	if (HX_mkdir(d) <= 0)
		return false;
	lchown(d, uid, gid);
	chmod(d, (S_IRWXU | S_IRWXG | S_IRWXO) & ~c->add_opts.umask);
	if (c->add_opts.skel_dir != NULL)
		HX_copy_dir(c->add_opts.skel_dir, d,
		            HXF_UID | HXF_GID | HXF_KEEP, uid, gid);
	return true;
}

static inline char *now_in_ymdhms(char *buf, size_t count)
{
	/* Write the current time as YYMMDD-HHMMSS into "buf" */
	time_t now = time(NULL);
	struct tm res;

	strftime(buf, MAXSNLEN, "%Y%m%d-%H%M%S", localtime_r(&now, &res));
	return buf;
}
