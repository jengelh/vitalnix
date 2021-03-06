/*
 *	dbdump - Dump database
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2004 - 2011
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/init.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
enum output_type_e {
	OUTPUT_SHADOW,
	OUTPUT_LDIF,
	OUTPUT_MYSQL,
};

enum dump_what_e {
	DUMP_PASSWD,
	DUMP_SHADOW,
	DUMP_VXSHADOW,
	DUMP_GROUP,
};

/* Functions */
static void d_ldif(struct vxdb_state *);
static void d_ldif_users(struct vxdb_state *);
static void d_ldif_groups(struct vxdb_state *);
static void d_mysql(struct vxdb_state *);
static void d_mysql_users(struct vxdb_state *);
static void d_mysql_groups(struct vxdb_state *);
static void d_shadow(struct vxdb_state *);

static bool get_options(int *, const char ***);
static void getopt_t(const struct HXoptcb *);
static void getopt_u(const struct HXoptcb *);
static void getopt_w(const struct HXoptcb *);
static bool ldif_safe(const char *);
static void show_version(const struct HXoptcb *);

/* Variables */
static enum output_type_e Output_type = OUTPUT_SHADOW;
static unsigned int Superuser_mode;

static char *Database    = "*";
static long Uid_range[2] = {0, LONG_MAX};
static bool Dump_what[4]  = {true, true, true, true};
static void (*Dump_functions[])(struct vxdb_state *) = {
	[OUTPUT_LDIF]   = d_ldif,
	[OUTPUT_MYSQL]  = d_mysql,
	[OUTPUT_SHADOW] = d_shadow,
};

