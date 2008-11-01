/*
 *	dbinfo - Print database driver information
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/deque.h>
#include <libHX/misc.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/defines.h>

/* Functions */
static void driver_info(const char *);
static void read_ldso_conf(void);
static void read_ldso_conf1(const char *);
static void read_environment(void);

static bool get_options(int *, const char ***);
static void show_version(const struct HXoptcb *);

/* Variables */
static struct HXdeque *Dirs = NULL;
static unsigned int mcount  = 0;
static unsigned int OP_open = false;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct HXdeque_node *cd;

	Dirs = HXdeque_init();
	HXdeque_push(Dirs, ".");
	read_ldso_conf();
#ifdef CONFIG_LIBDIR
	HXdeque_push(Dirs, CONFIG_LIBDIR);
#endif
	read_environment();

	if (!get_options(&argc, &argv))
		return EXIT_FAILURE;

	driver_info("*");

	for (cd = Dirs->first; cd != NULL; cd = cd->next) {
		struct HXdir *cdp;
		const char *dentry;

		if ((cdp = HXdir_open(cd->ptr)) == NULL)
			continue;

		while ((dentry = HXdir_read(cdp)) != NULL) {
			char buf[MAXFNLEN], *p;

			if (strncmp(dentry, "drv_", 4) != 0 ||
			    strcmp(dentry + strlen(dentry) - 3, ".so") != 0)
				continue;
			if ((p = strrchr(dentry, '-')) != NULL)
				snprintf(buf, sizeof(buf), "%.*s",
				         (int)(p - (dentry + 4)), dentry + 4);
			else
				snprintf(buf, sizeof(buf), "%.*s",
				         (int)(strlen(dentry + 4) - 3),
				         dentry + 4);
			driver_info(buf);
		}

		HXdir_close(cdp);
	}

	printf("------------------------------------------------------------\n");
	printf("Read %u modules\n", mcount);
	HXdeque_free(Dirs);
	return EXIT_SUCCESS;
}

/*
 * driver_info -
 * @fn:	filename of the driver
 *
 * Prints out info about the specified driver.
 */
static void driver_info(const char *fn)
{
	struct vxdb_state *md;
	char sepl = '<', sepr = '>';

	if ((md = vxdb_load(fn)) == NULL && strcmp(fn, "*") == 0)
		return;

	printf(
		"------------------------------------------------------------\n"
		"Module %c%s%c:\n",
		sepl, fn, sepr
	);

	if (md == NULL) {
		printf("	ERROR: Could not load module\n");
		return;
	}

	if (fn[0] == '*' && fn[1] == '\0')
		sepl = sepr = '"';

	printf(
		"	Name	   : %s\n"
		"	Description: %s\n",
		md->vtable->name, md->vtable->desc
	);

	if (OP_open) {
		vxdb_open(md, 0);
		printf("	Number of users/groups: %ld/%ld\n",
		       vxdb_modctl(md, VXDB_COUNT_USERS),
		       vxdb_modctl(md, VXDB_COUNT_GROUPS));
		vxdb_close(md);
	}

	vxdb_unload(md);
	++mcount;
}

/*
 * read_ldso_conf -
 *
 * Looks into /etc/ld.so.conf and /etc/ld.so.conf.d.
 */
static void read_ldso_conf(void)
{
	const char *dentry;
	char buf[MAXFNLEN];
	void *dirp;

	read_ldso_conf1("/etc/ld.so.conf");
	if ((dirp = HXdir_open("/etc/ld.so.conf.d")) == NULL)
		return;
	while ((dentry = HXdir_read(dirp)) != NULL) {
		snprintf(buf, sizeof(buf), "/etc/ld.so.conf.d/%s", dentry);
		read_ldso_conf1(buf);
	}

	HXdir_close(dirp);
}

/*
 * read_ldso_conf1 -
 * @file:	file to analyze
 *
 * Reads @file and adds the search paths listed therein.
 */
static void read_ldso_conf1(const char *file)
{
	hxmc_t *ln = NULL;
	FILE *fp;

	if ((fp = fopen(file, "r")) == NULL)
		return;

	while (HX_getl(&ln, fp) != NULL) {
		char *q;
		if (*ln != '/')
			continue;
		HX_chomp(ln);
		if ((q = strchr(ln, '=')) != NULL)
			*q = '\0';
		HXdeque_push(Dirs, HX_strdup(ln));
	}

	fclose(fp);
	HXmc_free(ln);
}

/*
 * read_environment -
 *
 * Adds the search paths listed in LD_LIBRARY_PATH.
 */
static void read_environment(void)
{
	char *working_copy, *walker, *dirname;

	working_copy = HX_strdup(getenv("LD_LIBRARY_PATH"));

	if (working_copy == NULL)
		return;
	while ((dirname = HX_strsep(&walker, ":")) != NULL)
		HXdeque_push(Dirs, dirname);

	free(working_copy);
}

static bool get_options(int *argc, const char ***argv)
{
	struct HXoption options_table[] = {
		{.sh = 'L', .type = HXTYPE_STRDQ, .ptr = Dirs,
		 .help = "Additional search directory", .htyp = "dir"},
		{.sh = 'O', .type = HXTYPE_NONE, .ptr = &OP_open,
		 .help = "Open the database driver (vxdb_open function)"},
		{.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
		 .help = "Show version information"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " dbinfo\n");
	exit(EXIT_SUCCESS);
}
