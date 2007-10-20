/*
 *	usermod - User manipulation
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
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxpdb/config.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
enum {
	E_SUCCESS = 0,
	E_OTHER,
	E_OPEN,
	E_NO_EXIST,
	E_UID_USED,
	E_NAME_USED,
	E_UPDATE,
	E_POST,
};

struct usermod_state {
	char *username;
	struct vxpdb_user newstuff;
	struct vxconfig_usermod config;
	const char *database;
	unsigned int allow_dup, lock_account, move_home;
	struct HXbtree *sr_map;
};

/* Functions */
static int usermod_fill_defaults(struct usermod_state *);
static bool usermod_get_options(int *, const char ***, struct usermod_state *);
static void usermod_getopt_expire(const struct HXoptcb *);
static void usermod_getopt_premod(const struct HXoptcb *);
static void usermod_getopt_postmod(const struct HXoptcb *);
static int usermod_read_config(struct usermod_state *);
static int usermod_run(struct usermod_state *);
static int usermod_run2(struct vxpdb_state *, struct usermod_state *);
static int usermod_run3(struct vxpdb_state *, struct usermod_state *);
static void usermod_show_version(const struct HXoptcb *);
static const char *usermod_strerror(int);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct usermod_state state;
	int ret;

	usermod_fill_defaults(&state);
	if (!usermod_get_options(&argc, &argv, &state))
		return E_OTHER;

	if (argc < 2) {
		fprintf(stderr, "You need to specify a username!\n");
		return E_OTHER;
	}

	if ((ret = usermod_run(&state)) != E_SUCCESS)
		fprintf(stderr, "%s: %s\n", usermod_strerror(ret),
		        strerror(errno));

	return (ret < 0) ? E_OTHER : ret;
}

static int usermod_fill_defaults(struct usermod_state *sp)
{
	struct vxpdb_user *u = &sp->newstuff;
	int ret;

	vxpdb_user_nomodify(u);
	sp->database = "*";
	if ((ret = usermod_read_config(sp)) <= 0)
		return ret;

	return ret;
}

static bool usermod_get_options(int *argc, const char ***argv,
    struct usermod_state *sp)
{
	struct vxconfig_usermod *conf = &sp->config;
	struct vxpdb_user *nu = &sp->newstuff;
	struct HXoption options_table[] = {
		/* New, Vitalnix-usermod options */
		{.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .cb = usermod_getopt_premod, .uptr = conf,
		 .help = "Program to run after user modification", .htyp = "cmd"},
		{.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .cb = usermod_getopt_postmod, .uptr = conf,
		 .help = "Program to run before user modification", .htyp = "cmd"},
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &sp->database,
		 .help = "Use specified database", .htyp = "name"},

