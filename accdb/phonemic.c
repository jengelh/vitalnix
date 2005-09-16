/*=============================================================================
Vitalnix User Management Suite

    Password generation function utilizing phonemic rules to generate secure,
    but pronouncible and easy-memorizable passwords.

    From http://heanet.dl.sf.net/sourceforge/pwgen/pwgen-2.03.tar.bz2
    Copyright (C) 2001, 2002 by Theodore Ts'o <tytso [at] mit edu>
    This file may be distributed under the terms of the GNU Public License. The
    code was taken from pwgen-2.03/pw_phonemes.c and has been placed into the
    Vitalnix tree. It is also altered. The original author has nothing to do
    with this here, so do not bug him if you find a bug.

    NOTICE:
    Including this file (which is GPL) in the linking process of libaccdb.so
    makes libaccdb.so (which is normally LGPL), AS WELL AS THE PROGRAMS (ALSO
    LGPL) USING THAT COMPILATION OF LIBACCDB ENTIRELY GPL. (IIRC, a so-called
    Combined Work.) For details, ask the FSF or your lawyer.
=============================================================================*/
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <libHX.h>
#include "include/accdb.h"

#define NUM_ELEMENTS (sizeof(phonemic_element) / sizeof(struct pw_element))
#define rnd(__number) HX_irand(randp, 0, (__number))

enum el_flags {
    FL_CONSONANT = 1 << 0,
    FL_VOWEL     = 1 << 1,
    FL_DIPTHONG  = 1 << 2,
    FL_NOT_FIRST = 1 << 3,
};

static const struct pw_element {
    const char *str;
    const enum el_flags flags;
} phonemic_element[] = {
    {"a",  FL_VOWEL},
    {"ae", FL_DIPTHONG | FL_VOWEL},
    {"ah", FL_DIPTHONG | FL_VOWEL},
    {"ai", FL_DIPTHONG | FL_VOWEL},
    {"b",  FL_CONSONANT},
    {"c",  FL_CONSONANT},
    {"ch", FL_CONSONANT | FL_DIPTHONG},
    {"d",  FL_CONSONANT},
    {"e",  FL_VOWEL},
    {"ee", FL_DIPTHONG | FL_VOWEL},
    {"ei", FL_DIPTHONG | FL_VOWEL},
    {"f",  FL_CONSONANT},
    {"g",  FL_CONSONANT},
    {"gh", FL_CONSONANT | FL_DIPTHONG | FL_NOT_FIRST},
    {"h",  FL_CONSONANT},
    {"i",  FL_VOWEL},
    {"ie", FL_DIPTHONG | FL_VOWEL},
    {"j",  FL_CONSONANT},
    {"k",  FL_CONSONANT},
    {"l",  FL_CONSONANT},
    {"m",  FL_CONSONANT},
    {"n",  FL_CONSONANT},
    {"ng", FL_CONSONANT | FL_DIPTHONG | FL_NOT_FIRST},
    {"o",  FL_VOWEL},
    {"oh", FL_DIPTHONG | FL_VOWEL},
    {"oo", FL_DIPTHONG | FL_VOWEL},
    {"p",  FL_CONSONANT},
    {"ph", FL_CONSONANT | FL_DIPTHONG},
    {"qu", FL_CONSONANT | FL_DIPTHONG},
    {"r",  FL_CONSONANT},
    {"s",  FL_CONSONANT},
    {"sh", FL_CONSONANT | FL_DIPTHONG},
    {"t",  FL_CONSONANT},
    {"th", FL_CONSONANT | FL_DIPTHONG},
    {"u",  FL_VOWEL},
    {"v",  FL_CONSONANT},
    {"w",  FL_CONSONANT},
    {"x",  FL_CONSONANT},
    {"y",  FL_CONSONANT},
    {"z",  FL_CONSONANT},
};

//-----------------------------------------------------------------------------
int genpw_phonemic(char *buf, size_t size, unsigned long ft_flags,
 void *randp)
{
    enum el_flags flags, prev, should_be;
    const char *str;
    size_t c, len;
    int first, i;

 try_again:
    c         = 0;
    prev      = 0x0;
    should_be = 0x0;
    first     = 1;
    should_be = rnd(2) ? FL_VOWEL : FL_CONSONANT;

    while(c < size) {
        i     = rnd(NUM_ELEMENTS);
        str   = phonemic_element[i].str;
        len   = strlen(str);
        flags = phonemic_element[i].flags;

        // Filter on the basic type of the next element
        if((flags & should_be) == 0) { continue; }

        // Handle the FL_NOT_FIRST flag
        if(first && (flags & FL_NOT_FIRST)) { continue; }

        // Do not allow FL_VOWEL followed a FL_VOWEL/FL_DIPTHONG pair
        if((prev & FL_VOWEL) && (flags & (FL_VOWEL | FL_DIPTHONG)) ==
         (FL_VOWEL | FL_DIPTHONG)) {
            continue;
        }

        // Do not allow us to overflow the buffer
        if(len > size - c) { continue; }

        // OK, we found an element which matches our criteria, let 's do it!
        strcpy(buf + c, str);

        // Handle GENPW_ONE_CASE
        if((ft_flags & GENPW_ONE_CASE) &&
         (first || (flags & FL_CONSONANT)) && rnd(10) < 3) {
            buf[c] = toupper(buf[c]);
            ft_flags &= ~GENPW_ONE_CASE;
        }

        c += len;

        // Time to stop?
        if(c >= size) { break; }

        // Handle GENPW_ONE_DIGIT
        if((ft_flags & GENPW_ONE_DIGIT) && !first && rnd(10) < 3) {
            buf[c++] = rnd(10) + '0';
            buf[c] = '\0';
            ft_flags &= ~GENPW_ONE_DIGIT;
            first     = 1;
            prev      = 0x0;
            should_be = rnd(2) ? FL_VOWEL : FL_CONSONANT;
            continue;
        }

        // OK, figure out what the next element should be
        if(should_be == FL_CONSONANT) {
            should_be = FL_VOWEL;
        } else {
            // should_be == FL_VOWEL
            if((prev & FL_VOWEL) || (flags & FL_DIPTHONG) || rnd(10) > 3) {
                should_be = FL_CONSONANT;
            } else {
                should_be = FL_VOWEL;
            }
        }

        prev  = flags;
        first = 0;
    }

    if(ft_flags & (GENPW_ONE_CASE | GENPW_ONE_DIGIT)) {
        goto try_again;
    }
    return 1;
}

//=============================================================================
