/*=============================================================================
Vitalnix User Management Suite
libvxmdfmt/s_pgrtf.c
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
#include <errno.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "libvxmdfmt/internal.h"
#include "libvxmdfmt/libvxmdfmt.h"
#include "libvxmdfmt/static-build.h"
#include "libvxmdfmt/vtable.h"
#include "libvxutil/libvxutil.h"

// Structures
struct pgrtf_data {
    char *template_data;
    const char *tps_file_header, *tps_tbl_header, *tps_tbl_entry,
               *tps_tbl_footer, *tps_file_footer;
};

// Functions
static int pgrtf_construct(struct pwlfmt_workspace *);
static void pgrtf_destruct(struct pwlfmt_workspace *);
static printfunc_t pgrtf_file_header, pgrtf_tbl_header, pgrtf_tbl_entry,
                   pgrtf_tbl_footer, pgrtf_file_footer;
static int pgrtf_read_template(const char *, struct pgrtf_data *);
static hmc_t *utf8_to_rtfuni(const char *);

// Variables
static const struct pwlstyle_vtable THIS_STYLE = {
    .name             = "pg_rtf",
    .desc             = "pvgrp-sorted text/rtf",
    .require_template = 1,

    .init             = pgrtf_construct,
    .exit             = pgrtf_destruct,
    .file_header      = pgrtf_file_header,
    .tbl_header       = pgrtf_tbl_header,
    .tbl_entry        = pgrtf_tbl_entry,
    .tbl_footer       = pgrtf_tbl_footer,
    .file_footer      = pgrtf_file_footer,
};

//-----------------------------------------------------------------------------
REGISTER_MODULE(pg_rtf, &THIS_STYLE);

static int pgrtf_construct(struct pwlfmt_workspace *w) {
    struct pgrtf_data priv;
    int ret;

    if((ret = pgrtf_read_template(w->template_file, &priv)) < 0)
        return ret;
    if((w->style_data = HX_memdup(&priv, sizeof(priv))) == NULL) {
        free(priv.template_data);
        return -errno;
    }

    return 1;
}

static void pgrtf_destruct(struct pwlfmt_workspace *w) {
    free(w->style_data);
    return;
}

static void pgrtf_file_header(struct pwlfmt_workspace *w,
 const struct pwl_data *data)
{
    const struct pgrtf_data *p = w->style_data;
    DEFCAT_FILE_HEADER(w, data);

    HX_fstrrep(w->output_fh, p->tps_file_header, catalog);
    return;
}

static void pgrtf_tbl_header(struct pwlfmt_workspace *w,
 const struct pwl_data *data)
{
    const struct pgrtf_data *p = w->style_data;
    DEFCAT_TBL_HEADER(w, data);

    HX_fstrrep(w->output_fh, p->tps_tbl_header, catalog);
    return;
}

static void pgrtf_tbl_entry(struct pwlfmt_workspace *w,
 const struct pwl_data *data)
{
    const struct pgrtf_data *p = w->style_data;
    hmc_t *uni_firstname       = utf8_to_rtfuni(data->first_name);
    hmc_t *uni_surname         = utf8_to_rtfuni(data->surname);

    struct HXoption catalog[] = {
        {.sh = PH_PVGROUP,   .type = HXTYPE_STRING, .ptr = (void *)data->pvgrp},
        {.sh = PH_SURNAME,   .type = HXTYPE_STRING, .ptr = uni_surname},
        {.sh = PH_PASSWORD,  .type = HXTYPE_STRING, .ptr = (void *)data->password},
        {.sh = PH_USERNAME,  .type = HXTYPE_STRING, .ptr = (void *)data->username},
        {.sh = PH_FIRSTNAME, .type = HXTYPE_STRING, .ptr = uni_firstname},
        HXOPT_TABLEEND,
    };

    HX_fstrrep(w->output_fh, p->tps_tbl_entry, catalog);
    hmc_free(uni_surname);
    hmc_free(uni_firstname);
    return;
}

static void pgrtf_tbl_footer(struct pwlfmt_workspace *w,
 const struct pwl_data *data)
{
    const struct pgrtf_data *p = w->style_data;
    fprintf(w->output_fh, p->tps_tbl_footer);
    return;
}

static void pgrtf_file_footer(struct pwlfmt_workspace *w,
 const struct pwl_data *data)
{
    const struct pgrtf_data *p = w->style_data;
    fprintf(w->output_fh, p->tps_file_footer);
    return;
}

//-----------------------------------------------------------------------------
static int pgrtf_read_template(const char *template_file,
 struct pgrtf_data *p)
{
    char *ptr;

    if(template_file == NULL)
        return -EFAULT;
    if((p->template_data = vxutil_slurp_file(template_file)) == NULL)
        return -errno;

    /* The following fixup is so that the RTF file can be edited more easily.
    And of course, we keep it in one file, which is essential. Microsoft Office
    successfully ignores our private tags, so that we can check out what our
    template looks like without having to do guesses. */
    ptr                = p->template_data;
    p->tps_file_header = HX_strsep2(&ptr, "{\\THDR}");
    p->tps_tbl_header  = HX_strsep2(&ptr, "{\\ENTRY}");
    p->tps_tbl_entry   = HX_strsep2(&ptr, "{\\TFOOT}");
    p->tps_tbl_footer  = HX_strsep2(&ptr, "{\\FOOTER}");
    p->tps_file_footer = ptr;
    if(p->tps_file_header == NULL) p->tps_file_header = "";
    if(p->tps_tbl_header  == NULL) p->tps_tbl_header  = "";
    if(p->tps_tbl_entry   == NULL) p->tps_tbl_entry   = "";
    if(p->tps_tbl_footer  == NULL) p->tps_tbl_footer  = "";
    if(p->tps_file_footer == NULL) p->tps_file_footer = "";
    return 1;
}

static hmc_t *utf8_to_rtfuni(const char *ip) {
    const char *cfh = ip;
    iconv_t cd      = iconv_open("wchar_t", "UTF-8");
    hmc_t *dest     = hmc_minit(NULL, 0);
    char buf[16];
    size_t is, os;
    wchar_t oc, *op = &oc;

    while(*ip != '\0') {
        if(*(const unsigned char *)ip < 0x80) {
            ++ip;
            continue;
        }

        is = strlen(ip);
        os = sizeof(wchar_t);
        hmc_memcat(&dest, cfh, ip - cfh);
        iconv(cd, (void *)&ip, &is, (void *)&op, &os);
        snprintf(buf, sizeof(buf), "\\uc0\\u%ld", (long)oc);
        hmc_strcat(&dest, buf);
        cfh = ip;
    }

    if(*cfh != '\0')
        hmc_strcat(&dest, cfh);
    return dest;
}

//=============================================================================
