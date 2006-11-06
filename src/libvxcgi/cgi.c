/*=============================================================================
Vitalnix User Management Suite
libvxcgi/cgi.c
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
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxcgi/libvxcgi.h>

//-----------------------------------------------------------------------------
EXPORT_SYMBOL char *vxcgi_read_data(int argc, const char **argv)
{
    const char *meth = getenv("REQUEST_METHOD");

    if(meth == NULL) {
        if(argc >= 2)
            return HX_strdup(argv[1]);
        return NULL;
    }

    if(strcmp(meth, "GET") == 0) {
        return HX_strdup(getenv("QUERY_STRING"));
    } else if(strcmp(meth, "POST") == 0) {
        char *len_s = getenv("CONTENT_LENGTH");
        size_t len  = strtoul(len_s, NULL, 0);
        char *new;

        if((new = malloc(len + 1)) != NULL) {
            new[len]    = '\0';
            fread(new, len, 1, stdin);
        }
        return new;
    }

    return NULL;
}

EXPORT_SYMBOL struct HXbtree *vxcgi_split(char *str)
{
    struct HXbtree *h = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_CDATA);
    char *bufp = str, *ptr;

    if(h == NULL || str == NULL)
        return h;

    while((ptr = HX_strsep(&bufp, "&")) != NULL) {
        const char *key = ptr;
        char *val = strchr(ptr, '=');

        if(val != NULL) {
            *val++ = '\0';
            if(*val == '\0')
                val = NULL;
        }
        HXbtree_add(h, key, val);
    }

    free(str);
    return h;
}

//=============================================================================
