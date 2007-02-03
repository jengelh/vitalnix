#ifndef _VITALNIX_LIBVXMDFMT_INTERNAL_H
#define _VITALNIX_LIBVXMDFMT_INTERNAL_H 1

#ifndef __cplusplus
#    include <stdio.h>
#else
#    include <cstdio>
#endif
#include <libHX.h>
#include <vitalnix/config.h>
#include "libvxmdfmt/static-build.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#    define DEFCAT_FILE_HEADER(t, p) \
        struct HXoption catalog[] = { \
            {.sh = PH_INFILE,  .type = HXTYPE_STRING, .ptr = (void *)(t)->input_file}, \
            {.sh = PH_OUTFILE, .type = HXTYPE_STRING, .ptr = (void *)(t)->output_file}, \
            {.sh = PH_VERSION, .type = HXTYPE_STRING, .ptr = VITALNIX_VERSION}, \
            HXOPT_TABLEEND, \
        };
#    define DEFCAT_TBL_HEADER(t, p) \
        struct HXoption catalog[] = { \
            {.sh = PH_PVGROUP, .type = HXTYPE_STRING, .ptr = (void *)(p)->pvgrp}, \
            {.sh = PH_VERSION, .type = HXTYPE_STRING, .ptr = VITALNIX_VERSION}, \
            HXOPT_TABLEEND, \
        };
#    define DEFCAT_TBL_ENTRY(t, p) \
        struct HXoption catalog[] = { \
            {.sh = PH_PVGROUP,   .type = HXTYPE_STRING, .ptr = (void *)(p)->pvgrp}, \
            {.sh = PH_SURNAME,   .type = HXTYPE_STRING, .ptr = (void *)(p)->surname}, \
            {.sh = PH_PASSWORD,  .type = HXTYPE_STRING, .ptr = (void *)(p)->password}, \
            {.sh = PH_USERNAME,  .type = HXTYPE_STRING, .ptr = (void *)(p)->username}, \
            {.sh = PH_FIRSTNAME, .type = HXTYPE_STRING, .ptr = (void *)(p)->first_name}, \
            {.sh = PH_VERSION,   .type = HXTYPE_STRING, .ptr = VITALNIX_VERSION}, \
            HXOPT_TABLEEND, \
        };
#endif

enum {
    PH_VERSION   = 'V',
    PH_PVGROUP   = 'g',
    PH_INFILE    = 'i',
    PH_OUTFILE   = 'o',
    PH_SURNAME   = 'n',
    PH_PASSWORD  = 'p',
    PH_USERNAME  = 'u',
    PH_FIRSTNAME = 'v',
};

struct pwl_data {
    const char *pvgrp, *first_name, *surname, *username, *password;
};

/*
 *      EXTRA.C
 */
extern int compare_wbc(const char *, const char *);
extern int pwlfmt_extra_whitespace(const char *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXMDFMT_INTERNAL_H

//=============================================================================
