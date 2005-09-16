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
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libHX.h>
#include "cui/data.h"
#include "cui/global.h"
#include "cui/main.h"

enum {
    CLASS_NONE = 0,
    CLASS_DIGIT,
    CLASS_ALPHA,
    CLASS_OTHER,
    /* compare_wbc() compares by class, then by context (i.e. numerically for
    CLASS_DIGIT and lexically for CLASS_ALPHA and CLASS_OTHER). The context
    with the smallest enumeration (see above) is considered less - in all
    cases (when two segments are of different class). */
};

#ifdef WIN32
static inline struct tm *localtime_r(const time_t *, struct tm *);
#endif
static inline unsigned int min_uint(unsigned int, unsigned int);
static char *transform7(const char *, char *, size_t);
static int wb_classof(char);

static const struct stab {
    const char *in, *out;
} subst_tab[] = {
    {"ß", "ss"},
    {"à", "a"}, {"á", "a"}, {"â", "a"}, {"ã", "a"}, {"ä", "ae"}, {"å", "a"}, {"æ", "a"},
    {"è", "e"}, {"é", "e"}, {"ê", "e"}, {"ë", "e"},
    {"ì", "i"}, {"í", "i"}, {"î", "i"}, {"ï", "i"},
    {"ò", "o"}, {"ó", "o"}, {"ô", "o"}, {"õ", "o"}, {"ö", "oe"}, {"ø", "o"},
    {"ù", "u"}, {"ú", "u"}, {"û", "u"}, {"ü", "ue"},
    {"À", "a"}, {"Á", "a"}, {"Â", "a"}, {"Ã", "a"}, {"Ä", "ae"}, {"Å", "a"}, {"Æ", "ae"},
    {"È", "e"}, {"É", "e"}, {"Ê", "e"}, {"Ë", "e"},
    {"Ì", "i"}, {"Í", "i"}, {"Î", "i"}, {"Ï", "i"},
    {"Ò", "o"}, {"Ó", "o"}, {"Ô", "o"}, {"Õ", "o"}, {"Ö", "oe"}, {"Ø", "o"},
    {"Ù", "u"}, {"Ú", "u"}, {"Û", "u"}, {"Ü", "ue"},
    {"ç", "c"}, {"Ç", "c"},
    {"Ñ", "n"}, {"ñ", "n"},
    {NULL},
};

//-----------------------------------------------------------------------------
char *day_to_xuid(const char *src, char *dest, size_t count) {
    /* A XUID (external user id) can have a format identifier which a
    underscore must follow. */
    int day = 0, month = 0, year = 0;

    if(strchr(src, '.') != NULL) {
        // Europe style
        sscanf(src, "%d.%d.%d", &day, &month, &year);
    } else if(strchr(src, '-') != NULL) {
        // ISO 8601 format string
        sscanf(src, "%d-%d-%d", &year, &month, &day);
    } else if(strchr(src, '/') != NULL) {
        // American style
        sscanf(src, "%d/%d/%d", &month, &day, &year);
    } else {
        return NULL;
    }

    if(day == 0 || month == 0 || year == 0) {
        /* If any of the three are zero, the input date is illegal. If all
        three are zero, the equivalent meaning is "no date given". In both
        cases, NULL is returned. */
        return NULL;
    }

    if(year >= 100 && year < 1000) {
        year += 1900;
    } else if(year < 100) {
        /* Fix two-digit year numbers. The range of two-digit years is always
        [now-50yrs to now+50yrs]. */
        struct tm res;
        time_t now = time(NULL);
        int bp, nc;

        localtime_r(&now, &res);
        bp = (res.tm_year + 50) % 100;
        nc = res.tm_year - res.tm_year % 100 + ((bp < 50) ? 100 : 0);
        year += 1900 + ((year > bp) ? (nc - 100) : nc);
    }

    // Scrambling version (currently 0) is hardcoded...
    if(dest == NULL) {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), "0_%03X%01X%02X", year, month, day);
        return HX_strdup(tmp);
    } else {
        snprintf(dest, count, "0_%03X%01X%02X", year, month, day);
        return dest;
    }
    return NULL;
}

