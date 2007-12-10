/*
 *	vxid - Show identification
 *	Copyright © CC Computer Consultants GmbH, 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/defines.h>

/* Functions */
static bool get_options(int *, const char ***);
static void id_current(struct vxdb_state *);
static void id(struct vxdb_state *, const char *);

/* Variables */
static char *Database = "*";

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct vxdb_state *db;
	int ret;

	if (!get_options(&argc, &argv))
		return EXIT_FAILURE;

	if ((db = vxdb_load(Database)) == NULL) {
		perror("Error loading database");
		return EXIT_FAILURE;
	}

	if ((ret = vxdb_open(db, 0)) <= 0) {
		vxdb_unload(db);
		fprintf(stderr, "Error opening database: %s\n",
		        strerror(-ret));
		return EXIT_FAILURE;
	}

	if (argc == 1)
		id_current(db);
	else
		while (*++argv != NULL)
			id(db, *argv);

	vxdb_close(db);
	vxdb_unload(db);
	return EXIT_SUCCESS;

}

static bool get_options(int *argc, const char ***argv)
{
	struct HXoption options_table[] = {
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &Database,
		 .help = "Use specified database", .htyp = "name"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void id_current(struct vxdb_state *db)
{
	unsigned int uid = getuid(), gid = getgid();
	unsigned int euid = geteuid(), egid = getegid();
	struct vxdb_user user = {};
	struct vxdb_group group = {};
	gid_t sgrp[128];
	unsigned int i;
	int ret, sgnum;

	printf("ruid=%u", uid);
	if ((ret = vxdb_getpwuid(db, uid, &user)) < 0)
		perror("- vxdb_getpwuid");
	else if (ret > 0)
		printf("(%s)", user.pw_name);

	printf(" rgid=%u", gid);
	if ((ret = vxdb_getgrgid(db, gid, &group)) < 0)
		perror("- vxdb_getgrgid");
	else if (ret > 0)
		printf("(%s)", group.gr_name);

	printf(" euid=%u", euid);
	if ((ret = vxdb_getpwuid(db, euid, &user)) < 0)
		perror("- vxdb_getpwuid");
	else if (ret > 0)
		printf("(%s)", user.pw_name);

	printf(" egid=%u", egid);
	if ((ret = vxdb_getgrgid(db, egid, &group)) < 0)
		perror("- vxdb_getgrgid");
	else if (ret > 0)
		printf("(%s)", group.gr_name);

	if ((sgnum = getgroups(ARRAY_SIZE(sgrp), sgrp)) < 0) {
		perror("- getgroups");
	} else {
		printf(" groups=");
		for (i = 0; i < sgnum; ++i) {
			printf("%u", sgrp[i]);
			if ((ret = vxdb_getgrgid(db, sgrp[i], &group)) < 0)
				perror("- vxdb_getgrgid");
			else if (ret > 0)
				printf("(%s)", group.gr_name);
			if (i != sgnum - 1)
				printf(",");
		}
	}

	printf("\n");
	vxdb_user_free(&user, false);
	vxdb_group_free(&group, false);
	return;
}

static void id(struct vxdb_state *db, const char *name)
{
	struct vxdb_group group = {};
	struct vxdb_user user = {};
	unsigned int uid;
	char *end;
	int ret;

	uid = strtoul(name, &end, 0);
	if (end != name && *end == '\0')
		ret = vxdb_getpwuid(db, uid, &user);
	else
		ret = vxdb_getpwnam(db, name, &user);

	if (ret < 0) {
		perror("- vxdb_getpw*");
	} else if (ret == 0) {
		printf("id: %s: no such user\n", name);
		return;
	}

	printf("uid=%u(%s) gid=%u", user.pw_uid, user.pw_name,
	       user.pw_gid);
	if ((ret = vxdb_getgrgid(db, user.pw_gid, &group)) < 0)
		perror("- vxdb_getgrgid");
	else if (ret > 0)
		printf("(%s)", group.gr_name);

	printf("\n");
	vxdb_user_free(&user, false);
	vxdb_group_free(&group, false);
	return;
}
