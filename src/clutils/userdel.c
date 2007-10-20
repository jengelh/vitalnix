/*
 *	userdel - User manipulation
 *	Copyright © CC Computer Consultants GmbH, 2003 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxpdb/config.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
enum {
	E_SUCCESS = 0,
	E_OTHER,
	E_OPEN,
	E_NO_EXIST,
	E_DENY,
	E_UPDATE,
	E_POST,
};

struct userdel_state {
	const char *username;
	struct vxconfig_userdel config;
	const char *database;
	unsigned int force, rm_cron, rm_home, rm_mail;
	struct HXbtree *sr_map;
};

/* Functions */
static int userdel_fill_defaults(struct userdel_state *);
static bool userdel_get_options(int *, const char ***, struct userdel_state *);
static void userdel_getopt_predel(const struct HXoptcb *);
static void userdel_getopt_postdel(const struct HXoptcb *);
static int userdel_read_config(struct userdel_state *);
static int userdel_run(struct userdel_state *);
static int userdel_run2(struct vxpdb_state *, struct userdel_state *);
static int userdel_run3(struct vxpdb_state *, struct userdel_state *);
static void userdel_show_version(const struct HXoptcb *);
static unsigned int userdel_slash_count(const char *);
static const char *userdel_strerror(int);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct userdel_state state;
	int ret;

	userdel_fill_defaults(&state);
	if (!userdel_get_options(&argc, &argv, &state))
		return E_OTHER;

	if (argc < 2) {
		fprintf(stderr, "You have to specify a username!\n");
		return E_OTHER;
	}

	state.username = argv[1];
	if ((ret = userdel_run(&state)) != E_SUCCESS)
		fprintf(stderr, "%s: %s\n", userdel_strerror(ret),
		    	strerror(errno));

	return (ret < 0) ? E_OTHER : ret;
}

static int userdel_fill_defaults(struct userdel_state *sp)
{
	int ret;

	memset(sp, 0, sizeof(struct userdel_state));
	sp->database = "*";

	if ((ret = userdel_read_config(sp)) <= 0)
		return ret;

	return 1;
}

static bool userdel_get_options(int *argc, const char ***argv,
    struct userdel_state *state)
{
	struct vxconfig_userdel *conf = &state->config;
	struct HXoption options_table[] = {
		/* New (vxuserdel) options */
		{.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .cb = userdel_getopt_postdel, .uptr = conf,
		 .help = "Program to run after user modification", .htyp = "cmd"},
		{.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .cb = userdel_getopt_predel, .uptr = conf,
		 .help = "Program to run before user modification", .htyp = "cmd"},
		{.sh = 'F', .type = HXTYPE_NONE, .ptr = &state->force,
		 .help = "Force deletion even if UID is 0 or name is 'root'"},
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &state->database,
		 .help = "Use specified database", .htyp = "name"},

