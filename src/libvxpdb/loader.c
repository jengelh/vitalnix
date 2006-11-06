/*=============================================================================
Vitalnix User Management Suite
libvxpdb/loader.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2006
  -- License restrictions apply (LGPL v2.1)

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
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxcore/loader.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/defines.h>

// Definitions
enum {
    CONFIG_READ = 1,
    CONFIG_FREE,
};

struct opt {
    int no_indirect;
    char *driver_name, *driver_lib;
    char driver_file[MAXFNLEN];
};

// Functions
static int vxpdb_cleanup(struct vxpdb_state *, int, struct opt *);
static int vxpdb_config(struct opt *, unsigned int, const char *);
static void *vxpdb_get_handle(const struct opt *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL struct vxpdb_state *vxpdb_load(const char *name)
{
    struct vxpdb_state *new = NULL;
    struct vxpdb_mvtable *vtable;
    struct opt cf = {};
    int ret;

    if(!vxpdb_config(&cf, CONFIG_READ, name) || cf.driver_name == NULL ||
     *cf.driver_name == '\0') {
        errno = vxpdb_cleanup(new, EINVAL, &cf);
        return NULL;
    }

    if((new = malloc(sizeof(struct vxpdb_state))) == NULL) {
        errno = vxpdb_cleanup(new, errno, &cf);
        return NULL;
    }

    if(!(
        // Try to load the .SO and look for THIS_MODULE
        ((new->handle = vxpdb_get_handle(&cf)) != NULL &&
        (vtable = HX_dlsym(new->handle, "THIS_MODULE")) != NULL) ||

        /* If the two fail, either because the .SO does not exist, THIS_MODULE
        has visibility=hidden, or the module is static, see if the module
        registered itself. */
        (vtable = vxcore_module_lookup("libvxpdb", cf.driver_name)) != NULL
        /* Invert the whole sense because this is the
        error path, not the success path. */
    )) {
        errno = vxpdb_cleanup(new, (errno != 0) ? errno : EINVAL, &cf);
        return NULL;
    }

    new->vtable = vtable;
    vxpdb_fix_vtable(vtable);
    if(vtable->init != NULL && (ret = vtable->init(new, cf.driver_file)) <= 0) {
        errno = vxpdb_cleanup(new, -ret, &cf);
        return NULL;
    }

    vxpdb_cleanup(NULL, 0, &cf);
    return new;
}

EXPORT_SYMBOL void vxpdb_unload(struct vxpdb_state *thx)
{
    if(thx->vtable->deinit != NULL)
        thx->vtable->deinit(thx);
    vxpdb_cleanup(thx, 0, NULL);
    return;
}

//-----------------------------------------------------------------------------
static int vxpdb_cleanup(struct vxpdb_state *thx, int err, struct opt *cf) {
    if(cf != NULL)
        vxpdb_config(cf, CONFIG_FREE, NULL);
    if(thx != NULL) {
        if(thx->handle != NULL)
            HX_dlclose(thx->handle);
        free(thx);
    }
    return err;
}

/*  vxpdb_config
    @cf:        pointer to config structure
    @action:    %CONFIG_READ or %CONFIG_FREE
    @L1_name:   level-1 name

    %CONFIG_READ: Resolve the standard database ("*") into a real database,
    and resolve     into its configuration file.
*/
static int vxpdb_config(struct opt *cf, unsigned int action,
  const char *L1_name)
{
    const char *L2_name = NULL;
    struct HXoption opt_driver[] = {
        {.ln = "_DRIVER", .type = HXTYPE_STRING, .ptr = &cf->driver_name},
        {.ln = "_DRIVER_LIB", .type = HXTYPE_STRING, .ptr = &cf->driver_lib},
        HXOPT_TABLEEND,
    };

    if(action == CONFIG_READ) {
        char L2_file[MAXFNLEN];
        struct stat sb;
        struct HXoption opt_database[] = {
            {.ln = "DEFAULT_DATABASE", .type = HXTYPE_STRING, .ptr = &L2_name},
            HXOPT_TABLEEND,
        };

        // If @name == "*", then read the database from libvxpdb.conf.
        if(strcmp(L1_name, "*") == 0) {
            HX_shconfig(CONFIG_SYSCONFDIR "/libvxpdb.conf", opt_database);
            if(L2_name == NULL) {
                HX_shconfig_free(opt_database);
                return 0;
            }
        } else {
            L2_name = L1_name;
        }

        snprintf(L2_file, sizeof(L2_file), "%s/db_%s.conf",
                 CONFIG_SYSCONFDIR, L2_name);

        if(strpbrk(L2_name, "./") != NULL) // direct file spec
            HX_strlcpy(cf->driver_file, L2_name, sizeof(cf->driver_file));
        else if(stat(L2_file, &sb) == 0) // etc/db_@@.conf
            HX_strlcpy(cf->driver_file, L2_file, sizeof(cf->driver_file));

        if(strcmp(L1_name, "*") == 0)
            HX_shconfig_free(opt_database);
        HX_shconfig(cf->driver_file, opt_driver);
    } else if(action == CONFIG_FREE) {
        HX_shconfig_free(opt_driver);
    }
    return 1;
}

/*  vxpdb_get_handle
    @filename:  Shared library to open

    Opens @filename or a construction of "drv_" @filename and an extension
    and returns the handle.
*/
static void *vxpdb_get_handle(const struct opt *cf) {
    static const char *const ext[] = {".so", ".dll", "", NULL};
    const char *const *extp = ext;
    void *handle;

    // Try plain filename first (as does pdbinfo)
    if(cf->driver_lib != NULL && (handle = HX_dlopen(cf->driver_lib)) != NULL)
        return handle;

    // If not, prepend prefix, append an extension and try again
    errno = ENOENT;
    while(*extp != NULL) {
        char fn[MAXFNLEN];
        snprintf(fn, MAXFNLEN, "drv_%s%s", cf->driver_name, *extp);
        if((handle = HX_dlopen(fn)) != NULL) {
            errno = 0;
            break;
        }
        ++extp;
    }
    return handle;
}

//=============================================================================
