/*
 *	groupadd - Group manipulation
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
	/* unable to open module database or driver */
	E_OPEN,
	/* GID already used and -o was not specified */
	E_GID_USED,
	/* group already exists */
	E_NAME_USED,
	/* db->groupadd() did not return ok */
	E_UPDATE,
};

/* Functions */
static int groupadd_main2(struct vxpdb_state *);
static int groupadd_main3(struct vxpdb_state *, struct HXbtree *);
static bool groupadd_get_options(int *, const char ***);
static bool groupadd_read_config(void);
static void groupadd_show_version(const struct HXoptcb *);

/* Variables */
static long group_id             = PDB_NOGID;
static unsigned int allow_dup    = false;
static unsigned int request_sys  = false;
static const char *action_before = NULL;
static const char *action_after  = NULL;
static const char *database_name = "*";
static const char *group_name;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct vxpdb_state *db;
	int ret;

	if (!groupadd_read_config() ||
	    !groupadd_get_options(&argc, &argv))
		return E_OTHER;

	group_name = argv[1];
	if (!vxutil_valid_username(group_name)) {
		fprintf(stderr, "\"%s\" is not a valid group name\n",
		        group_name);
		return E_OTHER;
	}

	if ((db = vxpdb_load(database_name)) == NULL) {
		perror("Error loading database");
		return E_OPEN;
	}

	ret = groupadd_main2(db);
	vxpdb_unload(db);
	return ret;
}

static int groupadd_main2(struct vxpdb_state *db)
{
	struct HXbtree *ext_catalog;
	int ret;

	if ((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
		fprintf(stderr, "Error opening database: %s\n",
		        strerror(-ret));
		return E_OPEN;
	}

	ext_catalog = HXformat_init();
	ret = groupadd_main3(db, ext_catalog);
	HXformat_free(ext_catalog);
	vxpdb_close(db);
	return ret;
}

static int groupadd_main3(struct vxpdb_state *db, struct HXbtree *ext_catalog)
{
	struct vxpdb_group group_info;
	int ret;

	if ((ret = vxpdb_getgrnam(db, group_name, NULL)) < 0) {
		fprintf(stderr, "Error querying database: %s\n",
		        strerror(-ret));
		return E_OTHER;
	} else if (ret > 0) {
		fprintf(stderr, "Group \"%s\" already exists\n", group_name);
		return E_NAME_USED;
	}

	if (group_id != PDB_NOGID) {
		/* -g (explicit GID) was passed */
		if (!allow_dup && vxpdb_getgrgid(db, group_id, NULL) > 0) {
			/*
			 * The -o flag (allow creating group with duplicate
			 * GID) was not passed.
			 */
			fprintf(stderr, "Group with GID %ld already exists."
			        " Use -o to override.\n", group_id);
			return E_GID_USED;
		}
	} else if (request_sys) {
		/* -r flag passed */
		group_id = PDB_AUTOGID_SYS;
	} else {
		group_id = PDB_AUTOGID;
	}

	HXformat_add(ext_catalog, "GROUP", group_name, HXTYPE_STRING);
	HXformat_add(ext_catalog, "GID", &group_id, HXTYPE_LONG);

	if (action_before != NULL)
		vxutil_replace_run(action_before, ext_catalog);

	group_info.gr_name = const_cast(char *, group_name);
	group_info.gr_gid  = group_id;

	if ((ret = vxpdb_groupadd(db, &group_info)) <= 0) {
		fprintf(stderr, "Warning: Group addition failed: %s\n",
		        strerror(-ret));
		return E_UPDATE;
	} else if (action_after != NULL) {
		vxutil_replace_run(action_after, ext_catalog);
	}

	return E_SUCCESS;
}

//-----------------------------------------------------------------------------
static bool groupadd_get_options(int *argc, const char ***argv)
{
	static const struct HXoption options_table[] = {
		/* New, Vitalnix-groupadd options */
		{.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .ptr = &action_after, .htyp = "cmd",
		 .help = "Program to run after group addition"},
		{.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .ptr = &action_before, .htyp = "cmd",
		 .help = "Program to run before group addition"},
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &database_name,
		 .help = "Use specified database", .htyp = "name"},

		/* Default options */
		{.sh = 'g', .type = HXTYPE_LONG, .ptr = &group_id,
		 .help = "Numerical value of the group's ID", .htyp = "gid"},
		{.sh = 'o', .type = HXTYPE_NONE, .ptr = &allow_dup,
		 .help = "Allow creating a group with non-unique GID"},
		{.sh = 'r', .type = HXTYPE_NONE, .ptr = &request_sys,
		 .help = "System group (use groupid < GID_MIN)"},
		{.sh = 'v', .ln = "version", .type = HXTYPE_NONE,
		 .cb = groupadd_show_version,
		 .help = "Show version information"},

		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	if (HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
		return false;
	if (argv[1] == NULL) {
		/* Group name is mandatory */
		fprintf(stderr, "Error: Need to specify a group name\n");
		return false;
	}

	return true;
}

static bool groupadd_read_config(void)
{
	static const struct HXoption config_table[] = {
		{.ln = "GROUP_PREADD",  .type = HXTYPE_STRING, .ptr = &action_before},
		{.ln = "GROUP_POSTADD", .type = HXTYPE_STRING, .ptr = &action_after},
		HXOPT_TABLEEND,
	};
	return HX_shconfig(CONFIG_SYSCONFDIR "/groupadd.conf", config_table) > 0;
}

static void groupadd_show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " groupadd\n");
	return;
}

//=============================================================================