//-----------------------------------------------------------------------------
static int main2(int argc, const char **argv)
{
	struct vxdb_state *db;
	int ret;

	if (!get_options(&argc, &argv))
		return EXIT_FAILURE;

	if ((db = vxdb_load(Database)) == NULL) {
		perror("Error loading database");
		return EXIT_FAILURE;
	}
	if ((ret = vxdb_open(db, Superuser_mode ? VXDB_ADMIN : 0)) <= 0) {
		vxdb_unload(db);
		fprintf(stderr, "Error opening database: %s\n",
		        strerror(-ret));
		return EXIT_FAILURE;
	}

	/* Walk users */
	Dump_functions[Output_type](db);
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

//-----------------------------------------------------------------------------
static void d_ldif(struct vxdb_state *db)
{
	if (Dump_what[DUMP_PASSWD])
		d_ldif_users(db);
	if (Dump_what[DUMP_GROUP])
		d_ldif_groups(db);
}

static void d_ldif_groups(struct vxdb_state *db)
{
	struct vxdb_group group = {};
	void *trav;

	if ((trav = vxdb_grouptrav_init(db)) == NULL) {
		fprintf(stderr, "# vxdb_grouptrav_init: %s\n",
		        strerror(errno));
		return;
	}

	while (vxdb_grouptrav_walk(db, trav, &group) > 0) {
		printf(
			"dn: cn=%s,ou=groups,dc=site\n"
			"objectClass: posixGroup\n"
			"cn: %s\n"
			"gidNumber: %u\n\n",
			group.gr_name, group.gr_name, group.gr_gid);
	}

	vxdb_grouptrav_free(db, trav);
	vxdb_group_free(&group, false);
}

static void d_ldif_users(struct vxdb_state *db) 
{
	struct vxdb_user user = {};
	char *freeme = NULL;
	void *trav;

	if ((trav = vxdb_usertrav_init(db)) == NULL) {
		fprintf(stderr, "# vxdb_usertrav_init: %s\n",
		        strerror(errno));
		return;
	}

	while (vxdb_usertrav_walk(db, trav, &user) > 0) {
		if (!(user.pw_uid >= Uid_range[0] &&
		    user.pw_uid <= Uid_range[1]))
			continue;

		/* defined in RFC 2307 and nis.schema */
		printf(
			"dn: uid=%s,ou=users,dc=site\n"
			"objectClass: account\n"
			"objectClass: posixAccount\n"
			"uid: %s\n"
			"uidNumber: %u\n"
			"gidNumber: %u\n",
			user.pw_name, user.pw_name, user.pw_uid, user.pw_gid);

		if (user.pw_real == NULL || *user.pw_real == '\0') {
			printf("cn: %s\n", user.pw_name);
		} else if (ldif_safe(user.pw_real)) {
			printf("cn: %s\n" "gecos: %s\n",
			       user.pw_real, user.pw_real);
		} else {
			char *text = vxutil_quote(user.pw_real,
			             VXQUOTE_BASE64, &freeme);
			printf("cn:: %s\n" "gecos:: %s\n", text, text);
		}

		if (user.pw_home == NULL || *user.pw_home == '\0')
			printf("homeDirectory:\n");
		else if (ldif_safe(user.pw_home))
			printf("homeDirectory: %s\n", user.pw_home);
		else
			printf("homeDirectory:: %s\n",
			       vxutil_quote(user.pw_home, VXQUOTE_BASE64,
			       &freeme));

		if (user.pw_shell != NULL && *user.pw_shell != '\0') {
			if (ldif_safe(user.pw_shell))
				printf("loginShell: %s\n", user.pw_shell);
			else
				printf("loginShell:: %s\n",
				       vxutil_quote(user.pw_shell,
				       VXQUOTE_BASE64, &freeme));
		}

		if (user.sp_passwd != NULL && *user.sp_passwd != '\0')
			printf("userPassword: {crypt}%s\n",
			       user.sp_passwd);

		if (Dump_what[DUMP_SHADOW]) {
			printf("objectClass: shadowAccount\n");
			if (user.sp_lastchg > 0)
				printf("shadowLastChange: %ld\n",
				       user.sp_lastchg);
			if (user.sp_min > 0)
				printf("shadowMin: %ld\n", user.sp_min);
			if (user.sp_max > 0)
				printf("shadowMax: %ld\n", user.sp_max);
			if (user.sp_warn > 0)
				printf("shadowWarning: %ld\n", user.sp_warn);
			if (user.sp_expire > 0)
				printf("shadowExpire: %ld\n", user.sp_expire);
			if (user.sp_inact > 0)
				printf("shadowInactive: %ld\n", user.sp_inact);
		}

		if (Dump_what[DUMP_VXSHADOW]) {
			printf("objectClass: vitalnixManagedAccount\n");
			if (user.vs_uuid != NULL)
				printf("vitalnixUUID: %s\n", user.vs_uuid);
			if (user.vs_pvgrp != NULL)
				printf("vitalnixGroup: %s\n", user.vs_pvgrp);
			if (user.vs_defer != 0)
				printf("vitalnixDeferTimer: %u\n", user.vs_defer);
		}

		printf("\n");
	}

	vxdb_usertrav_free(db, trav);
	vxdb_user_free(&user, false);
	free(freeme);
}

static void d_mysql(struct vxdb_state *db)
{
	printf(
		"drop table if exists users;\n"
		"drop table if exists shadow;\n"
		"drop table if exists vxshadow;\n"
		"drop table if exists groups;\n"
		"drop table if exists groupmap;\n"

		"create table users (\n"
		"	username varchar(64) not null default '',\n"
		"	uid int(11) not null default %d,\n"
		"	gid int(11) not null default %d,\n"
		"	realname varchar(255) not null default '',\n"
		"	homedir varchar(255) not null default '',\n"
		"	shell varchar(255) not null default '/bin/bash',\n"
		"	unused0 varchar(255) not null default 'x',\n"
		"	primary key (username),\n"
		"	index uid (uid)\n"
		") default charset=utf8;\n"

		"create table shadow (\n"
		"	username varchar(64) not null default '',\n"
		"	password varchar(255) not null default '!',\n"
		"	lastchange int(11) not null default 0,\n"
		"	minkeep int(11) not null default %d,\n"
		"	maxkeep int(11) not null default %d,\n"
		"	warnage int(11) not null default %d,\n"
		"	expire int(11) not null default %d,\n"
		"	inactive int(11) not null default %d,\n"
		"	primary key (username)\n"
		") default charset=utf8;\n"

		"create table vxshadow (\n"
		"	username varchar(64) not null default '',\n"
		"	uuid varchar(255) not null default '',\n"
		"	pvgrp varchar(64) not null default '',\n"
		"	defer int(11) not null default 0,\n"
		"	primary key (username)\n"
		") default charset=utf8;\n"

		"create table groups (\n"
		"	group_name varchar(64) not null default '',\n"
		"	gid int(11) not null default %d,\n"
		"	group_password varchar(255) not null default '*',\n"
		"	primary key (group_name),\n"
		"	index gid (gid)\n"
		") default charset=utf8;\n"

		"create table groupmap (\n"
		"	user_name varchar(64) not null default '',\n"
		"	group_name varchar(64) not null default ''\n"
		") default charset=utf8;\n",
		VXDB_NOUID, VXDB_NOGID, VXDB_DFL_KEEPMIN, VXDB_DFL_KEEPMAX,
		VXDB_DFL_WARNAGE, VXDB_NO_EXPIRE, VXDB_NO_INACTIVE, VXDB_NOGID
	);
	printf("lock tables users write, shadow write, vxshadow write, "
	       "groups write, groupmap write;\n");
	d_mysql_users(db);
	if (Dump_what[DUMP_GROUP])
		d_mysql_groups(db);
	printf("unlock tables;\n");
}

static void d_mysql_users(struct vxdb_state *db)
{
	struct vxdb_user user;
	void *trav;

	if ((trav = vxdb_usertrav_init(db)) == NULL)
		return;
	memset(&user, 0, sizeof(user));
	while (vxdb_usertrav_walk(db, trav, &user) > 0) {
		if (!(user.pw_uid >= Uid_range[0] &&
		    user.pw_uid <= Uid_range[1]))
			continue;
		if (Dump_what[DUMP_PASSWD])
			printf("insert into users (username, uid, gid, "
			       "realname, homedir, shell) values "
			       "('%s',%u,%u,'%s','%s','%s');\n",
			       user.pw_name, user.pw_uid, user.pw_gid,
			       user.pw_real, user.pw_home, user.pw_shell);
		if (Dump_what[DUMP_SHADOW] && user.sp_passwd != NULL)
			printf("insert into shadow values "
			       "('%s','%s',%ld,%ld,%ld,%ld,%ld,%ld);\n",
			       user.pw_name, user.sp_passwd, user.sp_lastchg,
			       user.sp_min, user.sp_max, user.sp_warn,
			       user.sp_expire, user.sp_inact);
		if (Dump_what[DUMP_VXSHADOW] && (user.vs_uuid != NULL ||
			user.vs_pvgrp != NULL || user.vs_defer != 0))
			printf("insert into vxshadow values "
			       "('%s','%s','%s',%u);\n",
			       user.pw_name, user.vs_uuid, user.vs_pvgrp,
			       user.vs_defer);
	}

	vxdb_usertrav_free(db, trav);
	vxdb_user_free(&user, false);
	printf("# Number of users: %ld\n", vxdb_modctl(db, VXDB_COUNT_USERS));
}

static void d_mysql_groups(struct vxdb_state *db)
{
	struct vxdb_group group;
	void *trav;

	if ((trav = vxdb_grouptrav_init(db)) == NULL)
		return;
	memset(&group, 0, sizeof(group));
	while (vxdb_grouptrav_walk(db, trav, &group) > 0)
		printf("insert into groups values ('%s',%u);\n",
		       group.gr_name, group.gr_gid);

	vxdb_grouptrav_free(db, trav);
	vxdb_group_free(&group, false);
	printf("# Number of groups: %ld\n", vxdb_modctl(db, VXDB_COUNT_GROUPS));
}

static void d_shadow(struct vxdb_state *db)
{
	struct vxdb_group group = {};
	struct vxdb_user user   = {};
	void *trav;

	/* Users */
	if (Dump_what[DUMP_PASSWD]) {
		printf("#---/etc/passwd---\n");
		if ((trav = vxdb_usertrav_init(db)) == NULL)
			return;
		while (vxdb_usertrav_walk(db, trav, &user) > 0) {
			if (!(user.pw_uid >= Uid_range[0] &&
			    user.pw_uid <= Uid_range[1]))
				continue;
			printf("%s:x:%u:%u:%s:%s:%s\n", user.pw_name,
			       user.pw_uid, user.pw_gid, user.pw_real,
			       user.pw_home, user.pw_shell);
		}
		vxdb_usertrav_free(db, trav);
	}

	/* Shadow */
	if (Dump_what[DUMP_SHADOW]) {
		printf("#---/etc/shadow---\n");
		if ((trav = vxdb_usertrav_init(db)) == NULL)
			return;
		while (vxdb_usertrav_walk(db, trav, &user) > 0) {
			if (!(user.pw_uid >= Uid_range[0] &&
			    user.pw_uid <= Uid_range[1]))
				continue;
			printf("%s:%s:%ld:%ld:%ld:%ld:%ld:%ld\n",
			       user.pw_name,
			       (user.sp_passwd == NULL) ?
			       "*" : user.sp_passwd,
			       user.sp_lastchg, user.sp_min,
			       user.sp_max, user.sp_warn,
			       user.sp_expire, user.sp_inact);
		}
		vxdb_usertrav_free(db, trav);
	}

	/* vxShadow */
	if (Dump_what[DUMP_VXSHADOW]) {
		printf("#---/etc/vxshadow---\n");
		if ((trav = vxdb_usertrav_init(db)) == NULL)
			return;
		while (vxdb_usertrav_walk(db, trav, &user) > 0) {
			if (!(user.pw_uid >= Uid_range[0] &&
			    user.pw_uid <= Uid_range[1]))
				continue;
			if (user.vs_uuid != NULL || user.vs_pvgrp != NULL ||
			    user.vs_defer != 0)
				printf("%s:%s:%s:%u\n", user.pw_name,
				       user.vs_uuid, user.vs_pvgrp,
				       user.vs_defer);
		}
		vxdb_usertrav_free(db, trav);
	}

	/* Group */
	if (Dump_what[DUMP_GROUP]) {
		printf("#---/etc/group---\n");
		if ((trav = vxdb_grouptrav_init(db)) == NULL)
			return;
		while (vxdb_grouptrav_walk(db, trav, &group) > 0)
			printf("%s:*:%u:\n", group.gr_name, group.gr_gid);
		vxdb_grouptrav_free(db, trav);
	}

	vxdb_user_free(&user, false);
	vxdb_group_free(&group, false);

	/* Info */
	printf("#---INFO---\n");
	printf("# Number of users: %ld\n", vxdb_modctl(db, VXDB_COUNT_USERS));
	printf("# Number of groups: %ld\n", vxdb_modctl(db, VXDB_COUNT_GROUPS));
}

//-----------------------------------------------------------------------------
static bool get_options(int *argc, const char ***argv)
{
	struct HXoption options_table[] = {
		{.ln = "vxdb", .type = HXTYPE_STRING, .ptr = &Database,
		 .help = "Use specified database", .htyp = "name"},
		{.sh = 'S', .type = HXTYPE_NONE, .ptr = &Superuser_mode,
		 .help = "Login with root DN"},
		{.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
		 .help = "Show version information"},
		{.sh = 't', .type = HXTYPE_STRING, .cb = getopt_t,
		 .help = "Output format (shadow, mysql, ldif)",
		 .htyp = "string"},
		{.sh = 'u', .type = HXTYPE_STRING, .cb = getopt_u,
		 .help = "Specify UID range to include (from:to)",
		 .htyp = "range"},
		{.sh = 'w', .type = HXTYPE_STRING, .cb = getopt_w,
		 .htyp = "string", .help = "Export what (default: "
		 "passwd,shadow,vxshadow,group)"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) ==
	       HXOPT_ERR_SUCCESS;
}

static void getopt_t(const struct HXoptcb *cbi)
{
	if (strcmp(cbi->data, "shadow") == 0)
		Output_type = OUTPUT_SHADOW;
	else if (strcmp(cbi->data, "mysql") == 0)
		Output_type = OUTPUT_MYSQL;
	else if (strcmp(cbi->data, "ldif") == 0)
		Output_type = OUTPUT_LDIF;
}

static void getopt_u(const struct HXoptcb *cbi)
{
	char *wk = HX_strdup(cbi->data);
	char *from = wk, *to;
	if ((to = strchr(from, ':')) != NULL)
		*to++ = '\0';
	if (*from != '\0')
		Uid_range[0] = strtol(from, NULL, 0);
	if (to != NULL && *to != '\0')
		Uid_range[1] = strtol(to, NULL, 0);
	free(wk);
}

static void getopt_w(const struct HXoptcb *cbi)
{
	char *orig_wk, *wk;
	const char *p;

	if (cbi->data == NULL)
		return;
	memset(Dump_what, 0, sizeof(Dump_what));
	wk = orig_wk = HX_strdup(cbi->data);

	while ((p = HX_strsep(&wk, ",")) != NULL) {
		if (strcmp(p, "passwd") == 0)
			Dump_what[DUMP_PASSWD] = true;
		else if (strcmp(p, "shadow") == 0)
			Dump_what[DUMP_SHADOW] = true;
		else if (strcmp(p, "vxshadow") == 0)
			Dump_what[DUMP_VXSHADOW] = true;
		else if (strcmp(p, "group") == 0)
			Dump_what[DUMP_GROUP] = true;
	}

	free(orig_wk);
}

/**
 * ldif_safe -
 * @s:	string to analyze
 *
 * Returns false if the string @s needs to be BASE-64 encoded to correspond
 * to the LDIF standard (RFC 2849). Using HX_isprint here would have been
 * nice, but more than just printable characters are allowed verbatim.
 * Returns true if it can be used as-is.
 */
static bool ldif_safe(const char *s)
{
	if (*signed_cast(const unsigned char *, s) >= 128 ||
	    *s == '\n' || *s == '\r' || *s == ' ' || *s == ':' || *s == '<')
		return false;

	while (*++s != '\0')
		if (*signed_cast(const unsigned char *, s) >= 128 ||
		    *s == '\n' || *s == '\r')
			return false;
	return true;
}

static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " dbdump\n");
	exit(EXIT_SUCCESS);
}
