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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libHX.h>
#include "sysprog/shared.h"

//-----------------------------------------------------------------------------
void SH_interactive(struct itab *qtabp) {
    char *ln = NULL;

    while(qtabp->fmt != NULL) {
        if(qtabp->type == 's') {
            char **ptr = qtabp->ptr;
            printf(qtabp->fmt, (*ptr != NULL) ? *ptr : "");
            fflush(stdout);
            HX_getl(&ln, stdin);
            HX_chomp(ln);
            if(*ln != '\0') { HX_strclone(ptr, ln); }
        } else if(qtabp->type == 'd') {
            int *ptr = qtabp->ptr;
            printf(qtabp->fmt, (ptr != NULL) ? *ptr : 0);
            fflush(stdout);
            HX_getl(&ln, stdin);
            HX_chomp(ln);
            if(*ln != '\0') { *ptr = strtol(ln, NULL, 0); }
        }

        if(!(qtabp->callback != NULL &&
         !qtabp->callback(qtabp->fmt, qtabp->type, qtabp->ptr))) {
            ++qtabp;
        }
    }

    if(ln != NULL) { hmc_free(ln); }
    return;
}

int SH_runcmd(const char *fmt, struct HX_repmap *map) {
    char cmd[4 * MAXLNLEN];
    HX_strrep5(fmt, map, cmd, sizeof(cmd));
    return system(cmd);
}

long SH_string_to_iday(const char *s) {
    struct tm td;
    time_t r;
    memset(&td, 0, sizeof(struct tm));

    if(strchr(s, '.') != NULL) {
        // DD.MM.YYYY
        sscanf(s, "%d.%d.%d", &td.tm_mday, &td.tm_mon, &td.tm_year);
    } else if(strchr(s, '/') != NULL) {
        // MM/DD/YYYY
        sscanf(s, "%d/%d/%d", &td.tm_mon, &td.tm_mday, &td.tm_year);
    } else if(strchr(s, '-') != NULL) {
        // YYYY-MM-DD (ISO 8601) format
        sscanf(s, "%d-%d-%d", &td.tm_year, &td.tm_mon, &td.tm_mday);
    } else {
        return -1;
    }

    r = mktime(&td);
    if(r == -1) { return -1; }
    return (unsigned long)r / 86400;
}

int SH_only_digits(char *p) {
    if(p == NULL) { return 0; }
    while(*p != '\0') {
        if(!isdigit(*p++)) { return 0; }
    }
    return 1;
}

//=============================================================================