		/* Default options */
		{.sh = 'G', .type = HXTYPE_STRING, .ptr = &nu->pw_sgrp,
		 .help = "The new supplementary groups for the user", .htyp = "groups"},
		{.sh = 'L', .type = HXTYPE_VAL, .ptr = &sp->lock_account, .val = 1,
		 .help = "Lock user account"},
		{.sh = 'U', .type = HXTYPE_VAL, .ptr = &sp->lock_account, .val = 2,
		 .help = "Unlock user account"},
		{.sh = 'c', .type = HXTYPE_STRING, .ptr = &nu->pw_real,
		 .help = "New real name", .htyp = "name"},
		{.sh = 'd', .type = HXTYPE_STRING, .ptr = &nu->pw_home,
		 .help = "New home directory (implies home automove)", .htyp = "dir"},
		{.sh = 'e', .type = HXTYPE_STRING, .cb = usermod_getopt_expire,
		 .help = "Date when the account expires", .htyp = "date"},
		{.sh = 'f', .type = HXTYPE_INT, .ptr = &nu->sp_inact,
		 .help = "Days until account becomes inactive", .htyp = "days"},
		{.sh = 'g', .type = HXTYPE_STRING, .ptr = &nu->pw_igrp,
		 .help = "The new primary group for the user", .htyp = "group"},
		{.sh = 'l', .type = HXTYPE_STRING, .ptr = &nu->pw_name,
		 .help = "New login name of the user"},
		{.sh = 'm', .type = HXTYPE_NONE, .ptr = &sp->move_home,
		 .help = "Move home directory if -d is given"},
		{.sh = 'o', .type = HXTYPE_NONE, .ptr = &sp->allow_dup,
		 .help = "Allow creating a user with non-unique UID"},
		{.sh = 'p', .type = HXTYPE_STRING, .ptr = &nu->sp_passwd,
		 .help = "New password to use (encrypted form)"},
		{.sh = 's', .type = HXTYPE_STRING, .ptr = &nu->pw_shell,
		 .help = "New default shell for user", .htyp = "file"},
		{.sh = 'u', .type = HXTYPE_UINT, .ptr = &nu->pw_uid,
		 .help = "New numerical value of the user's ID", .htyp = "uid"},
		{.sh = 'v', .ln = "version", .type = HXTYPE_NONE,
		 .cb = usermod_show_version, .help = "Show version information"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	if (HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
		return false;

	if (argv[1] == NULL) {
		/* loginname is mandatory */
		fprintf(stderr, "You need to specify a username\n");
		return false;
	}

	return true;
}

static int usermod_run(struct usermod_state *state)
{
	struct vxpdb_state *db;
	int ret;

	if ((db = vxpdb_load(state->database)) == NULL)
		return E_OPEN;

	ret = usermod_run2(db, state);
	vxpdb_unload(db);
	return ret;
}

static const char *usermod_strerror(int e)
{
	switch (e) {
		case E_OTHER:
			return "Error";
		case E_OPEN:
			return "Could not load/open database";
		case E_NAME_USED:
			return "User already exists";
		case E_UID_USED:
			return "UID already exists";
		case E_UPDATE:
			return "Error adding user";
	}
	return NULL;
}

static void usermod_getopt_expire(const struct HXoptcb *cbi)
{
	struct vxpdb_user *user = cbi->current->ptr;
	user->sp_expire = vxutil_string_iday(cbi->data);
	return;
}

static void usermod_getopt_premod(const struct HXoptcb *cbi)
{
	struct vxconfig_usermod *conf = cbi->current->uptr;
	conf->master_premod = NULL;
	conf->user_premod   = HX_strdup(cbi->data);
	return;
}

static void usermod_getopt_postmod(const struct HXoptcb *cbi)
{
	struct vxconfig_usermod *conf = cbi->current->uptr;
	conf->master_postmod = NULL;
	conf->user_postmod   = HX_strdup(cbi->data);
	return;
}

static int usermod_read_config(struct usermod_state *sp)
{
	int err, ret = 0;
	struct HXoption config_table[] = {
		{.ln = "MOVE_HOME", .type = HXTYPE_BOOL, .ptr = &sp->move_home},
		HXOPT_TABLEEND,
	};
	err = vxconfig_read_usermod(CONFIG_SYSCONFDIR "/usermod.conf", &sp->config);
	if (err < 0)
		ret = err;
	err = HX_shconfig(CONFIG_SYSCONFDIR "/usermod.conf", config_table);
	if (err < 0 && ret == 0)
		ret = err;
	return ret;
}

static int usermod_run2(struct vxpdb_state *db, struct usermod_state *state)
{
	int ret;
	if ((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0)
		return E_OPEN;

	state->sr_map = HXformat_init();
	ret = usermod_run3(db, state);
	HXformat_free(state->sr_map);
	vxpdb_close(db);
	return ret;
}

static int usermod_run3(struct vxpdb_state *db, struct usermod_state *state)
{
	struct vxconfig_usermod *conf = &state->config;
	struct vxpdb_user modify_mask = {};
	int ret;

	if ((ret = vxpdb_getpwnam(db, state->username, NULL)) < 0)
		return E_OTHER;
	else if (ret == 0)
		return E_NO_EXIST;

	if (conf->master_premod != NULL)
		vxutil_replace_run(conf->master_premod, state->sr_map);
	if (conf->user_premod != NULL)
		vxutil_replace_run(conf->user_premod, state->sr_map);

	if ((ret = vxpdb_usermod(db, state->username, &modify_mask)) <= 0)
		return E_UPDATE;

	if (conf->user_postmod != NULL)
		vxutil_replace_run(conf->user_postmod, state->sr_map);
	if (conf->master_postmod != NULL)
		vxutil_replace_run(conf->master_postmod, state->sr_map);

	return E_SUCCESS;
}

static void usermod_show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " usermod\n");
	return;
}

//=============================================================================
