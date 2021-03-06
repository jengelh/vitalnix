/*
 *	fixuuid - Fix UUID of a user
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2011
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
#include <libHX/init.h>
#include <libHX/option.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
struct mdf_priv {
	char *database, *bday, *realname, *username;
};

/* Functions */
static char *rebuild_uuid(const struct mdf_priv *, struct vxdb_state *);
static bool get_options(int *, const char ***, struct mdf_priv *);

//-----------------------------------------------------------------------------
static int main2(int argc, const char **argv)
{
	struct vxdb_user mod_mask;
	struct vxdb_state *db;
	struct mdf_priv p = {};
	int ret;

	p.database = "*";
	if (!get_options(&argc, &argv, &p))
		return EXIT_FAILURE;
	if ((db = vxdb_load(p.database)) == NULL) {
		fprintf(stderr, "Error loading database: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if ((ret = vxdb_open(db, VXDB_WRLOCK)) <= 0) {
		fprintf(stderr, "Error opening database: %s\n", strerror(-ret));
		return EXIT_FAILURE;
	}

	vxdb_user_nomodify(&mod_mask);
	mod_mask.pw_real = p.realname;
	mod_mask.vs_uuid = rebuild_uuid(&p, db);
	if (mod_mask.vs_uuid == NULL)
		return EXIT_FAILURE;

	if ((ret = vxdb_usermod(db, p.username, &mod_mask)) <= 0) {
		fprintf(stderr, "User update unsuccessful: %s\n",
		        strerror(-ret));
		return EXIT_FAILURE;
	}

	vxdb_close(db);
	vxdb_unload(db);
	return EXIT_SUCCESS;
}

int main(int argc, const char **argv)
{
	int ret;

	if ((ret = HX_init()) <= 0)
		abort();
	ret = main2(argc, argv);
	HX_exit();
	return ret;
}

static char *rebuild_uuid(const struct mdf_priv *p, struct vxdb_state *db)
{
	struct vxdb_user info = {};
	char *res = NULL, *name;
	int ret, xday = 0;

	if ((ret = vxdb_getpwnam(db, p->username, &info)) < 0) {
		fprintf(stderr, "Error querying database: %s\n",
		        strerror(-ret));
		return NULL;
	} else if (ret == 0) {
		fprintf(stderr, "User \"%s\" not found\n", p->username);
		return NULL;
	}

	if (p->bday != NULL) {
		if ((xday = vxutil_string_xday(p->bday)) == -1) {
			fprintf(stderr, "Invalid date \"%s\"\n", p->bday);
			goto out;
		}
	} else if (info.vs_uuid != NULL) {
		xday = vxuuid_vx3_get_xday(info.vs_uuid);
		if (xday == -1)
			xday = 0;
	}

	if ((name = p->realname) == NULL)
		name = info.pw_real;
	if (name != NULL)
		res = vxuuid_vx3(name, xday);

 out:
	vxdb_user_free(&info, false);
	return res;
}

//-----------------------------------------------------------------------------
static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " fixuuid\n");
	exit(EXIT_SUCCESS);
}

static bool get_options(int *argc, const char ***argv, struct mdf_priv *p)
{
	struct HXoption options_table[] = {
		{.ln = "vxdb", .type = HXTYPE_STRING, .ptr = &p->database,
		 .help = "Use specified database", .htyp = "name"},
		{.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
		 .help = "Show version information"},
		{.sh = 'b', .type = HXTYPE_STRING, .ptr = &p->bday,
		 .help = "New birthdate", .htyp = "date"},
		{.sh = 'r', .type = HXTYPE_STRING, .ptr = &p->realname,
		 .help = "New realname"},
		{.sh = 'u', .type = HXTYPE_STRING, .ptr = &p->username,
		 .help = "Username to operate on"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	if (HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) !=
	    HXOPT_ERR_SUCCESS)
		return false;
	if (p->username == NULL || (p->bday == NULL && p->realname == NULL)) {
		fprintf(stderr, "You need to specify the -u option and at "
		        "least one of -b or -r\n");
		return false;
	}
	return true;
}
