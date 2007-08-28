/*
 *	useradd - User manipulation
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2003 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxpdb/config.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
enum {
	E_SUCCESS = 0,
	E_OTHER,
	E_OPEN,
	E_UID_USED,
	E_NAME_USED,
	E_UPDATE,
	E_POST,
};

struct useradd_state {
	struct vxconfig_useradd config;
	const char *database;
	unsigned int allow_dup, force, sys_uid;
	struct HXbtree *sr_map;
};

/* Functions */
static int useradd_fill_defaults(struct useradd_state *);
static char *useradd_genhome(struct useradd_state *);
static bool useradd_get_options(int *, const char ***, struct useradd_state *);
static void useradd_getopt_expire(const struct HXoptcb *);
static void useradd_getopt_preadd(const struct HXoptcb *);
static void useradd_getopt_postadd(const struct HXoptcb *);
static int useradd_read_config(struct useradd_state *);
static int useradd_run(struct useradd_state *);
static int useradd_run2(struct vxpdb_state *, struct useradd_state *);
static int useradd_run3(struct vxpdb_state *, struct useradd_state *,
	struct vxpdb_user *);
static void useradd_show_version(const struct HXoptcb *);
static const char *useradd_strerror(int);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct useradd_state state;
	struct vxpdb_user *user = &state.config.defaults;
	int ret;

	useradd_fill_defaults(&state);
	if (!useradd_get_options(&argc, &argv, &state))
		return E_OTHER;

	if (argc > 1) {
		HX_strclone(&user->pw_name, argv[1]);
		if (user->pw_home == NULL)
			user->pw_home = useradd_genhome(&state);
	}

	if (state.config.defaults.pw_name == NULL) {
		fprintf(stderr, "You have to specify a username!\n");
		return E_OTHER;
	}

	if ((ret = useradd_run(&state)) != E_SUCCESS)
		fprintf(stderr, "%s: %s\n", useradd_strerror(ret),
		    	strerror(errno));

	return (ret < 0) ? E_OTHER : ret;
}

/*
 * useradd_fill_defaults -
 * @state:	pointer to useradd state
 *
 * Fills @state with the hardcoded defaults and with the defaults from
 * configuration file.
 */
static int useradd_fill_defaults(struct useradd_state *state)
{
	struct vxconfig_useradd *conf = &state->config;
	struct vxpdb_user *user	   = &conf->defaults;
	int ret;

	memset(state, 0, sizeof(struct useradd_state));
	vxpdb_user_clean(user);
	user->pw_shell    = "/bin/bash";
	user->sp_lastchg  = vxutil_now_iday();
	state->database   = "*";
	conf->create_home = 1;
	conf->skel_dir    = "/var/lib/empty";
	conf->home_base   = "/home";
	conf->umask       = S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	if ((ret = useradd_read_config(state)) <= 0)
		return ret;

	return 1;
}

/*
 * useradd_genhome -
 *
 * Generate a home directory path based upon the split level that was set in
 * the configuration file.
 */
static char *useradd_genhome(struct useradd_state *state)
{
	struct vxconfig_useradd *conf = &state->config;
	struct vxpdb_user *user       = &conf->defaults;
	char buf[MAXFNLEN];

	if (user->pw_name == NULL || conf->home_base == NULL)
		return NULL;

	return HX_strdup(vxutil_propose_home(buf, sizeof(buf), conf->home_base,
		   user->pw_name, conf->split_level));
}

static bool useradd_get_options(int *argc, const char ***argv,
    struct useradd_state *state)
{
	struct vxconfig_useradd *conf = &state->config;
	struct vxpdb_user *user	   = &conf->defaults;
	struct HXoption options_table[] = {
		/* New, Vitalnix-useradd options */
		{.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .cb = useradd_getopt_postadd, .uptr = conf,
		 .help = "Program to run after user addition", .htyp = "cmd"},
		{.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .cb = useradd_getopt_preadd, .uptr = conf,
		 .help = "Program to run before user addition", .htyp = "cmd"},
		{.sh = 'F', .type = HXTYPE_NONE, .ptr = &state->force,
		 .help = "Force usage of dangerous umask"},
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &state->database,
		 .help = "Use specified database", .htyp = "name"},
		{.sh = 'S', .type = HXTYPE_INT, .ptr = &conf->split_level,
		 .help = "Use split home feature (specify split level)"},

