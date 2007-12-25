/*
 *	groupbld - Build system groups from Vitalnix group tags
 *	Copyright © CC Computer Consultants GmbH, 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Functions */
static bool groupbld_get_options(int *, const char ***);
static void groupbld_show_version(const struct HXoptcb *);
static unsigned int groupbld_select_group(struct vxdb_state *, const char *);
static bool groupbld_loop(struct vxdb_state *, unsigned int);
static bool groupbld_create(struct vxdb_state *, const char *);
static void groupbld_transform(const char *, hmc_t **);
static bool groupbld_move(struct vxdb_state *, const char *, const char *);

/* Variables */
static char *Limit_group      = NULL;
static unsigned int Limit_gid = VXDB_NOGID;
static unsigned int Verbose   = true;
static char *Database         = "*";
static char *vg_prefix        = "vg-";
static unsigned int vg_prefix_len;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct vxdb_state *db;
	int ret;

	if (!groupbld_get_options(&argc, &argv))
		return EXIT_FAILURE;
	if (!vxutil_valid_username(vg_prefix)) {
		fprintf(stderr, "%s: \"%s\" is not a valid prefix\n",
		        *argv, vg_prefix);
		return EXIT_FAILURE;
	}

	vg_prefix_len = strlen(vg_prefix);

	if ((db = vxdb_load(Database)) == NULL) {
		perror("Error loading database");
		return EXIT_FAILURE;
	}

	if ((ret = vxdb_open(db, VXDB_WRLOCK)) <= 0) {
		vxdb_unload(db);
		fprintf(stderr, "Error opening database: %s\n",
		        strerror(-ret));
		return EXIT_FAILURE;
	}

	if (Limit_group != NULL) {
		Limit_gid = groupbld_select_group(db, Limit_group);
		if (Limit_gid == VXDB_NOGID)
			return EXIT_FAILURE;
	}
		

	ret = groupbld_loop(db, Limit_gid) ? EXIT_SUCCESS : EXIT_FAILURE;
	vxdb_close(db);
	vxdb_unload(db);
	return ret;
}

static bool groupbld_get_options(int *argc, const char ***argv)
{
	struct HXoption options_table[] = {
		{.sh = 'G', .type = HXTYPE_STRING, .ptr = &Limit_group,
		 .help = "Limit operation to specified group", .htyp = "group"},
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &Database,
		 .help = "Use specified database", .htyp = "name"},
		{.sh = 'V', .type = HXTYPE_NONE, .cb = groupbld_show_version,
		 .help = "Show version information"},
		{.sh = 'p', .type = HXTYPE_STRING, .ptr = &vg_prefix,
		 .help = "Prefix to use for vitalnix groups", .htyp = "string"},
		{.sh = 'q', .type = HXTYPE_VAL, .ptr = &Verbose, .val = false,
		 .help = "Quiet operation (only show errors)"},
		{.sh = 'v', .type = HXTYPE_VAL, .ptr = &Verbose, .val = true,
		 .help = "Verbose operation"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void groupbld_show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " groupbld\n");
	exit(EXIT_SUCCESS);
}

/*
 * groupbld_select_group - Transform group name into GID
 * @db:		db handle
 * @name:	group name
 *
 * Lookup group by name and return its GID. If the name is already a number,
 * return it. When the group is not found or on error, %VXDB_NOGID is returned.
 */
static unsigned int groupbld_select_group(struct vxdb_state *db,
    const char *name)
{
	struct vxdb_group group = {};
	unsigned int gid;
	char *end;
	int ret;

	gid = strtoul(name, &end, 0);
	if (end != name && *end == '\0')
		return gid;

	if ((ret = vxdb_getgrnam(db, name, &group)) < 0) {
		perror("vxdb_getgrnam");
		return VXDB_NOGID;
	} else if (ret == 0) {
		fprintf(stderr, "Group \"%s\" not found\n", name);
		return VXDB_NOGID;
	}

	gid = group.gr_gid;
	vxdb_group_free(&group, false);
	return gid;
}

static inline bool groupbld_timebolt(void)
{
	static time_t last = 0;
	time_t now = time(NULL);

	if (now <= last)
		return false;

	last = now;
	return true;
}

/*
 * groupbld_loop - traverse over all users and update VUG membership
 */
