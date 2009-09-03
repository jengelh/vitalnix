/*
 *	groupdel - Group manipulation
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2009
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
#include <libHX/option.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/libvxutil.h>

enum {
	E_SUCCESS = 0,
	/* other error, see errno */
	E_OTHER,
	/* unable to open database or driver */
	E_OPEN,
	/* group does not exist */
	E_NOEXIST,
	/* db->groupdel() did not return ok */
	E_UPDATE,
	/* db->close() did not return ok */
	E_CLOSE,
	/* will not delete a user's primary group, unless -F is given */
	E_PRIMARY,
};

/* Functions */
static int groupdel_main2(struct vxdb_state *);
static int groupdel_main3(struct vxdb_state *, struct HXformat_map *);
static bool groupdel_check_pri_group(struct vxdb_state *, struct vxdb_group *);
static bool groupdel_get_options(int *, const char ***);
static bool groupdel_read_config(void);
static void groupdel_show_version(const struct HXoptcb *);

/* Variables */
static unsigned int force_deletion = false;
static const char *action_before   = NULL;
static const char *action_after    = NULL;
static const char *database_name   = "*";
static const char *group_name;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct vxdb_state *db;
	int ret;

	if (!groupdel_read_config() || !groupdel_get_options(&argc, &argv))
		return E_OTHER;

	if (argc < 2 || argv[1] == NULL) {
		/* Group name is mandatory */
		fprintf(stderr, "Error: Need to specify a group name\n");
		return false;
	}
	group_name = argv[1];
	if (!vxutil_valid_username(group_name)) {
		fprintf(stderr, "\"%s\" is not a valid group name\n",
		        group_name);
		return E_OTHER;
	}

	if ((db = vxdb_load(database_name)) == NULL) {
		fprintf(stderr, "Could not load database \"%s\": %s\n",
		        database_name, strerror(errno));
		return E_OPEN;
	}

	ret = groupdel_main2(db);
	vxdb_unload(db);
	return ret;
}

static int groupdel_main2(struct vxdb_state *db)
{
	struct HXformat_map *ext_catalog;
	int ret;

	if ((ret = vxdb_open(db, VXDB_WRLOCK)) <= 0) {
		fprintf(stderr, "Could not open database: %s\n",
		        strerror(-ret));
		return E_OPEN;
	}

	ext_catalog = HXformat_init();
	ret = groupdel_main3(db, ext_catalog);
	HXformat_free(ext_catalog);
	vxdb_close(db);
	return ret;
}

static int groupdel_main3(struct vxdb_state *db,
    struct HXformat_map *ext_catalog)
{
	struct vxdb_group group_info = {};
	int ret;

	if ((ret = vxdb_getgrnam(db, group_name, &group_info)) < 0) {
		fprintf(stderr, "Error querying database: %s\n",
		        strerror(-ret));
		return E_OTHER;
	} else if (ret == 0) {
		fprintf(stderr, "Group \"%s\" does not exist\n", group_name);
		return E_NOEXIST;
	}

	if (!force_deletion && groupdel_check_pri_group(db, &group_info)) {
		fprintf(stderr, "Will not remove a user's primary group\n");
		return E_PRIMARY;
	}

	HXformat_add(ext_catalog, "GROUP", group_name, HXTYPE_STRING);

	if (action_before != NULL)
		vxutil_replace_run(action_before, ext_catalog);

	if ((ret = vxdb_groupdel(db, group_name)) <= 0) {
		fprintf(stderr, "Error: Deleting group failed: %s\n",
		        strerror(-ret));
		return E_UPDATE;
	} else if (action_after != NULL) {
		vxutil_replace_run(action_after, ext_catalog);
	}

	return E_SUCCESS;
}

//-----------------------------------------------------------------------------
static bool groupdel_check_pri_group(struct vxdb_state *db,
    struct vxdb_group *mm)
{
	struct vxdb_user user;
	bool pg = false; /* some user has this as primary group */
	void *travp;

	if ((travp = vxdb_usertrav_init(db)) == NULL)
		return false;

	while (vxdb_usertrav_walk(db, travp, &user) > 0)
		if ((user.pw_gid != VXDB_NOGID && user.pw_gid == mm->gr_gid) ||
		    (user.pw_igrp != NULL && mm->gr_name != NULL &&
		    strcmp(user.pw_igrp, mm->gr_name) == 0)) {
			pg = true;
			break;
		}

	vxdb_usertrav_free(db, travp);
	return pg;
}

static bool groupdel_get_options(int *argc, const char ***argv)
{
	static const struct HXoption options_table[] = {
		/* New, Vitalnix-userdel options */
		{.ln = "after", .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .ptr = &action_after, .htyp = "cmd",
		 .help = "Program to run after group deletion"},
		{.ln = "before", .type = HXTYPE_STRING | HXOPT_OPTIONAL,
		 .ptr = &action_before, .htyp = "cmd",
		 .help = "Program to run before group deletion"},
		{.ln = "vxdb", .type = HXTYPE_STRING, .ptr = &database_name,
		 .help = "Use specified database", .htyp = "name"},
		{.sh = 'F', .type = HXTYPE_NONE, .ptr = &force_deletion,
		 .help = "Force deletion of group even if users have it "
		 "as primary group"},

		/* Default options */
		{.sh = 'v', .ln = "version", .type = HXTYPE_NONE,
		 .cb = groupdel_show_version,
		 .help = "Show version information"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static bool groupdel_read_config(void)
{
	static const struct HXoption config_table[] = {
		{.ln = "GROUP_PREDEL", .type = HXTYPE_STRING, .ptr = &action_before},
		{.ln = "GROUP_PREDEL", .type = HXTYPE_STRING, .ptr = &action_after},
		HXOPT_TABLEEND,
	};
	return HX_shconfig(CONFIG_SYSCONFDIR "/groupdel.conf", config_table) > 0;
}

static void groupdel_show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " groupdel\n");
	exit(EXIT_SUCCESS);
}
