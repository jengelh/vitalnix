/*=============================================================================
Vitalnix User Management Suite
libvxeds/eds.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2004 - 2007
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
#include <errno.h>
#include <stdio.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include <vitalnix/libvxeds/vtable.h>
#include <vitalnix/libvxcore/loader.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

// Definitions
struct vxeds_state {
    const struct edsformat_vtable *vtable;
    void *private_data;
};

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int vxeds_open(const char *identifier, const char *fmt,
  void **state_pptr)
{
    struct vxeds_state state, *stp;
    int ret;

    if(fmt == NULL && (fmt = vxeds_derivefromname(identifier)) == NULL)
        return -EINVAL;
    if((state.vtable = vxcore_module_lookup("libvxeds", fmt)) == NULL)
        return -EINVAL;
    if((stp = *state_pptr = HX_memdup(&state, sizeof(state))) == NULL)
        return -errno;
    if((ret = stp->vtable->open(identifier, &stp->private_data)) <= 0) {
        free(stp);
        return ret;
    }

    return 1;
}

EXPORT_SYMBOL int vxeds_read(void *state_ptr, struct vxeds_entry *e)
{
    struct vxeds_state *state = state_ptr;
    return state->vtable->read(state->private_data, e);
}

EXPORT_SYMBOL void vxeds_close(void *state_ptr)
{
    struct vxeds_state *state = state_ptr;
    state->vtable->close(state->private_data);
    free(state);
    return;
}

EXPORT_SYMBOL const char *vxeds_derivefromname(const char *filename)
{
    const char *p;

    if((p = strrchr(filename, '.')) == NULL)
        return NULL;
    if(vxcore_module_lookup("libvxeds", ++p) != NULL)
        return p;

    return NULL;
}

EXPORT_SYMBOL void vxeds_free_entry(struct vxeds_entry *e)
{
    free(e->username);
    free(e->first_name);
    free(e->surname);
    free(e->full_name);
    free(e->pvgrp);
    free(e->uuid);
    return;
}

EXPORT_SYMBOL char *vxeds_bfullname(const char *first_name,
  const char *surname)
{
    char *full_name;
    if(surname == NULL)
        return HX_strdup(first_name);

    full_name = malloc(strlen(first_name) + strlen(surname) + 2);
    strcpy(full_name, first_name);
    strcat(full_name, " ");
    strcat(full_name, surname);
    return full_name;
}

EXPORT_SYMBOL
const struct edsformat_vtable *vxeds_formats_trav(void **trav_pptr)
{
    return vxcore_section_trav(trav_pptr, "libvxeds");
}

//=============================================================================
