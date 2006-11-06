/*=============================================================================
Vitalnix User Management Suite
libvxcgi/libvxcgi.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2006
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
#ifndef _VITALNIX_LIBVXCGI_LIBVXCGI_H
#define _VITALNIX_LIBVXCGI_LIBVXCGI_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct HXbtree;

/*
 *      AUTH.C
 */
extern int vxcgi_authenticate(const char *, const char *);
extern int vxcgi_authenticate_ext(const char *, const char *, const char *);

/*
 *      CGI.C
 */
extern char *vxcgi_read_data(int, const char **);
extern struct HXbtree *vxcgi_split(char *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXCGI_LIBVXCGI_H

//=============================================================================