int compare_wbc(const char *str_a, const char *str_b) {
    /* Split both strings at word boundaries and compare each fragment pair
    according to context (numerical or lexically). (They are not really split,
    but it mimics Perl's regular \b expression.)

    A boundary in compare_wbc() is between two characters which are of
    different class. Classes are: '\0', [0-9], [a-z], [^0-9a-z]. (Thus, the
    following would hold true:
    11C < 12C, 8EK < 8F2, 10A > 9B, 9 < A2, B > A2, 0 < '-') */

    const char *walk_a = str_a, *walk_b = str_b;
    int cl_a, cl_b;

    cl_a = wb_classof(*walk_a++);
    cl_b = wb_classof(*walk_b++);
    if(cl_a != cl_b) { return cl_a - cl_b; }

    /* The clever analyst saw that cl_a == cl_b, thus being able to reduce
    cl_a == CLASS_NONE && cl_b == CLASS_NONE into...: */
    if(cl_a == CLASS_NONE) { return 0; }

    while(1) {
        char buf_a[MAXSNLEN], buf_b[MAXSNLEN];
        int d;

        // Find the next segment start
        while(wb_classof(*walk_a) == cl_a) { ++walk_a; }
        while(wb_classof(*walk_b) == cl_b) { ++walk_b; }

        // Compare current segments' content
        d = walk_a - str_a;
        buf_a[d] = '\0';
        strncpy(buf_a, str_a, d);

        d = walk_b - str_b;
        buf_b[d] = '\0';
        strncpy(buf_b, str_b, d);

        if(cl_a == CLASS_DIGIT) {
            int va = strtoul(buf_a, NULL, 10), vb = strtoul(buf_b, NULL, 10);
            if(va != vb) { return va - vb; }
        } else if(cl_a == CLASS_ALPHA || cl_a == CLASS_OTHER) {
            if((d = strcmp(buf_a, buf_b)) != 0) { return d; }
        } else if(cl_a == CLASS_NONE) {
            return 0;
        }

        // Current segment was equal [again], so try next segment.
        cl_a = wb_classof(*walk_a);
        cl_b = wb_classof(*walk_b);
        if(cl_a != cl_b) { return cl_a - cl_b; }
        if(cl_a == CLASS_NONE) { return 0; }

        str_a = walk_a;
        str_b = walk_b;
    }

    return 0;
}

void complete_pentry(struct eds_entry *entry, char *key, size_t count) {
    /* Takes a ds_entry and fills in .lname, .rname from .vname and .nname.
    Also generates a unique user string, put into KEY. */
    entry->lname = make_login_name(entry->nname, entry->vname);

    if(entry->nname != NULL) {
        // using "key" as a temporary buffer
        snprintf(key, count, "%s %s", entry->vname, entry->nname);
        entry->rname = HX_strdup(key);
    } else {
        entry->rname = HX_strdup(entry->vname);
    }

    snprintf(key, count, "%s\x01%s", entry->rname,
     (entry->xuid != NULL) ? entry->xuid : "");
    return;
}

int file_exists(const char *fn) {
    struct stat sb;
    return stat(fn, &sb) == 0;
}

char *fixup_login_name(const char *ln, unsigned int idx, char *dest,
 size_t dsize) {
    /* Login names are fixated by adding a unique number them,
    necessarily truncating part of the original login name. */
    char idxstr[16], tln[16];
    snprintf(idxstr, 16, "%u", idx);
    snprintf(tln, 8 - strlen(idxstr) + 1, "%s", ln);
    snprintf(dest, dsize, "%s%s", tln, idxstr);
    return dest;
}

