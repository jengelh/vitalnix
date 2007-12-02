/*
 *	mdfixuuid - Fix UUID of a user
 *	Copyright © CC Computer Consultants GmbH, 2006 - 2007
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
#include <libHX.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
struct mdf_priv {
	char *database, *bday, *realname, *username;
};

/* Functions */
static char *rebuild_uuid(const struct mdf_priv *, struct vxpdb_state *);
static bool get_options(int *, const char ***, struct mdf_priv *);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct vxpdb_user mod_mask;
	struct vxpdb_state *db;
	struct mdf_priv p = {};
	int ret;

	p.database = "*";
	if (!get_options(&argc, &argv, &p))
		return EXIT_FAILURE;
	if ((db = vxpdb_load(p.database)) == NULL) {
		fprintf(stderr, "Error loading database: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if ((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
		fprintf(stderr, "Error opening database: %s\n", strerror(-ret));
		return EXIT_FAILURE;
	}

	vxpdb_user_nomodify(&mod_mask);
	mod_mask.pw_real = p.realname;
	mod_mask.vs_uuid = rebuild_uuid(&p, db);
	if (mod_mask.vs_uuid == NULL)
		return EXIT_FAILURE;

	if ((ret = vxpdb_usermod(db, p.username, &mod_mask)) <= 0) {
		fprintf(stderr, "User update unsuccessful: %s\n",
		        strerror(-ret));
		return EXIT_FAILURE;
	}

	vxpdb_close(db);
	vxpdb_unload(db);
	return EXIT_SUCCESS;
}

static char *rebuild_uuid(const struct mdf_priv *p, struct vxpdb_state *db)
{
	struct vxpdb_user info = {};
	char *res = NULL, *name;
	int ret, xday = 0;

	if ((ret = vxpdb_getpwnam(db, p->username, &info)) < 0) {
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
	vxpdb_user_free(&info, false);
	return res;
}

//-----------------------------------------------------------------------------
static bool get_options(int *argc, const char ***argv, struct mdf_priv *p)
{
	struct HXoption options_table[] = {
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &p->database,
		 .help = "Use specified database", .htyp = "name"},
		{.sh = 'b', .type = HXTYPE_STRING, .ptr = &p->bday,
		 .help = "New birthdate", .htyp = "date"},
		{.sh = 'r', .type = HXTYPE_STRING, .ptr = &p->realname,
		 .help = "New realname"},
		{.sh = 'u', .type = HXTYPE_STRING, .ptr = &p->username,
		 .help = "Username to operate on"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	if (HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
		return false;
	if (p->username == NULL || (p->bday == NULL && p->realname == NULL)) {
		fprintf(stderr, "You need to specify the -u option and at "
		        "least one of -b or -r\n");
		return false;
	}
	return true;
}

//=============================================================================
