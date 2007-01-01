/*=============================================================================
Vitalnix User Management Suite
libvxeds/static-build.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007
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
#ifndef _VITALNIX_LIBVXEDS_STATICBUILD_H
#define _VITALNIX_LIBVXEDS_STATICBUILD_H 1

#include <vitalnix/compiler.h>
#include <vitalnix/libvxcore/loader.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REGISTER_MODULE(name, ptr) \
    static CONSTRUCTOR void register_libvxeds_##name(void) { \
        vxcore_module_register("libvxeds", #name, (ptr)); \
        return; \
    } \
    static DESTRUCTOR void unregister_libvxeds_##name(void) { \
        vxcore_module_unregister("libvxeds", #name); \
        return; \
    }

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXEDS_STATICBUILD_H

//=============================================================================
