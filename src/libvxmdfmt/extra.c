/*
    libvxmdfmt/extra.c
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
*/
#include <ctype.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vitalnix/compiler.h>
#include "libvxmdfmt/internal.h"
#include <vitalnix/libvxutil/defines.h>

// Definitions
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

// Functions
static inline int wb_classof(char);

//-----------------------------------------------------------------------------
/*  compare_wbc - Compare two strings on a per-class basis

    A boundary in compare_wbc() is between two characters which are of
    different class. Classes are: '\0', [0-9], [a-z], [^0-9a-z]. (Thus, the
    following would hold true:

        "11C" < "12C",
        "8EK" < "8F2",
        "10A" > "9B",
        "9" < "A2",
        "B" > "A2",
        "0" < "-"

    Returns an integer less than, equal to, or greater than zero if A is found,
    respectively, to be less than, to match, or be greater than B.
*/
int compare_wbc(const char *str_a, const char *str_b) {
    const char *walk_a = str_a, *walk_b = str_b;
    int cl_a, cl_b;

    cl_a = wb_classof(*walk_a++);
    cl_b = wb_classof(*walk_b++);
    if(cl_a != cl_b)
        return cl_a - cl_b;

    /* cl_a == cl_b, therefore (cl_a == CLASS_NONE && cl_b == CLASS_NONE) is
    redundant and allows for: */
    if(cl_a == CLASS_NONE)
        return 0;

    while(1) {
        char buf_a[MAXSNLEN], buf_b[MAXSNLEN];
        int d;

        // Find the next segment start
        while(wb_classof(*walk_a) == cl_a) ++walk_a;
        while(wb_classof(*walk_b) == cl_b) ++walk_b;

        // Compare current segments' content
        d = walk_a - str_a;
        strncpy(buf_a, str_a, d);
        buf_a[d] = '\0';

        d = walk_b - str_b;
        strncpy(buf_b, str_b, d);
        buf_b[d] = '\0';

        if(cl_a == CLASS_DIGIT) {
            int va = strtoul(buf_a, NULL, 10), vb = strtoul(buf_b, NULL, 10);
            if(va != vb)
                return va - vb;
        } else if(cl_a == CLASS_ALPHA || cl_a == CLASS_OTHER) {
            if((d = strcmp(buf_a, buf_b)) != 0)
                return d;
        } else if(cl_a == CLASS_NONE) {
            return 0;
        }

        // Current segment was equal [again], so try next segment.
        cl_a = wb_classof(*walk_a);
        cl_b = wb_classof(*walk_b);
        if(cl_a != cl_b)
            return cl_a - cl_b;
        if(cl_a == CLASS_NONE)
            return 0;

        str_a = walk_a;
        str_b = walk_b;
    }

    return 0;
}

struct HXbtree *defcat_file_header(const struct pwlfmt_workspace *ws)
{
	struct HXbtree *ret;
	if((ret = HXformat_init()) == NULL)
		return NULL;
	HXformat_add(ret, "INFILE",  ws->input_file,   HXTYPE_STRING);
	HXformat_add(ret, "OUTFILE", ws->output_file,  HXTYPE_STRING);
	HXformat_add(ret, "VERSION", VITALNIX_VERSION, HXTYPE_STRING);
	return ret;
}

struct HXbtree *defcat_tbl_header(const struct pwlfmt_workspace *ws,
    const struct pwl_data *data)
{
	struct HXbtree *ret;
	if((ret = HXformat_init()) == NULL)
		return NULL;
	HXformat_add(ret, "PVGROUP", data->pvgrp,      HXTYPE_STRING);
	HXformat_add(ret, "VERSION", VITALNIX_VERSION, HXTYPE_STRING);
	return ret;
}

struct HXbtree *defcat_tbl_entry(const struct pwlfmt_workspace *ws,
    const struct pwl_data *data)
{
	struct HXbtree *ret;
	if((ret = HXformat_init()) == NULL)
		return NULL;
	HXformat_add(ret, "PVGROUP",   data->pvgrp,      HXTYPE_STRING);
	HXformat_add(ret, "SURNAME",   data->surname,    HXTYPE_STRING);
	HXformat_add(ret, "PASSWORD",  data->password,   HXTYPE_STRING);
	HXformat_add(ret, "USERNAME",  data->username,   HXTYPE_STRING);
	HXformat_add(ret, "FIRSTNAME", data->first_name, HXTYPE_STRING);
	HXformat_add(ret, "VERSION",   VITALNIX_VERSION, HXTYPE_STRING);
	return ret;
}

int pwlfmt_extra_whitespace(const char *s) {
    size_t in_size = strlen(s) + 1,
          out_size = in_size * sizeof(wchar_t);
    iconv_t cd     = iconv_open("wchar_t", "UTF-8");
    int extra      = 0;
    wchar_t *tw;
    void *tmp;

    if((tmp = malloc(out_size + 1)) == NULL)
        return 0;

    tw = tmp;
    iconv(cd, reinterpret_cast(char **, &s), &in_size,
          reinterpret_cast(char **, &tw), &out_size);

    tw = tmp;
    while(*tw != 0)
        if(*tw++ >= 0x80)
            ++extra;

    free(tmp);
    iconv_close(cd);
    return extra;
}

//-----------------------------------------------------------------------------
static inline int wb_classof(char x) {
    if(isalpha(x)) return CLASS_ALPHA;
    if(isdigit(x)) return CLASS_DIGIT;
    if(x == '\0')  return CLASS_NONE;
    return CLASS_OTHER;
}

//=============================================================================
