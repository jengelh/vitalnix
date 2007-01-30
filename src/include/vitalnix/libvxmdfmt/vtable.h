/*=============================================================================
Vitalnix User Management Suite
libvxmdfmt/vtable.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#ifndef _VITALNIX_LIBVXMDFMT_VTABLE_H
#define _VITALNIX_LIBVXMDFMT_VTABLE_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct pwl_data;
struct pwlfmt_workspace;
typedef void (printfunc_t)(struct pwlfmt_workspace *, const struct pwl_data *);

struct pwlstyle_vtable {
    const char *name, *desc;
    int require_template;
    int (*init)(struct pwlfmt_workspace *);
    void (*exit)(struct pwlfmt_workspace *);
    printfunc_t *file_header, *tbl_header, *tbl_entry,
                *tbl_footer, *file_footer;
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXMDFMT_VTABLE_H

//=============================================================================