char *make_login_name(const char *in_nname, const char *in_vname) {
    /* This takes the name (in_nname) and prenom (in_vname) and makes a 7-bit
    clean login name in the style VNNNNNN. Numerical indexes are done in
    fixup_login_name() (called from the main code). */
    char buf[8], fix_nname[8], fix_vname[8], *p;

    transform7(in_vname, fix_vname, 8);

    if(in_nname != NULL) {
        if((p = strchr(in_nname, ' ')) != NULL) { in_nname = ++p; }
        transform7(in_nname, fix_nname, 8);
        snprintf(buf, 8, "%c%s", *fix_vname, fix_nname);
    } else {
        snprintf(buf, 8, "%s", fix_vname);
    }

    if(strlen(buf) < 2) {
        // In case the login name is too short, for whatever reason, append a 0
        strcat(buf, "0");
    }
    return HX_strdup(HX_strlower(buf));
}

long now_in_days(void) {
    return time(NULL) / 86400;
}

#ifdef _WIN32
// A dirty hack, since WIN32 does not provide localtime_r() yet.
static inline struct tm *localtime_r(const time_t *timep, struct tm *result) {
    struct tm *statik = localtime(timep);
    memcpy(result, statik, sizeof(struct tm));
    return result;
}
#endif

char *now_in_ymdhms(char *buf, size_t count) {
    // Write the current time as YYMMDD-HHMMSS into "buf"
    time_t now = time(NULL);
    struct tm res;
    strftime(buf, MAXSNLEN, "%Y%m%d-%H%M%S", localtime_r(&now, &res));
    return buf;
}

flag_t only_digits(const char *p) {
    // Returns whether the string in P consists only of digits
    while(*p != '\0') {
        if(!isdigit(*p++)) { return 0; }
    }
    return 1;
}

char *read_file(const char *fn) {
    struct stat sb;
    char *dst;
    FILE *fp;

    if((fp = fopen(fn, "rb")) == NULL || fstat(fileno(fp), &sb) != 0 ||
     (dst = calloc(1, sb.st_size + 1)) == NULL) {
        return NULL;
    }

    if(fread(dst, sb.st_size, 1, fp) < 1) {
        free(dst);
        return NULL;
    }

    fclose(fp);
    return dst;
}

//-----------------------------------------------------------------------------
static inline unsigned int min_uint(unsigned int a, unsigned int b) {
    return (a < b) ? a : b;
}

static char *transform7(const char *src, char *dest, size_t dsize) {
    /* Transform a name (possibly with umlauts, etc.) into a 7-bit clean,
    A-to-Z-only login name. */
    char *od = dest;
    --dsize;
    while(dsize && *src != '\0') {
        if((*src >= 'A' && *src <= 'Z') || (*src >= 'a' && *src <= 'z')) {
            *dest++ = *src++;
            --dsize;
        } else if(*(unsigned char *)src >= 128) {
            const struct stab *sp = subst_tab;
            int ok = 0;
            while(sp->in != NULL) {
                if(strstr(src, sp->in) == src) {
                    size_t l = strlen(sp->out);
                    if(dsize >= l) {
                        strncpy(dest, sp->out, l);
                        src   += strlen(sp->in);
                        dest  += l;
                        dsize -= l;
                        ++ok;
                        break;
                    } else {
                        /* Do not let "abcdeöa" through, because if there was
                        too few space, the "ö" gets skipped but dsize is still
                        big enough to let the "a" in. */
                        dsize = 0;
                    }
                    break;
                }
                ++sp;
            }
            // Just skip things we can not transform
            if(!ok) { ++src; }
        } else {
            ++src;
        }
    }
    *dest = '\0';
    return od;
}

static int wb_classof(char x) {
    if(isalpha(x)) { return CLASS_ALPHA; }
    if(isdigit(x)) { return CLASS_DIGIT; }
    if(x == '\0') { return CLASS_NONE; }
    return CLASS_OTHER;
}

//=============================================================================
