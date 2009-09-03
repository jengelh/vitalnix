/*
 *	libvxcore - Module registrator
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2009
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <libHX/map.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxcore/loader.h>

/* Variables */
static struct HXmap *module_map;
static pthread_mutex_t module_lock = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------------
static CONSTRUCTOR void vxcore_init(void)
{
	pthread_mutex_lock(&module_lock);
	if (module_map == NULL)
		module_map = HXmap_init(HXMAPT_DEFAULT, HXMAP_SCKEY);
	pthread_mutex_unlock(&module_lock);
}

static DESTRUCTOR void vxcore_exit(void)
{
	pthread_mutex_lock(&module_lock);
	if (module_map != NULL) {
		HXmap_free(module_map);
		module_map = NULL;
	}
	pthread_mutex_unlock(&module_lock);
}

EXPORT_SYMBOL int vxcore_module_register(const char *section, const char *name,
    const void *ptr)
{
	struct HXmap *sect_map;
	int esave, ret;

	if (module_map == NULL)
		vxcore_init();

	pthread_mutex_lock(&module_lock);
	if ((sect_map = HXmap_get(module_map, section)) == NULL) {
		sect_map = HXmap_init(HXMAPT_DEFAULT, HXMAP_SCKEY);
		if (sect_map == NULL) {
			esave = errno;
			fprintf(stderr, "%s: Unable to create new section "
			        "map for %s\n",
			        __func__, section);
			return -esave;
		}
		ret = HXmap_add(module_map, section, sect_map);
		if (ret <= 0) {
			fprintf(stderr, "%s: Unable to add new section %s to "
			        "main map: %s\n", __func__, section,
			        strerror(-ret));
			HXmap_free(sect_map);
			return ret;
		}
	}

	HXmap_add(sect_map, name, ptr);
	pthread_mutex_unlock(&module_lock);
	return 1;
}

EXPORT_SYMBOL void vxcore_module_unregister(const char *section,
    const char *name)
{
	struct HXmap *sect_map;

	pthread_mutex_lock(&module_lock);
	if ((sect_map = HXmap_get(module_map, section)) != NULL)
		HXmap_del(sect_map, name);
	pthread_mutex_unlock(&module_lock);
}

EXPORT_SYMBOL struct HXmap *vxcore_section_lookup(const char *section)
{
	struct HXmap *ret;
	pthread_mutex_lock(&module_lock);
	ret = HXmap_get(module_map, section);
	pthread_mutex_unlock(&module_lock);
	return ret;
}

EXPORT_SYMBOL void *vxcore_module_lookup(const char *section, const char *name)
{
	const struct HXmap *sect_map;
	void *ret = NULL;

	pthread_mutex_lock(&module_lock);
	if ((sect_map = HXmap_get(module_map, section)) != NULL)
		ret = HXmap_get(sect_map, name);
	pthread_mutex_unlock(&module_lock);
	return ret;
}

EXPORT_SYMBOL const void *vxcore_section_trav(void **trav_pptr,
    const char *section)
{
	const struct HXmap_node *node;
	const struct HXmap *map;

	if ((map = vxcore_section_lookup(section)) == NULL) {
		errno = ENOENT;
		return NULL;
	}

	if (*trav_pptr == NULL)
		*trav_pptr = HXmap_travinit(map, HXMAP_DTRAV);

	if ((node = HXmap_traverse(*trav_pptr)) == NULL) {
		HXmap_travfree(*trav_pptr);
		return NULL;
	}

	return node->data;
}
