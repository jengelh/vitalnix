/*
 *	vxusersync - Mass user synchronization
 *	Copyright © CC Computer Consultants GmbH, 2003 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxcli/libvxcli.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include <vitalnix/libvxmdsync/libvxmdsync.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>

/* Definitions */
#define NOTICE_REDO \
	"Error during operation. You might need to redo the " \
	"Synchronization process to complete all jobs.\n"

struct private_info {
	char *db_name, *group_name, *input_file, *input_fmt, *output_file;
	unsigned int debug, no_add, no_update, no_delete, yestoall;

	bool open_status;
	struct vxpdb_state *db_handle;
	struct mdsync_workspace *mdsw;
	time_t last_print;
};

/* Functions */
static bool sync_init(struct private_info *);
static bool sync_run(struct private_info *);
static bool sync_add(struct private_info *);
static bool sync_mod(struct private_info *);
static bool sync_del(struct private_info *);
static void sync_cleanup(struct private_info *);

static bool ask_continue(const struct private_info *, const char *);
static void cb_report(unsigned int, const struct mdsync_workspace *,
	unsigned int, unsigned int);
static void print_compare_input(const struct mdsync_workspace *);
static void print_compare_output(const struct mdsync_workspace *);
static void print_compare_output2(const struct mdsync_workspace *);

static bool get_options(int *, const char ***, struct private_info *);
static void show_version(const struct HXoptcb *);
static bool time_limit(time_t *, time_t);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct private_info priv;
	int ret = EXIT_SUCCESS;

	memset(&priv, 0, sizeof(priv));
	priv.db_name = HX_strdup("*");

	if (!get_options(&argc, &argv, &priv))
		return EXIT_FAILURE;
	if (!sync_init(&priv) || !sync_run(&priv))
		ret = EXIT_FAILURE;

	sync_cleanup(&priv);
	return ret;
}

static bool sync_init(struct private_info *priv)
{
	struct mdsync_workspace *mdsw;
	int ret;

	if (priv->group_name == NULL || priv->input_file == NULL ||
		priv->output_file == NULL) {
		fprintf(stderr, "-g, -i and -o options are required.\n");
		return false;
	}

	if (strcmp(priv->output_file, "-") == 0)
		HX_strclone(&priv->output_file, "/dev/stdout");

	if ((priv->db_handle = vxpdb_load(priv->db_name)) == NULL) {
		perror("Could not load database");
		return false;
	}

	if ((ret = vxpdb_open(priv->db_handle, PDB_WRLOCK)) <= 0) {
		fprintf(stderr, "Could not open database: %s\n",
		        strerror(-ret));
		return false;
	}

	priv->open_status = true;
	if ((priv->mdsw = mdsw = mdsync_init()) == NULL) {
		perror("Init procedure failed");
		return false;
	}

	mdsw->database     = priv->db_handle;
	mdsw->report       = cb_report;
	mdsw->user_private = priv;
	return true;
}

static bool sync_run(struct private_info *priv)
{
	struct mdsync_workspace *mdsw = priv->mdsw;
	const char *fmt;
	int ret;

	if ((ret = mdsync_prepare_group(mdsw, priv->group_name)) < 0) {
		fprintf(stderr, "Error querying database: %s\n",
		        strerror(-ret));
		return false;
	} else if (ret == 0) {
		fprintf(stderr, "Group \"%s\" does not exist\n",
		        priv->group_name);
		return false;
	}

	if ((fmt = priv->input_fmt) == NULL &&
	    (fmt = vxeds_derivefromname(priv->input_file)) == NULL) {
		fprintf(stderr, "Could not determine file type of input "
		        "data source\n");
		return false;
	}

	if ((ret = mdsync_read_file(mdsw, priv->input_file, fmt)) <= 0) {
		fprintf(stderr, "Error while reading Data Source: %s\n",
		        strerror(-ret));
		return false;
	}

	if ((ret = mdsync_open_log(mdsw, priv->output_file)) <= 0) {
		fprintf(stderr, "Error trying to open logfile: %s\n",
		        strerror(-ret));
		return false;
	}

	if (mdsw->config.add_opts.defaults.vs_defer > 0)
		printf("Deferred Deletion feature enabled (%ld day(s)).\n"
		       "Note that the deletion count therefore may be lower than\n"
		       "expected, which is normal. These users should instead show up\n"
		       "under Deferred Deletion Timer Start.\n",
		       mdsw->config.add_opts.defaults.vs_defer);

	print_compare_input(mdsw);
	mdsync_compare(mdsw);
	print_compare_output(mdsw);
	mdsync_fixup(mdsw);
	if (priv->debug) {
		print_compare_output2(mdsw);
		return true;
	}

	/* Note short-circuiting */
	return sync_add(priv) && sync_mod(priv) && sync_del(priv);
}

