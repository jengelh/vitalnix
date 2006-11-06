/*=============================================================================
Vitalnix User Management Suite
libvxeds/libvxeds.h
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
#ifndef _VITALNIX_LIBVXEDS_LIBVXEDS_H
#define _VITALNIX_LIBVXEDS_LIBVXEDS_H 1

#ifdef __cplusplus
extern "C" {
#endif

// Definitions
struct edsformat_vtable;
struct vxeds_entry {
    char *username, *first_name, *surname, *full_name, *pvgrp, *uuid;
};

/*
 *      EDS.C
 */
extern int vxeds_open(const char *, const char *, void **);
extern int vxeds_read(void *, struct vxeds_entry *);
extern void vxeds_close(void *);
extern const char *vxeds_derivefromname(const char *);
extern void vxeds_free_entry(struct vxeds_entry *);
extern const struct edsformat_vtable *vxeds_formats_trav(void **);

/*
 *      INTERNAL USE
 */
extern char *vxeds_bfullname(const char *, const char *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXEDS_LIBVXEDS_H

//=============================================================================