		// Default options
		{.sh = 'r', .type = HXTYPE_NONE, .ptr = &state->rm_home,
		 .help = "Remove home directory, cron tables and mail spool"},
		{.sh = 'v', .ln = "version", .type = HXTYPE_NONE,
		 .cb = userdel_show_version, .help = "Show version information"},

		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static int userdel_run(struct userdel_state *state)
{
	struct vxpdb_state *db;
	int ret;
	if ((db = vxpdb_load(state->database)) == NULL)
		return E_OPEN;
	ret = userdel_run2(db, state);
	vxpdb_unload(db);
	return ret;
}

static const char *userdel_strerror(int e)
{
	switch (e) {
		case E_OTHER:
			return "Error";
		case E_OPEN:
			return "Could not load/open database";
		case E_NO_EXIST:
			return "User does not exist";
		case E_DENY:
			return "Refusing to remove \"root\" or UID 0 accounts when not using the force option";
		case E_UPDATE:
			return "Error deleting user";
		case E_POST:
			return "Error during post setup";
	}
	return NULL;
}

static void userdel_getopt_predel(const struct HXoptcb *cbi)
{
	struct vxconfig_userdel *conf = cbi->current->uptr;
	conf->master_predel = NULL;
	conf->user_predel   = HX_strdup(cbi->data);
	return;
}

static void userdel_getopt_postdel(const struct HXoptcb *cbi)
{
	struct vxconfig_userdel *conf = cbi->current->uptr;
	conf->master_postdel = NULL;
	conf->user_postdel   = HX_strdup(cbi->data);
	return;
}

static int userdel_read_config(struct userdel_state *state)
{
	int err, ret = 0;
	struct HXoption config_table[] = {
		{.ln = "REMOVE_CRON", .type = HXTYPE_BOOL, .ptr = &state->rm_cron},
		{.ln = "REMOVE_HOME", .type = HXTYPE_BOOL, .ptr = &state->rm_home},
		{.ln = "REMOVE_MAIL", .type = HXTYPE_BOOL, .ptr = &state->rm_mail},
		HXOPT_TABLEEND,
	};
	err = vxconfig_read_userdel(CONFIG_SYSCONFDIR "/userdel.conf",
		  &state->config);
	if (err < 0)
		ret = err;
	err = HX_shconfig(CONFIG_SYSCONFDIR "/userdel.conf", config_table);
	if (err < 0 && ret == 0)
		ret = err;
	return ret;
}

static int userdel_run2(struct vxpdb_state *db, struct userdel_state *state)
{
	int ret;
	if ((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0)
		return E_OPEN;
	state->sr_map = HXformat_init();
	ret = userdel_run3(db, state);
	HXformat_free(state->sr_map);
	vxpdb_close(db);
	return ret;
}

static int userdel_run3(struct vxpdb_state *db, struct userdel_state *state)
{
	struct vxconfig_userdel *conf = &state->config;
	struct vxpdb_user result = {};
	char *home, *username;
	int ret;

	if ((ret = vxpdb_getpwnam(db, state->username, &result)) < 0)
		return E_OTHER;
	else if (ret == 0)
		return E_OTHER;

	if (!state->force && (strcmp(result.pw_name, "root") == 0 ||
	    result.pw_uid == 0))
		return E_DENY;

	if (conf->master_predel != NULL)
		vxutil_replace_run(conf->master_predel, state->sr_map);
	if (conf->user_predel != NULL)
		vxutil_replace_run(conf->user_predel, state->sr_map);

	username = HX_strdup(result.pw_name);
	home     = HX_strdup(result.pw_home);

	if ((ret = vxpdb_userdel(db, username)) <= 0) {
		free(username);
		free(home);
		return E_UPDATE;
	}

	if (state->rm_home) {
		if (home == NULL || *home == '\0')
			fprintf(stderr, "Warning: User had no home directory. "
			        "Not removing anything.\n");
		else if (strcmp(home, "/") == 0)
			fprintf(stderr, "Warning: Will refuse to delete "
			        "home directory \"/\"\n");
		else if (userdel_slash_count(home) <= 1 && !state->force)
			fprintf(stderr, "Warning: Will not remove home "
			        "directory \"%s\" which has less than two "
			        "slashes and thus looks like a system or a "
			        "malformed directory. (Remove it manually.)\n",
			        home);
		else
			HX_rrmdir(home);
	}
	if (state->rm_cron) {
		char buf[MAXFNLEN];
		snprintf(buf, sizeof(buf), "crontab -r \"%s\"", username);
		system(buf);
	}
	if (state->rm_mail) {
		char buf[MAXFNLEN];
		snprintf(buf, sizeof(buf), "/var/spool/mail/%s", username);
		unlink(buf);
	}

	if (conf->user_postdel != NULL)
		vxutil_replace_run(conf->user_postdel, state->sr_map);
	if (conf->master_postdel != NULL)
		vxutil_replace_run(conf->master_postdel, state->sr_map);

	free(home);
	free(username);
	return E_SUCCESS;
}

static void userdel_show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " userdel\n");
	return;
}

static unsigned int userdel_slash_count(const char *fn)
{
	const char *ptr = fn;
	int n = 0;

	if (fn == NULL)
		return 0;

	while ((ptr = strchr(ptr, '/')) != NULL) {
		++n;
		++ptr;
	}

	return n;
}

//=============================================================================
