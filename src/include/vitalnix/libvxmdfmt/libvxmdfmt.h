/*=============================================================================
Vitalnix User Management Suite
libvxmdfmt/libvxmdfmt.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2006
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
#ifndef _VITALNIX_LIBVXMDFMT_LIBVXMDFMT_H
#define _VITALNIX_LIBVXMDFMT_LIBVXMDFMT_H 1

#ifndef __cplusplus
#    include <stdio.h>
#else
#    include <cstdio>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PWLFMT_ESUCCESS = 1500,
    PWLFMT_ENOSTYLE,
    PWLFMT_EREQTPL,
    PWLFMT_EEINPUT,
    PWLFMT_EEOUTPUT,
    PWLFMT_EINPUTVER,
    PWLFMT_ENOUSERS,
};

struct pwlstyle_vtable;

struct pwlfmt_workspace {
    // Input (public)
    char *style_name, *input_file, *output_file, *template_file;
    void (*report)(const struct pwlfmt_workspace *, int, int);

    // Workspace (private)
    const struct pwlstyle_vtable *vtable;
    FILE *input_fh, *output_fh;
    void *style_data, *template_data;
    long num_entries;
};

/*
 *      PWLFMT.C
 */
extern int pwlfmt_new(struct pwlfmt_workspace *);
extern int pwlfmt_process(struct pwlfmt_workspace *);
extern const char *pwlfmt_strerror(int);
extern const struct pwlstyle_vtable *pwlstyles_trav(void **);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXMDFMT_LIBVXMDFMT_H

//=============================================================================
