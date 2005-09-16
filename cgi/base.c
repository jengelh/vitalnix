/*=============================================================================
Vitalnix User Management Suite
  Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2003 - 2005
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
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "cgi/base.h"

//-----------------------------------------------------------------------------
char *cgi_read_data(int argc, const char **argv) {
    const char *meth = getenv("REQUEST_METHOD");

    if(meth == NULL) {
        if(argc >= 2) { return HX_strdup(argv[1]); }
        return NULL;
    }
    if(strcmp(meth, "GET") == 0) {
        return HX_strdup(getenv("QUERY_STRING"));
    } else if(strcmp(meth, "POST") == 0) {
        char *rt = NULL;
        HX_getl(&rt, stdin);
        HX_chomp(rt);
        return rt;
    }
    return NULL;
}

struct HXbtree *cgi_split(/* nonconst */ char *str) {
    struct HXbtree *h = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_CDATA);

    if(str == NULL) {
        // Caller needs a "working" array
        return h;
    }

    char *bufp = str, *ptr;
    while((ptr = HX_strsep(&bufp, "&")) != NULL) {
        char *key = ptr, *val = strchr(ptr, '=');
        if(val != NULL) {
            *val++ = '\0';
            if(*val == '\0') { val = NULL; }
        }
        HXbtree_add(h, key, val);
    }

    free(str);
    return h;
}

//=============================================================================
