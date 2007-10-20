/*
 *	libvxcore - Module registrator
 *	Copyright © CC Computer Consultants GmbH, 2006 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxcore/loader.h>

/* Variables */
static struct HXbtree *module_tree = NULL;
static pthread_mutex_t module_lock = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------------
static CONSTRUCTOR void vxcore_init(void)
{
	pthread_mutex_lock(&module_lock);
	if (module_tree == NULL)
		module_tree = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_SCMP | HXBT_CID);
	pthread_mutex_unlock(&module_lock);
	return;
}

static DESTRUCTOR void vxcore_exit(void)
{
	pthread_mutex_lock(&module_lock);
	if (module_tree != NULL)
		HXbtree_free(module_tree);
	module_tree = NULL;
	pthread_mutex_unlock(&module_lock);
	return;
}

EXPORT_SYMBOL int vxcore_module_register(const char *section, const char *name,
    const void *ptr)
{
	struct HXbtree *sect_tree;
	int esave;

	if (module_tree == NULL)
		vxcore_init();

	pthread_mutex_lock(&module_lock);
	if ((sect_tree = HXbtree_get(module_tree, section)) == NULL) {
		sect_tree = HXbtree_init(HXBT_MAP | HXBT_CKEY |
		            HXBT_SCMP | HXBT_CID);
		if (sect_tree == NULL) {
			esave = errno;
			fprintf(stderr, "%s: Unable to spawn new section tree for %s\n",
			     	__func__, section);
			return -esave;
		}
		if (HXbtree_add(module_tree, section, sect_tree) == NULL) {
			esave = errno;
			fprintf(stderr, "%s: Unable to add new section %s to "
			        "main tree: %s\n", __func__, section,
			        strerror(esave));
			HXbtree_free(sect_tree);
			return -esave;
		}
	}

	HXbtree_add(sect_tree, name, ptr);
	pthread_mutex_unlock(&module_lock);
	return 1;
}

EXPORT_SYMBOL void vxcore_module_unregister(const char *section,
    const char *name)
{
	struct HXbtree *sect_tree;

	pthread_mutex_lock(&module_lock);
	if ((sect_tree = HXbtree_get(module_tree, section)) != NULL)
		HXbtree_del(sect_tree, name);
	pthread_mutex_unlock(&module_lock);
	return;
}

EXPORT_SYMBOL struct HXbtree *vxcore_section_lookup(const char *section)
{
	struct HXbtree *ret;
	pthread_mutex_lock(&module_lock);
	ret = HXbtree_get(module_tree, section);
	pthread_mutex_unlock(&module_lock);
	return ret;
}

EXPORT_SYMBOL void *vxcore_module_lookup(const char *section, const char *name)
{
	const struct HXbtree *sect_tree;
	void *ret = NULL;

	pthread_mutex_lock(&module_lock);
	if ((sect_tree = HXbtree_get(module_tree, section)) != NULL)
		ret = HXbtree_get(sect_tree, name);
	pthread_mutex_unlock(&module_lock);
	return ret;
}

EXPORT_SYMBOL const void *vxcore_section_trav(void **trav_pptr,
    const char *section)
{
	const struct HXbtree_node *node;
	const struct HXbtree *btree;

	if ((btree = vxcore_section_lookup(section)) == NULL) {
		errno = ENOENT;
		return NULL;
	}

	if (*trav_pptr == NULL)
		*trav_pptr = HXbtrav_init(btree);

	if ((node = HXbtraverse(*trav_pptr)) == NULL) {
		HXbtrav_free(*trav_pptr);
		return NULL;
	}

	return node->data;
}

//=============================================================================
