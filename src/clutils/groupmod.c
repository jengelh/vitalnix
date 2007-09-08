/*
 *	groupmod - Group manipulation
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2003 - 2007
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
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include <vitalnix/libvxutil/libvxutil.h>

enum {
	E_SUCCESS = 0,
	/* other error, see errno */
	E_OTHER,
	/* unable to open database or driver */
	E_OPEN,
	/* group does not exist */
	E_NOEXIST,
	/* GID already used and -o was not specified */
	E_GID_USED,
	/* new group name (-n) already exists */
	E_NAME_USED,
	/* db->groupmod() did not return ok */
	E_UPDATE,
	/* db->close() did not return ok */
	E_CLOSE,
};

/* Functions */
static int groupmod_main2(struct vxpdb_state *);
static int groupmod_main3(struct vxpdb_state *, struct HXbtree *);
static bool groupmod_get_options(int *, const char ***);
static bool groupmod_read_config(void);
static void groupmod_show_version(const struct HXoptcb *);

/* Variables */
static unsigned int allow_dup     = false;
static const char *new_group_name = NULL;
static unsigned int new_group_id  = PDB_NOGID;
static const char *action_before  = NULL;
static const char *action_after   = NULL;
static const char *database_name  = "*";
static const char *group_name;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct vxpdb_state *db;
	int ret;

	if (!groupmod_read_config() || !groupmod_get_options(&argc, &argv))
		return E_OTHER;

	if (new_group_name != NULL && !vxutil_valid_username(new_group_name)) {
		fprintf(stderr, "\"%s\" is not a valid group name\n",
		        new_group_name);
		return E_OTHER;
	}

	if ((db = vxpdb_load(database_name)) == NULL) {
		fprintf(stderr, "Could not load database \"%s\": %s\n",
		        database_name, strerror(errno));
		return E_OPEN;
	}

	ret = groupmod_main2(db);
	vxpdb_unload(db);
	return ret;
}

static int groupmod_main2(struct vxpdb_state *db)
{
	struct HXbtree *ext_catalog;
	int ret;

	if ((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
		fprintf(stderr, "Could not open database: %s\n",
		        strerror(-ret));
		return E_OPEN;
	}

	ext_catalog = HXformat_init();
	ret = groupmod_main3(db, ext_catalog);
	HXformat_free(ext_catalog);
	vxpdb_close(db);
	return ret;
}

static int groupmod_main3(struct vxpdb_state *db, struct HXbtree *ext_catalog)
{
	struct vxpdb_group current = {}, mod_request;
	int ret;

	if ((ret = vxpdb_getgrnam(db, group_name, &current)) < 0) {
		fprintf(stderr, "Error querying database: %s\n",
		        strerror(-ret));
		return E_OTHER;
	} else if (ret == 0) {
		fprintf(stderr, "Group \"%s\" does not exist\n", group_name);
		return E_NOEXIST;
	}

	if (new_group_id != PDB_NOGID && new_group_id != current.gr_gid &&
	    !allow_dup && vxpdb_getgrgid(db, new_group_id, NULL) > 0) {
		/* If GID has changed */
		fprintf(stderr, "A group with GID %u already exists."
		        " Use -o to override.\n", new_group_id);
		return E_GID_USED;
	}

	if (new_group_name != NULL &&
	    strcmp(current.gr_name, new_group_name) != 0 &&
	    vxpdb_getgrnam(db, new_group_name, NULL) > 0) {
		/* If name has changed */
		fprintf(stderr, "A group with that name (\"%s\") already "
		        "exists.\n", new_group_name);
		return E_NAME_USED;
	}

	HXformat_add(ext_catalog, "GROUP", new_group_name, HXTYPE_STRING);
	HXformat_add(ext_catalog, "OLD_GROUP", group_name, HXTYPE_STRING);
	HXformat_add(ext_catalog, "GID", &new_group_id, HXTYPE_UINT);

	if (action_before != NULL)
		vxutil_replace_run(action_before, ext_catalog);

	mod_request.gr_name = static_cast(char *, new_group_name);
	mod_request.gr_gid  = new_group_id;

	if ((ret = vxpdb_groupmod(db, group_name, &mod_request)) <= 0) {
		fprintf(stderr, "Error: Group updating failed: %s\n",
		        strerror(-ret));
		return E_UPDATE;
	} else if (action_after != NULL) {
		vxutil_replace_run(action_after, ext_catalog);
	}

	return E_SUCCESS;
}

//-----------------------------------------------------------------------------
static bool groupmod_get_options(int *argc, const char ***argv)
{
	static const struct HXoption options_table[] = {
		/* New, Vitalnix-groupmod options */
		{.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .ptr = &action_after, .htyp = "cmd",
		 .help = "Program to run after group modification"},
		{.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .ptr = &action_before, .htyp = "cmd",
		 .help = "Program to run before group modification"},
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &database_name,
		 .help = "Use specified database", .htyp = "name"},

		/* Default options */
		{.sh = 'g', .type = HXTYPE_UINT, .ptr = &new_group_id,
		 .htyp = "gid", .help = "Numerical value of the group's "
		 "ID (DB module might ignore this)"},
		{.sh = 'n', .type = HXTYPE_STRING, .ptr = &new_group_name,
		 .help = "New name of the group"},
		{.sh = 'o', .type = HXTYPE_NONE, .ptr = &allow_dup,
		 .help = "Allow creating a group with non-unique GID"
		 " (might be disabled by the DB driver)"},
		{.sh = 'v', .ln = "version", .type = HXTYPE_NONE,
		 .cb = groupmod_show_version,
		 .help = "Show version information"},

		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	if (HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
		return false;
	if (argv[1] == NULL) {
		/* Group name is mandatory */
		fprintf(stderr, "You need to specify a group name\n");
		return false;
	}

	return true;
}

static bool groupmod_read_config(void)
{
	static const struct HXoption config_table[] = {
		{.ln = "AC_BEFORE", .type = HXTYPE_STRING, .ptr = &action_before},
		{.ln = "AC_AFTER",  .type = HXTYPE_STRING, .ptr = &action_after},
		HXOPT_TABLEEND,
	};
	return HX_shconfig(CONFIG_SYSCONFDIR "/groupmod.conf", config_table) > 0;
}

static void groupmod_show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " groupmod\n");
	return;
}

//=============================================================================