		/* Default options */
		{.sh = 'G', .type = HXTYPE_STRING, .ptr = &user->pw_sgrp,
		 .help = "Supplementary groups", .htyp = "groups"},
		{.sh = 'c', .type = HXTYPE_STRING, .ptr = &user->pw_real,
		 .help = "User's comment in the password file"},
		{.sh = 'd', .type = HXTYPE_STRING, .ptr = &user->pw_home,
		 .help = "Home directory of the user (overrides -S)", .htyp = "dir"},
		{.sh = 'e', .type = HXTYPE_STRING, .cb = useradd_getopt_expire,
		 .ptr = user, .help = "Date when the account expires", .htyp = "date"},
		{.sh = 'f', .type = HXTYPE_INT, .ptr = &user->sp_inact,
		 .help = "Days until account becomes inactive", .htyp = "days"},
		{.sh = 'g', .type = HXTYPE_STRING, .ptr = &user->pw_igrp,
		 .help = "Initial group of the user", .htyp = "group"},
		{.sh = 'k', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .ptr = &conf->skel_dir, .help = "Skeleton directory", .htyp = "dir"},
		{.sh = 'm', .type = HXTYPE_NONE, .ptr = &conf->create_home,
		 .help = "Create home directory"},
		{.sh = 'o', .type = HXTYPE_NONE, .ptr = &state->allow_dup,
		 .help = "Allow creating a user with non-unique UID"},
		{.sh = 'p', .type = HXTYPE_STRING, .ptr = &user->sp_passwd,
		 .help = "Encrypted password to use"},
		{.sh = 'r', .type = HXTYPE_NONE, .ptr = &state->sys_uid,
		 .help = "System account (use userid < UID_MIN)"},
		{.sh = 's', .type = HXTYPE_STRING, .ptr = &user->pw_shell,
		 .help = "The user's default shell", .htyp = "prog"},
		{.sh = 'u', .type = HXTYPE_LONG, .ptr = &user->pw_uid,
		 .help = "Numerical value of the user's ID", .htyp = "uid"},
		{.sh = 'v', .ln = "version", .type = HXTYPE_NONE,
		 .cb = useradd_show_version, .help = "Show version information"},

		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	if (HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
		return false;
	if (conf->split_level > 2)
		conf->split_level = 2;
	return true;
}

static int useradd_run(struct useradd_state *state)
{
	struct vxpdb_state *db;
	int ret;

	if ((db = vxpdb_load(state->database)) == NULL)
		return E_OPEN;

	ret = useradd_run2(db, state);
	vxpdb_unload(db);
	return ret;
}

static const char *useradd_strerror(int e)
{
	switch (e) {
		case E_OTHER:
			return "Error";
		case E_OPEN:
			return "Could not load/open database";
		case E_UID_USED:
			return "UID already exists";
		case E_NAME_USED:
			return "User already exists";
		case E_UPDATE:
			return "Error adding user";
		case E_POST:
			return "Error during post setup";
	}
	return NULL;
}

static void useradd_getopt_expire(const struct HXoptcb *cbi)
{
	struct vxpdb_user *user = cbi->current->ptr;
	user->sp_expire = vxutil_string_iday(cbi->data);
	return;
}

static void useradd_getopt_preadd(const struct HXoptcb *cbi)
{
	struct vxconfig_useradd *conf = cbi->current->uptr;
	conf->master_preadd = NULL;
	conf->user_preadd   = HX_strdup(cbi->data);
	return;
}

static void useradd_getopt_postadd(const struct HXoptcb *cbi)
{
	struct vxconfig_useradd *conf = cbi->current->uptr;
	conf->master_postadd = NULL;
	conf->user_postadd   = HX_strdup(cbi->data);
	return;
}

static int useradd_read_config(struct useradd_state *state)
{
	int ret = vxconfig_read_useradd(CONFIG_SYSCONFDIR "/useradd.conf",
	          &state->config);
	unsigned int mask = state->config.umask;

	if ((mask & (S_IWGRP | S_IWOTH)) != (S_IWGRP | S_IWOTH) &&
		!state->force) {
		/* complain if not all write permission bits are cleared */
		fprintf(stderr, "Error: will refuse to allow foreign "
		    	"write-access for user directory,\n"
		        "use -F to override.\n");
		exit(E_OTHER);
	} else if ((mask & (S_IRGRP | S_IROTH)) != (S_IRGRP | S_IROTH) &&
		!state->force) {
		/* warn if not all read permission bits are cleared */
		fprintf(stderr, "Warning: other users will possibly be able "
		        "to spy on the home directory\n");
	}

	return ret;
}

static int useradd_run2(struct vxpdb_state *db, struct useradd_state *state)
{
	struct vxconfig_useradd *conf = &state->config;
	struct vxpdb_user *user;
	int ret;

	if ((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0)
		return E_OPEN;

	user = HX_memdup(&conf->defaults, sizeof(conf->defaults));
	if (user == NULL)
		return -ENOMEM;

	state->sr_map = HXformat_init();
	ret = useradd_run3(db, state, user);
	HXformat_free(state->sr_map);
	free(user);
	vxpdb_close(db);
	return ret;
}

static int useradd_run3(struct vxpdb_state *db, struct useradd_state *state,
    struct vxpdb_user *user)
{
	struct vxconfig_useradd *conf = &state->config;
	int ret;

	if ((ret = vxpdb_getpwnam(db, user->pw_name, NULL)) < 0)
		return E_OTHER;
	else if (ret > 0)
		return E_NAME_USED;

	if (user->pw_uid != PDB_NOUID) {
		/* -u is provided */
		if (!state->allow_dup &&
		    vxpdb_getpwuid(db, user->pw_uid, NULL) > 0)
			/*
			 * The -o flag (allow creating user with duplicate UID)
			 * was not passed.
			 */
			return E_UID_USED;
	} else if (state->sys_uid) {
		/* -r flag passed */
		user->pw_uid = PDB_AUTOUID_SYS;
	} else {
		user->pw_uid = PDB_AUTOUID;
	}

	user->sp_lastchg = vxutil_now_iday();

	if (vxutil_only_digits(user->pw_igrp)) {
		user->pw_gid  = strtoul(user->pw_igrp, NULL, 0);
		user->pw_igrp = NULL;
	} else {
		user->pw_gid  = PDB_NOGID;
	}

	HXformat_add(state->sr_map, "USERNAME", user->pw_name, HXTYPE_STRING);
	HXformat_add(state->sr_map, "UID", &user->pw_uid, HXTYPE_LONG);

	if (conf->master_preadd != NULL)
		vxutil_replace_run(conf->master_preadd, state->sr_map);

	if ((ret = vxpdb_useradd(db, user)) <= 0)
		return E_UPDATE;

	if (conf->create_home) {
		if (HX_mkdir(user->pw_home) <= 0) {
			/*
			fprintf(stderr, "Warning: Could not create home "
					"directory %s\n", strerror(errno));
			*/
			return E_POST;
		}

		lchown(user->pw_home, user->pw_uid, user->pw_gid);
		chmod(user->pw_home, 0755 & ~conf->umask);

		if (conf->skel_dir != NULL && *conf->skel_dir != '\0')
			HX_copy_dir(conf->skel_dir, user->pw_home,
			            HXF_UID | HXF_GID | HXF_KEEP,
			            user->pw_uid, user->pw_gid);
	}

	if (conf->master_postadd != NULL)
		vxutil_replace_run(conf->master_postadd, state->sr_map);

	return E_SUCCESS;
}

static void useradd_show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " useradd\n");
	return;
}

//=============================================================================
