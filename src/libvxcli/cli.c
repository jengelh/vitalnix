/*=============================================================================
Vitalnix User Management Suite
libvxcli/cli.c
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
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "compiler.h"
#include "libvxcli/libvxcli.h"
#include "libvxutil/defines.h"

// Functions
static inline int big_prompt(const struct vxcq_entry *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL char *vxcli_query(const char *msg, const char *prompt,
 const char *defl, long opts, char *buf, size_t size)
{
    hmc_t *answer = NULL;
    int i = 0;

    if(msg != NULL)
        printf("%s", msg);
    if(opts & VXCQ_ABORT) {
        ++i;
        printf("(Use <CTRL+A>,<Enter> to abort) ");
    }
    if(defl != NULL) {
        ++i;
        printf("(Leave blank to use the default)");
        if(opts & VXCQ_EMPTY)
            printf(" (Use <CTRL+E>,<Enter> to supply an empty answer)");
    }
    if(i)
        printf("\n");

    while(1) {
        if(prompt != NULL) printf("%s ", prompt);
        if(defl != NULL)   printf("[%s] ", defl);
        printf("> ");
        fflush(stdout);

        if(HX_getl(&answer, stdin) == NULL)
            return NULL;
        HX_chomp(answer);

        if((opts & VXCQ_ABORT) && answer[0] == '\x01' && answer[1] == '\0') {
            hmc_free(answer);
            return NULL;
        }

        if(*answer == '\0') { // use default
            if(defl != NULL) {
                if(buf == NULL) {
                    buf = HX_strdup(defl);
                    break;
                }
                if(defl == buf)
                    break;
            } else if(!(opts & VXCQ_EMPTY)) {
                printf("Answer may not be empty!\n");
                continue;
            }
            // empty answer, no default, may be empty => take it
        }

        if((opts & VXCQ_EMPTY) && answer[0] == '\x05' && answer[1] == '\0')
            *answer = '\0';

        if(buf == NULL)
            buf = HX_strdup(answer);
        else
            HX_strlcpy(buf, answer, size);

        break;
    }

    hmc_free(answer);
    return buf;
}

EXPORT_SYMBOL int vxcli_query_v(const struct vxcq_entry *tp)
{
    unsigned int count = 0;

    while(tp->msg != NULL || tp->prompt != NULL) {
        int big_prev = big_prompt(tp);
        int big_next;

        if(tp->type == HXTYPE_STRING) {
            char *res = vxcli_query(tp->msg, tp->prompt, tp->defl,
                        tp->flags, NULL, 0);

            if(res == NULL)
                return count;

            if((tp->flags & VXCQ_ZNULL) && *res == '\0')
                *static_cast(char **, tp->ptr) = NULL;
            else
                *static_cast(char **, tp->ptr) = res;

            if(tp->validate != NULL && !tp->validate(tp)) {
                free(res);
                continue;
            }
        } else {
            char buf[MAXFNLEN];

            if(vxcli_query(tp->msg, tp->prompt, tp->defl, tp->flags,
             buf, sizeof(buf)) == NULL)
                return count;

            if(!(tp->flags & VXCQ_ZNULL) || *buf != '\0') {
                if(tp->type == HXTYPE_INT)
                    *static_cast(int *, tp->ptr) = strtol(buf, NULL, 0);
                else if(tp->type == HXTYPE_LONG)
                    *static_cast(long *, tp->ptr) = strtol(buf, NULL, 0);
                else if(tp->type == HXTYPE_DOUBLE)
                    *static_cast(double *, tp->ptr) = strtod(buf, NULL);
                else
                    fprintf(stderr, "%s: Unknown type\n", __FUNCTION__);

                if(tp->validate != NULL && !tp->validate(tp))
                    continue;
            }
        }

        big_next = big_prompt(++tp);
        if(big_prev ^ big_next)
            printf("\n");
        ++count;
    }
    return 1;
}

//-----------------------------------------------------------------------------
static inline int big_prompt(const struct vxcq_entry *e) {
    return e->msg != NULL || e->defl != NULL ||
           (e->flags & (VXCQ_ABORT | VXCQ_EMPTY));
}

//=============================================================================