static bool groupbld_loop(struct vxdb_state *db, unsigned int gid)
{
	struct vxdb_user user = {};
	unsigned int iuser    = 0;
	hmc_t *vgname         = NULL;
	bool success          = true;
	void *trav;
	int nusers;

	if ((nusers = vxdb_modctl(db, VXDB_COUNT_USERS)) < 0)
		nusers = 0;

	if ((trav = vxdb_usertrav_init(db)) == NULL) {
		fprintf(stderr, "vxdb_usertrav_init: %s\n", strerror(errno));
		return false;
	}

	while (vxdb_usertrav_walk(db, trav, &user) > 0) {
		++iuser;
		if (user.vs_pvgrp == NULL || *user.vs_pvgrp == '\0')
			continue;
		if (gid != VXDB_NOGID && user.pw_gid != gid)
			continue;

		if (Verbose && groupbld_timebolt())
			printf("Status: %u/%d\n", iuser, nusers);

		groupbld_transform(user.vs_pvgrp, &vgname);
		if (!groupbld_create(db, vgname) ||
		    !groupbld_move(db, user.pw_name, vgname)) {
			success = false;
			break;
		}
	}

	if (Verbose)
		printf("Done.\n");
	hmc_free(vgname);
	vxdb_user_free(&user, false);
	vxdb_usertrav_free(db, trav);
	return success;
}

/*
 * groupbld_transform -
 * @name:	input name
 * @unix_name:	pointer for result
 *
 * Transform a Vitalnix Group name into a valid UNIX group name.
 */
static void groupbld_transform(const char *name, hmc_t **unix_name)
{
	unsigned int i;
	char buf[32], *p;

	hmc_strasg(unix_name, vg_prefix);

	/* Only call hmc_strcat() in chunks */
	while (*name != '\0') {
		for (i = 0, p = buf; i < sizeof(buf) - 1; ++i, ++name) {
			if (*name == '\0')
				break;
			else if (isalnum(*name))
				*p++ = *name;
			else if (*buf == '\0' || *(p-1) != '-')
				*p++ = '-';
		}
		*p = '\0';
		if (*buf != '\0' && *(p-1) == '-')
			*(p-1) = '\0';
		hmc_strcat(unix_name, buf);
	}

	return;
}

/*
 * groupbld_create - create VUG if it does not exist already
 * @db:		db handle
 * @name:	name of UNIX group
 */
static bool groupbld_create(struct vxdb_state *db, const char *name)
{
	struct vxdb_group group = {};
	int ret;

	if ((ret = vxdb_getgrnam(db, name, &group)) < 0) {
		fprintf(stderr, "vxdb_getgrnam: %s\n", strerror(-ret));
		return false;
	}

	if (ret > 0) {
		vxdb_group_free(&group, false);
		return true;
	}

	group.gr_name = const_cast(char *, name);
	group.gr_gid  = VXDB_AUTOGID;
	if ((ret = vxdb_groupadd(db, &group)) <= 0) {
		perror("vxdb_groupadd");
		return false;
	}

	if (Verbose)
		fprintf(stderr, "Created group %s\n", name);
	return true;
}

static bool groupbld_move(struct vxdb_state *db, const char *user,
    const char *vgname)
{
	bool added = false, success = true;
	unsigned int ngroups;
	char **sgrp, **p;
	int ret;

	if ((ret = vxdb_sgmapget(db, user, &sgrp)) < 0) {
		fprintf(stderr, "Could not retrieve sgrp list for %s: %s\n",
		        user, strerror(-ret));
		return false;
	}

	if (ret > 0) {
		for (p = sgrp; ngroups > 0 && *p != NULL; ++p) {
			if (strcmp(*p, vgname) == 0) {
				added = true;
				continue;
			}
			if (strncmp(*p, vg_prefix, vg_prefix_len) != 0)
				continue;
			if ((ret = vxdb_sgmapdel(db, user, *p)) <= 0) {
				fprintf(stderr, "Deleting mapping (user=%s, "
				        "group=%s) failed: %s\n",
				        user, *p, strerror(-ret));
				success = false;
				break;
			}
		}
		HX_zvecfree(sgrp);
	}

	if (success && !added)
		if ((ret = vxdb_sgmapadd(db, user, vgname)) <= 0) {
			fprintf(stderr, "Adding mapping (user=%s, group=%s) "
			        "failed: %s\n", user, vgname, strerror(-ret));
			success = false;
		}

	return success;
}
