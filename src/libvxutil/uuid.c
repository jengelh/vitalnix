/*=============================================================================
Vitalnix User Management Suite
libvxutil/uuid.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <libHX.h>
#include "compiler.h"
#include "libvxutil/libvxutil.h"

//-----------------------------------------------------------------------------
EXPORT_SYMBOL char *vxuuid_vx3(const char *full_name, long iday)
{
    unsigned char md[MD5_DIGEST_LENGTH];
    char tmp[48];

    MD5((const unsigned char *)full_name, strlen(full_name), md);
#define B "%02x"
#define D B B B B
    snprintf(tmp, sizeof(tmp), "{VX3}%04lx" D D D D,
             iday, md[0], md[1], md[2], md[3], md[4], md[5], md[6], md[7],
             md[8], md[9], md[10], md[11], md[12], md[13], md[14], md[15]);
#undef B
#undef D
    return HX_strdup(tmp);
}

EXPORT_SYMBOL long vxuuid_vx3_get_iday(const char *s)
{
    char tmp[5];
    if(strncmp(s, "{VX3}", 5) != 0)
        return -1;
    HX_strlcpy(tmp, s + 5, sizeof(tmp));
    return strtol(tmp, NULL, 16);
}

//=============================================================================