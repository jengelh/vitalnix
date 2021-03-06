/*
 *	libvxdb/loader.c - Back-end module loader
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/misc.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxcore/loader.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxutil/defines.h>

/* Definitions */
enum {
	CONFIG_READ = 1,
	CONFIG_FREE,
};

struct opt {
	char *driver_name, *driver_lib;
	char driver_file[MAXFNLEN];
};

/* Functions */
static int vxdb_cleanup(struct vxdb_state *, int, struct opt *);
static int vxdb_config(struct opt *, unsigned int, const char *);
static void *vxdb_get_handle(const struct opt *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL struct vxdb_state *vxdb_load(const char *name)
{
	struct vxdb_state *new = NULL;
	struct vxdb_driver *vtable;
	struct opt cf = {};
	int ret;

	if (!vxdb_config(&cf, CONFIG_READ, name) || cf.driver_name == NULL ||
	    *cf.driver_name == '\0') {
		errno = EINVAL;
		goto out;
	}

	if ((new = malloc(sizeof(struct vxdb_state))) == NULL)
		goto out;

	/* Try to load the .SO and look for THIS_MODULE */
	new->handle = vxdb_get_handle(&cf);
	if (new->handle == NULL)
		goto fail;
	vtable = HX_dlsym(new->handle, "THIS_MODULE");
	if (vtable == NULL)
		goto fail;

	new->vtable = vtable;
	vxdb_fix_vtable(vtable);
	if (vtable->init != NULL && (ret = vtable->init(new, cf.driver_file)) <= 0) {
		errno = -ret;
		goto out;
	}

	vxdb_cleanup(NULL, 0, &cf);
	return new;

 fail:
	if (errno == 0)
		errno = EINVAL;
 out:
	errno = vxdb_cleanup(new, errno, &cf);
	return NULL;
}

EXPORT_SYMBOL void vxdb_unload(struct vxdb_state *thx)
{
	if (thx->vtable->exit != NULL)
		thx->vtable->exit(thx);
	vxdb_cleanup(thx, 0, NULL);
}

//-----------------------------------------------------------------------------
static int vxdb_cleanup(struct vxdb_state *thx, int err, struct opt *cf)
{
	if (cf != NULL)
		vxdb_config(cf, CONFIG_FREE, NULL);
	if (thx != NULL) {
		if (thx->handle != NULL)
			HX_dlclose(thx->handle);
		free(thx);
	}
	return err;
}

/**
 * vxdb_config -
 * @cf:		pointer to config structure
 * @action:	%CONFIG_READ or %CONFIG_FREE
 * @L1_name:	level-1 name
 *
 * %CONFIG_READ: Resolve the standard database ("*") into a real database,
 * and resolve into its configuration file.
 */
static int vxdb_config(struct opt *cf, unsigned int action,
    const char *L1_name)
{
	const char *L2_name = NULL;
	struct HXoption opt_driver[] = {
		{.ln = "_DRIVER", .type = HXTYPE_STRING, .ptr = &cf->driver_name},
		{.ln = "_DRIVER_LIB", .type = HXTYPE_STRING, .ptr = &cf->driver_lib},
		HXOPT_TABLEEND,
	};

	if (action == CONFIG_READ) {
		char L2_file[MAXFNLEN];
		struct stat sb;
		struct HXoption opt_database[] = {
			{.ln = "DEFAULT_DATABASE", .type = HXTYPE_STRING, .ptr = &L2_name},
			HXOPT_TABLEEND,
		};

		/* If @name == "*", then read the database from libvxdb.conf */
		if (strcmp(L1_name, "*") == 0) {
			HX_shconfig(CONFIG_SYSCONFDIR "/libvxdb.conf", opt_database);
			if (L2_name == NULL) {
				HX_shconfig_free(opt_database);
				return 0;
			}
		} else {
			L2_name = L1_name;
		}

		snprintf(L2_file, sizeof(L2_file), "%s/db_%s.conf",
				 CONFIG_SYSCONFDIR, L2_name);

		if (strpbrk(L2_name, "./") != NULL)
			/* direct file spec */
			HX_strlcpy(cf->driver_file, L2_name, sizeof(cf->driver_file));
		else if (stat(L2_file, &sb) == 0)
			/* "etc/db_@@.conf" */
			HX_strlcpy(cf->driver_file, L2_file, sizeof(cf->driver_file));

		if (strcmp(L1_name, "*") == 0)
			HX_shconfig_free(opt_database);
		HX_shconfig(cf->driver_file, opt_driver);
	} else if (action == CONFIG_FREE) {
		HX_shconfig_free(opt_driver);
	}
	return 1;
}

/**
 * vxdb_get_handle -
 * @filename:	Shared library to open
 *
 * Opens @filename or a construction of "drv_", @filename and an extension
 * and returns the handle on success, or %NULL on failure.
 */
static void *vxdb_get_handle(const struct opt *cf)
{
	static const char *const ext[] = {".so", ".dll", "", NULL};
	const char *const *extp = ext;
	void *handle;

	/* Try plain filename first (as does vxdbinfo) */
	if (cf->driver_lib != NULL &&
	    (handle = HX_dlopen(cf->driver_lib)) != NULL)
		return handle;

	/* If not, prepend prefix, append an extension and try again */
	errno = ENOENT;
	while (*extp != NULL) {
		char fn[MAXFNLEN];
		snprintf(fn, MAXFNLEN, CONFIG_LIBDIR "/drv_%s%s",
		         cf->driver_name, *extp);
		if ((handle = HX_dlopen(fn)) != NULL) {
			errno = 0;
			break;
		}
		++extp;
	}
	return handle;
}