static bool sync_add(struct private_info *priv)
{
	int ret;
	if (priv->mdsw->add_req->items == 0) {
		printf("No new users to add.\n");
		return true;
	}
	if (priv->no_add) {
		printf("Not adding any users due to request (command-line).\n");
		return true;
	}
	if (!ask_continue(priv, "Continue with adding users?\n"))
		return true;
	if ((ret = mdsync_add(priv->mdsw)) <= 0) {
		printf("Add procedure failed: %s\n" NOTICE_REDO,
		       strerror(-ret));
		return false;
	}
	printf("Successfully added %u users\n", priv->mdsw->add_req->items);
	return true;
}

static bool sync_mod(struct private_info *priv)
{
	struct mdsync_workspace *mdsw = priv->mdsw;
	unsigned int total = mdsw->defer_start->items + mdsw->defer_stop->items;
	int ret;

	if (total == 0) {
		printf("No deferred deletion timers to adjust.\n");
		return true;
	}
	if (priv->no_update) {
		printf("Not modifying deferred deletion timers due to request"
		       " (command-line).\n");
		return true;
	}
	if (!ask_continue(priv, "Continue with modifying users?\n"))
		return true;
	if ((ret = mdsync_mod(priv->mdsw)) <= 0) {
		printf("Modify procedure failred: %s\n" NOTICE_REDO,
		       strerror(-ret));
		return false;
	}
	printf("Successfully modified %u timers\n", total);
	return true;
}

static bool sync_del(struct private_info *priv)
{
	int ret;

	if (priv->mdsw->delete_now->items == 0) {
		printf("No old users to delete.\n");
		return true;
	}
	if (priv->no_delete) {
		printf("Not deleting any users due to request (command-line).\n");
		return true;
	}
	if (!ask_continue(priv, "Continue with deleting users?\n"))
		return true;
	if ((ret = mdsync_del(priv->mdsw)) <= 0) {
		printf("Deletion procedure failed: %s\n" NOTICE_REDO,
		       strerror(-ret));
		return false;
	}
	printf("Successfully deleted %u users\n", priv->mdsw->delete_now->items);
	return true;
}

static void sync_cleanup(struct private_info *priv)
{
	if (priv->mdsw != NULL)
		mdsync_free(priv->mdsw);
	if (priv->db_handle != NULL) {
		if (priv->open_status)
			vxpdb_close(priv->db_handle);
		vxpdb_unload(priv->db_handle);
	}
	free(priv->db_name);
	free(priv->group_name);
	free(priv->input_file);
	free(priv->input_fmt);
	free(priv->output_file);
	return;
}

//-----------------------------------------------------------------------------
static bool ask_continue(const struct private_info *priv, const char *msg)
{
	char buf[4] = {};

	if (priv->yestoall || !isatty(STDIN_FILENO))
		return true;

	vxcli_query(msg, NULL, "yes", VXCQ_NONE, buf, sizeof(buf));
	return tolower(*buf) == 'y';
}

static void cb_report(unsigned int type, const struct mdsync_workspace *mdsw,
    unsigned int current, unsigned int max)
{
	static const char *const fmt[] = {
		[MDREP_ADD]     = "Add process",
		[MDREP_UPDATE]  = "Update process",
		[MDREP_DSTART]  = "Starting DDTs",
		[MDREP_DSTOP]   = "Stopping DDTs",
		[MDREP_DELETE]  = "Deletion process",
		[MDREP_COMPARE] = "Compare process",
		[MDREP_FIXUP]   = "Fixup process",
	};
	struct private_info *priv = mdsw->user_private;
	double pct;

	if (current == 1)
		priv->last_print = 0;
	if (!time_limit(&priv->last_print, 1) && current != max)
		return;

	printf("%s", fmt[type]);
	pct = current * 100.0 / max;
	if (type == MDREP_COMPARE || type == MDREP_FIXUP)
		printf(": %.2f%%\n", pct);
	else
		printf(": %u/%u users (%.2f%%)\n", current, max, pct);
	return;
}

