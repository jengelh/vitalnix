/*=============================================================================
Vitalnix User Management Suite
libvxcore/loader.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxcore/loader.h>

// Variables
static struct HXbtree *module_tree = NULL;
static pthread_mutex_t module_lock = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------------
static CONSTRUCTOR void vxcore_init(void) {
    pthread_mutex_lock(&module_lock);
    if(module_tree == NULL)
        module_tree = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_SCMP | HXBT_CID);
    pthread_mutex_unlock(&module_lock);
    return;
}

static DESTRUCTOR void vxcore_exit(void) {
    pthread_mutex_lock(&module_lock);
    if(module_tree != NULL)
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

    if(module_tree == NULL)
        vxcore_init();

    pthread_mutex_lock(&module_lock);
    if((sect_tree = HXbtree_get(module_tree, section)) == NULL) {
        sect_tree = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_SCMP | HXBT_CID);
        if(sect_tree == NULL) {
            esave = errno;
            fprintf(stderr, "%s: Unable to spawn new section tree for %s\n",
                    __FUNCTION__, section);
            return -esave;
        }
        if(HXbtree_add(module_tree, section, sect_tree) == NULL) {
            esave = errno;
            fprintf(stderr, "%s: Unable to add new section %s to main "
                    "tree: %s\n", __FUNCTION__, section, strerror(esave));
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
    if((sect_tree = HXbtree_get(module_tree, section)) != NULL)
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
    if((sect_tree = HXbtree_get(module_tree, section)) != NULL)
        ret = HXbtree_get(sect_tree, name);
    pthread_mutex_unlock(&module_lock);
    return ret;
}

EXPORT_SYMBOL const void *vxcore_section_trav(void **trav_pptr,
  const char *section)
{
    const struct HXbtree_node *node;
    const struct HXbtree *btree;

    if((btree = vxcore_section_lookup(section)) == NULL) {
        errno = ENOENT;
        return NULL;
    }

    if(*trav_pptr == NULL)
        *trav_pptr = HXbtrav_init(btree, NULL);

    if((node = HXbtraverse(*trav_pptr)) == NULL) {
        HXbtrav_free(*trav_pptr);
        return NULL;
    }

    return node->data;
}

//=============================================================================
