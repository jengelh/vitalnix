/*
 *	pdbinfo - Print database driver information
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2003 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include <vitalnix/libvxutil/defines.h>

/* Functions */
static void driver_info(const char *);
static void read_ldso_conf(void);
static void read_ldso_conf1(const char *);
static void read_environment(void);

static int get_options(int *, const char ***);
static void show_version(const struct HXoptcb *);

/* Variables */
static struct HXdeque *Dirs = NULL;
static unsigned int mcount  = 0;
static int OP_open          = 0;

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
			char buf[MAXFNLEN];
			if (strncmp(dentry, "drv_", 8) != 0)
				continue;
			snprintf(buf, sizeof(buf), "%s/%s",
			    	 static_cast(const char *, cd->ptr), dentry);
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
	struct vxpdb_state *md;
	char sepl = '<', sepr = '>';

	if ((md = vxpdb_load(fn)) == NULL && strcmp(fn, "*") == 0)
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
		vxpdb_open(md, 0);
		vxpdb_close(md);
	}

	vxpdb_unload(md);
	++mcount;
	return;
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
	return;
}

/*
 * read_ldso_conf1 -
 * @file:	file to analyze
 *
 * Reads @file and adds the search paths listed therein.
 */
static void read_ldso_conf1(const char *file)
{
	hmc_t *ln = NULL;
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
	hmc_free(ln);
	return;
}

/*
 * read_environment -
 *
 * Adds the search paths listed in LD_LIBRARY_PATH.
 */
static void read_environment(void)
{
	char *entry, *our, *travp;
	if ((entry = getenv("LD_LIBRARY_PATH")) == NULL)
		return;

	HX_strclone(&our, entry);
	travp = our;
	while ((travp = HX_strsep(&travp, ":")) != NULL)
		HXdeque_push(Dirs, travp);

	free(our);
	return;
}

static int get_options(int *argc, const char ***argv)
{
	struct HXoption options_table[] = {
		{.sh = 'L', .type = HXTYPE_STRDQ, .ptr = Dirs,
		 .help = "Additional search directory", .htyp = "dir"},
		{.sh = 'O', .type = HXTYPE_NONE, .ptr = &OP_open,
		 .help = "Open the database driver (vxpdb_open function)"},
		{.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
		 .help = "Show version information"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " VITALNIX_VERSION " pdbinfo\n");
	exit(EXIT_SUCCESS);
}

//=============================================================================