static void print_compare_input(const struct mdsync_workspace *mdsw)
{
	printf("Comparing EDS to PDB\n");
	printf("%u user(s) in EDS list\n", mdsw->add_req->items);
	return;
}

static void print_compare_output(const struct mdsync_workspace *mdsw)
{
	printf(
		"%u group member(s) found in PDB\n"
		"	%u to keep and update group descriptors\n"
		"	%u to start deferred deletion timer\n"
		"	%u to wait for deletion\n"
		"	%u to stop deferred deletion timer\n"
		"	%u to delete\n"
		"%u new users to add\n",
		mdsw->num_grp,
		mdsw->update_req->items,
		mdsw->defer_start->items,
		mdsw->defer_wait->items,
		mdsw->defer_stop->items,
		mdsw->delete_now->items,
		mdsw->add_req->items);
	return;
}

static void print_compare_output2(const struct mdsync_workspace *mdsw)
{
	const struct HXbtree_node *b;
	const struct HXdeque_node *d;
	void *travp;

	if ((travp = HXbtrav_init(mdsw->add_req)) != NULL) {
		while ((b = HXbtraverse(travp)) != NULL) {
			const struct vxeds_entry *entry = b->data;
			printf("A   %s (%s)\n", entry->username,
			       entry->full_name);
		}
		HXbtrav_free(travp);
	}

	if ((travp = HXbtrav_init(mdsw->update_req)) != NULL) {
		while ((b = HXbtraverse(travp)) != NULL) {
			const struct vxpdb_user *user = b->data;
			printf("U   %s (%s)\n", user->pw_name, user->pw_real);
		}
		HXbtrav_free(travp);
	}

	for (d = mdsw->defer_start->first; d != NULL; d = d->next)
		printf("XA  %s\n", static_cast(const char *, d->ptr));
	for (d = mdsw->defer_wait->first; d != NULL; d = d->next)
		printf("XW  %s\n", static_cast(const char *, d->ptr));
	for (d = mdsw->defer_stop->first; d != NULL; d = d->next)
		printf("XS  %s\n", static_cast(const char *, d->ptr));
	for (d = mdsw->delete_now->first; d != NULL; d = d->next)
		printf("D   %s\n", static_cast(const char *, d->ptr));
	return;
}

//-----------------------------------------------------------------------------
static bool get_options(int *argc, const char ***argv, struct private_info *p)
{
	struct HXoption options_table[] = {
		{.sh = 'D', .type = HXTYPE_NONE, .ptr = &p->debug,
		 .help = "Enable some more debugging output"},
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &p->db_name,
		 .help = "Use sepcified database", .htyp = "name"},
		{.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
		 .help = "Show version information"},
		{.sh = 'Y', .type = HXTYPE_NONE, .ptr = &p->yestoall,
		 .help = "Assume yes on all questions"},
		{.sh = 'g', .type = HXTYPE_STRING, .ptr = &p->group_name,
		 .help = "System group to synchronize against", .htyp = "name"},
		{.sh = 'i', .type = HXTYPE_STRING, .ptr = &p->input_file,
		 .help = "External Data Source", .htyp = "file"},
		{.sh = 't', .type = HXTYPE_STRING, .ptr = &p->input_fmt,
		 .help = "EDS type", .htyp = "TYPE"},
		{.sh = 'o', .type = HXTYPE_STRING, .ptr = &p->output_file,
		 .help = "Output log file (for -S)", .htyp = "file"},
		{.ln = "no-add", .type = HXTYPE_NONE, .ptr = &p->no_add,
		 .help = "Do not add any users"},
		{.ln = "no-mod", .type = HXTYPE_NONE, .ptr = &p->no_update,
		 .help = "Do not update any existing users (e.g. vitalnixGroup and vitalnixDeferTimer field)"},
		{.ln = "no-del", .type = HXTYPE_NONE, .ptr = &p->no_delete,
		 .help = "Do not delete any users"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " mdsync\n");
	exit(EXIT_SUCCESS);
}

static bool time_limit(time_t *last, time_t interval)
{
	time_t now = time(NULL);
	if (now - *last < interval)
		return false;
	*last = now;
	return true;
}

//=============================================================================
