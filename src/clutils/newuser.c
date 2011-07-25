/*
 *	newuser - MDSYNC-compatible single user add
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2011
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
#include <time.h>
#include <unistd.h>
#include <libHX/ctype_helper.h>
#include <libHX/defs.h>
#include <libHX/init.h>
#include <libHX/map.h>
#include <libHX/option.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxcli/libvxcli.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include <vitalnix/libvxmdsync/libvxmdsync.h>
#include <vitalnix/libvxdb/config.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
struct private_info {
	char *db_name, *bday, *first_name, *group_name, *pref_username;
	char *pvgrp, *surname, *uuid;
	unsigned int interactive, run_master;

	bool open_status;
	struct vxdb_state *db_handle;
	struct mdsync_workspace *mdsw;
};

/* Functions */
static int single_init(struct private_info *);
static int single_run(struct private_info *);
static void single_cleanup(struct private_info *);
static void single_interactive(struct private_info *);
static int validate_group(const struct vxcq_entry *);

static bool get_options(int *, const char ***, struct private_info *);
static void show_version(const struct HXoptcb *);

//-----------------------------------------------------------------------------
static int main2(int argc, const char **argv)
{
	struct private_info priv;
	int ret = EXIT_SUCCESS;

	memset(&priv, 0, sizeof(priv));
	priv.db_name	= HX_strdup("*");
	priv.run_master = 1; /* enabled by default */

	if (!get_options(&argc, &argv, &priv))
		return EXIT_FAILURE;
	if (priv.interactive)
		show_version(NULL);
	fprintf(stderr, "Note that Vitalnix/mdsingle will not check if a user "
	        "with the same UUID already exists. Make sure you do not "
	        "add a user twice by accident.\n");
	if (priv.interactive)
		single_interactive(&priv);
	if (!single_init(&priv) || !single_run(&priv))
		return EXIT_FAILURE;

	single_cleanup(&priv);
	return ret;
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

static int single_init(struct private_info *priv)
{
	struct mdsync_workspace *mdsw;
	int ret;

	if (priv->group_name == NULL || priv->first_name == NULL) {
		fprintf(stderr, "You have to specify either -f and -g, or -I. "
		        "Use -? to get help.\n");
		return 0;
	}

	if ((priv->db_handle = vxdb_load(priv->db_name)) == NULL) {
		perror("Error loading database");
		return 0;
	}

	if ((ret = vxdb_open(priv->db_handle, VXDB_WRLOCK)) <= 0) {
		fprintf(stderr, "Error opening database: %s\n",
		        strerror(-ret));
		return 0;
	}

	priv->open_status = true;

	if ((priv->mdsw = mdsw = mdsync_init()) == NULL) {
		perror("Init procedure failed");
		return 0;
	}

	mdsw->database     = priv->db_handle;
	mdsw->user_private = priv;
	return 1;
}

static int single_run(struct private_info *priv)
{
	struct mdsync_workspace *mdsw = priv->mdsw;
	char *username, *password, tmp[8];
	struct vxeds_entry ee;
	int ret;

	if ((ret = mdsync_prepare_group(mdsw, priv->group_name)) < 0) {
		fprintf(stderr, "Error querying database: %s\n",
		        strerror(-ret));
		return 0;
	} else if (ret == 0) {
		fprintf(stderr, "Group \"%s\" does not exist\n",
		        priv->group_name);
		return 0;
	}

	memset(&ee, 0, sizeof(ee));
	ee.first_name = HX_strdup(priv->first_name);
	ee.surname    = HX_strdup(priv->surname);
	ee.full_name  = vxeds_bfullname(ee.first_name, ee.surname);
	ee.pvgrp      = HX_strdup(priv->pvgrp);
	if (priv->pref_username == NULL)
		ee.username = HX_strdup(vxutil_propose_lname(tmp, sizeof(tmp),
		              ee.surname, ee.first_name));
	else
		ee.username = priv->pref_username;

	/* The space allocated for ->pref_username is handed over to EDS. */
	priv->pref_username = NULL;

	if ((ee.uuid = priv->uuid) == NULL && priv->bday != NULL)
		ee.uuid = vxuuid_vx3(ee.full_name,
		          vxutil_string_xday(priv->bday));

	HXmap_add(mdsw->add_req, ee.uuid, HX_memdup(&ee, sizeof(ee)));
	mdsync_compare_simple(mdsw);
	mdsync_fixup(mdsw);

	if (!priv->run_master) {
		struct vxconfig_useradd *c = &mdsw->config.add_opts;
		HX_strclone(&c->master_preadd, NULL);
		HX_strclone(&c->master_postadd, NULL);
	}
	if ((ret = mdsync_add(mdsw)) <= 0) {
		printf("Add procedure failed: %s\n", strerror(-ret));
		return 0;
	}

	HX_chomp(mdsw->output_data);
	username = mdsw->output_data;
	if ((password = strchr(mdsw->output_data, ':')) != NULL)
		*password++ = '\0';
	printf("Successfully added \"%s\" with password \"%s\"\n",
	       username, password);
	return 1;
}

static void single_cleanup(struct private_info *priv)
{
	if (priv->mdsw != NULL)
		mdsync_free(priv->mdsw);
	if (priv->db_handle != NULL) {
		if (priv->open_status)
			vxdb_close(priv->db_handle);
		vxdb_unload(priv->db_handle);
	}
	free(priv->db_name);
	free(priv->bday);
	free(priv->first_name);
	free(priv->group_name);
	/*
	 * Do not free(priv->pref_username); -- that pointer
	 * belongs to the EDS subsystem by now.
	 */
	free(priv->pvgrp);
	free(priv->surname);
	free(priv->uuid);
}

static void single_interactive(struct private_info *priv)
{
	char tmp[9];
	struct vxcq_entry table_1[] = {
		{
			.msg    = "Enter the first name of the new user.\n",
			.prompt = "First name",
			.defl   = priv->first_name,
			.type   = HXTYPE_STRING,
			.ptr    = &priv->first_name,
			.flags  = VXCQ_ZNULL,
		},
		{
			.msg    = "Enter the last name of the new user.\n",
			.prompt = "Last name",
			.defl   = priv->surname,
			.type   = HXTYPE_STRING,
			.ptr    = &priv->surname,
			.flags  = VXCQ_EMPTY | VXCQ_ZNULL,
		},
		VXCQ_TABLE_END,
	};
	struct vxcq_entry table_2[] = {
		{
			.msg    = "This is the proposed username of the new "
			          "account, which you may change. If "
			          "necessary, Vitalnix will add an index "
			          "number to the username or adjust it to "
			          "avoid conflict with an already existing "
			          "username.\n",
			.prompt = "Preferred username",
			.defl   = tmp,
			.type   = HXTYPE_STRING,
			.ptr    = &priv->pref_username,
			.flags  = VXCQ_ZNULL,
		},
		{
			.msg      = "Enter the group name or GID to add the "
			            "user to.\n",
			.prompt   = "System group",
			.defl     = priv->group_name,
			.type     = HXTYPE_STRING,
			.ptr      = &priv->group_name,
			.uptr     = priv->db_name,
			.flags    = VXCQ_ZNULL,
			.validate = validate_group,
		},
		{
			.msg    = "Enter a user-defined subgroup of "
			          "the new user.\n",
			.prompt = "Private group/Class",
			.defl   = priv->pvgrp,
			.type   = HXTYPE_STRING,
			.ptr    = &priv->pvgrp,
			.flags  = VXCQ_EMPTY | VXCQ_ZNULL,
		},
		{
			.msg    = "If there is no external UUID available to "
			          "uniquely identify the user, you can have "
			          "one made up from the name-day tuple. In "
			          "that case, enter the birthdate of the "
			          "user, otherwise leave blank.\n",
			.prompt = "YYYY-MM-DD",
			.defl   = priv->bday,
			.type   = HXTYPE_STRING,
			.ptr    = &priv->bday,
			.flags  = VXCQ_EMPTY | VXCQ_ZNULL,
		},
		VXCQ_TABLE_END,
	};
	struct vxcq_entry table_3[] = {
		{
			.msg    = "Enter the external UUID of the user. If "
			          "you specify something here, it will "
			          "override the automatic UUID generation "
			          "from the birthdate.\n",
			.prompt = "UUID string",
			.defl   = priv->uuid,
			.type   = HXTYPE_STRING,
			.ptr    = &priv->uuid,
			.flags  = VXCQ_EMPTY | VXCQ_ZNULL,
		},
		VXCQ_TABLE_END,
	};

	vxcli_query_v(table_1);
	if (!HX_isupper(*priv->first_name)) {
		if (isatty(STDOUT_FILENO))
			printf("\e[1;37;41m"); /* ] */
		printf("WARNING: The first char of the name is not uppercase, "
		       "which is incorrect in most cases. (Hit Ctrl+C to "
		       "exit and retry.)\n");
		if (isatty(STDOUT_FILENO))
			printf("\e[0m"); /* ] */
	}

	vxutil_propose_lname(tmp, sizeof(tmp), priv->surname, priv->first_name);
	vxcli_query_v(table_2);
	if (priv->bday == NULL)
		vxcli_query_v(table_3);
}

static int validate_group(const struct vxcq_entry *e)
{
	struct vxdb_group group = {};
	struct vxdb_state *mh;
	const char *group_name = *static_cast(const char **, e->ptr);
	int ret;

	if ((mh = vxdb_load(e->uptr)) == NULL) {
		fprintf(stderr, "Could not load database for group "
		        "validation: %s\n", strerror(errno));
		return 0;
	}
	if ((ret = vxdb_open(mh, 0)) <= 0) {
		fprintf(stderr, "Could not open database for group "
		        "validation: %s\n", strerror(-ret));
		goto out_open;
	}

	if (vxutil_only_digits(group_name))
		ret = vxdb_getgrgid(mh, strtol(group_name, NULL, 0), &group);
	else
		ret = vxdb_getgrnam(mh, group_name, &group);

	if (ret < 0)
		fprintf(stderr, "Error querying database for group "
		        "validation: %s\n", strerror(-ret));
	else if (ret == 0)
		fprintf(stderr, "Group \"%s\" does not exist\n", group_name);

	vxdb_group_free(&group, false);

 out_open:
	vxdb_unload(mh);
	return ret > 0;
}

//-----------------------------------------------------------------------------
static bool get_options(int *argc, const char ***argv, struct private_info *p)
{
	struct HXoption options_table[] = {
		{.ln = "master", .type = HXTYPE_NONE, .ptr = &p->run_master,
		 .help = "Do run MASTER_* scripts"},
		{.ln = "vxdb", .type = HXTYPE_STRING, .ptr = &p->db_name,
		 .help = "Use specified database", .htyp = "name"},
		{.sh = 'I', .type = HXTYPE_NONE, .ptr = &p->interactive,
		 .help = "Run in interactive mode"},
		{.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
		 .help = "Show version information"},
		{.sh = 'b', .type = HXTYPE_STRING, .ptr = &p->bday,
		 .htyp = "date", .help = "Generate a UUID from name and "
		 "birthdate (overridden by -x)"},
		{.sh = 'f', .type = HXTYPE_STRING, .ptr = &p->first_name,
		 .help = "First name of the user"},
		{.sh = 'g', .type = HXTYPE_STRING, .ptr = &p->group_name,
		 .help = "System group to put this user in", .htyp = "group"},
		{.sh = 'p', .type = HXTYPE_STRING, .ptr = &p->pvgrp,
		 .help = "User-defined private group id"},
		{.sh = 's', .type = HXTYPE_STRING, .ptr = &p->surname,
		 .help = "Last name of the user"},
		{.sh = 'x', .type = HXTYPE_STRING, .ptr = &p->uuid,
		 .help = "Unique identifier for user", .htyp = "uuid"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " newuser\n");
	if (cbi != NULL)
		exit(EXIT_SUCCESS);
}
