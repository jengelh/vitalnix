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
#include <errno.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libHX.h>
#include "cui/data.h"
#include "cui/xml_in.h"
#define ICONV_NULL ((iconv_t)(-1))

struct eds_state {
    int type;
    void *sub;
};

struct sdfds_state {
    char *file;
    FILE *fp;
    int line;
    iconv_t cd;
};

struct xmlds_state {
    xmlDoc *doc;
    xmlNode *ptr;
    iconv_t cd;
};

static int sdfds_open(const char *, void **);
static int sdfds_read(void *, struct eds_entry *);
static void sdfds_close(void *);
static int xmlds_open(const char *, void **);
static int xmlds_read(void *, struct eds_entry *);
static void xmlds_close(void *);

static char *convert(iconv_t, char *);
static inline int strcmp_1u(const unsigned char *, const char *);
static inline char *xmlGetProp_2s(xmlNode *, const char *);

static struct {
    int (*open)(const char *, void **);
    int (*read)(void *, struct eds_entry *);
    void (*close)(void *);
} eds_table[] = {
    [EDSTYPE_NONE] = {NULL,       NULL,       NULL},
    [EDSTYPE_SDF]  = {sdfds_open, sdfds_read, sdfds_close},
    [EDSTYPE_XML]  = {xmlds_open, xmlds_read, xmlds_close},
};

//-----------------------------------------------------------------------------
int eds_open(const char *fn, void **state, int type) {
    struct eds_state *sp;
    int eax;

    if(type <= EDSTYPE_NONE || type >= EDSTYPE_MAX) {
        fprintf(stderr, "%s: Illegal filetype %d\n", __FUNCTION__, type);
        return -(errno = EINVAL);
    }

    if((sp = *state = malloc(sizeof(struct eds_state))) == NULL) {
        return -errno;
    }
    if((eax = eds_table[type].open(fn, &sp->sub)) <= 0) {
        free(sp);
        errno = -eax;
        return eax;
    }
    sp->type = type;
    return 1;
}

int eds_read(void *state, struct eds_entry *e) {
    struct eds_state *sp = state;
    return eds_table[sp->type].read(sp->sub, e);
}

void eds_close(void *state) {
    struct eds_state *sp = state;
    eds_table[sp->type].close(sp->sub);
    free(sp);
    return;
}

int eds_derivefromname(const char *fn) {
    size_t len = strlen(fn);
    if(len < 4) { return EDSTYPE_NONE; }
    if(strcmp(fn + len - 4, ".sdf") == 0) {
        return EDSTYPE_SDF;
    } else if(strcmp(fn + len - 4, ".xml") == 0) {
        return EDSTYPE_XML;
    }
    return EDSTYPE_NONE;
}

//-----------------------------------------------------------------------------
static int sdfds_open(const char *fn, void **state) {
    struct sdfds_state *sp;

    if((sp = *state = calloc(1, sizeof(struct sdfds_state))) == NULL) {
        return -errno;
    }

    if((sp->cd = iconv_open("utf-8", "cp437")) == ICONV_NULL) {
        free(sp);
        return -errno;
    }

    if((sp->fp = fopen(fn, "r")) == NULL) {
        fprintf(stderr, "%s: Could not open %s: %s\n",
         __FUNCTION__, fn, strerror(errno));
        free(sp);
        return -errno;
    }

    sp->file = HX_strdup(fn);
    return 1;
}

static int sdfds_read(void *state, struct eds_entry *e) {
    struct sdfds_state *sp = state;
    char tmp[64], *data[5], *ln, *hmc = NULL;
    int num_fields;

    if(HX_getl(&hmc, sp->fp) == NULL) { return 0; }
    ++sp->line;
    HX_chomp(hmc);

    if(*hmc == '#' || strncmp(hmc, "(*", 2) == 0 || *hmc == '\0') { // *)
        // Skip comment line
        return sdfds_read(state, e);
    }
    ln = convert(sp->cd, hmc);
    hmc_free(hmc);

    if((num_fields = HX_split5(ln, ";", 5, data)) < 3) {
        fprintf(stderr, "%s: SDF-4 format: Line %d has less than 3 fields,"
         " skipping!\n", __FUNCTION__, sp->line);
        free(ln);
        return sdfds_read(state, e);
    }

    if(only_digits(data[0])) {
        /* SDF style from Max-Plank-Gymnasium has another (though, unimportant)
        field at the front. */
        if(--num_fields < 3) {
            fprintf(stderr, "%s: SDF-5 format: Line %d has less than 4"
             " fields, skipping!\n", __FUNCTION__, sp->line);
            free(ln);
            return sdfds_read(state, e);
        }
        memmove(&data[0], &data[1], 4 * sizeof(char *));
    }

    e->nname  = HX_strdup(data[0]);
    e->vname  = HX_strdup(data[1]);
    e->xuid   = HX_strdup(day_to_xuid(data[2], tmp, sizeof(tmp)));
    e->sgroup = HX_strdup(data[3]);
    free(ln);
    return 1;
}

static void sdfds_close(void *state) {
    struct sdfds_state *sp = state;
    if(sp->cd != ICONV_NULL) { iconv_close(sp->cd); }
    if(sp->fp != NULL) { fclose(sp->fp); }
    if(sp->file != NULL) { free(sp->file); }
    return;
}

//-----------------------------------------------------------------------------
static int xmlds_open(const char *fn, void **state) {
    struct xmlds_state *sp;
    if((sp = *state = calloc(1, sizeof(struct xmlds_state))) == NULL) {
        return -errno;
    }

    xmlKeepBlanksDefault(0);

    /* xmlParseFile() already scans the file completely and stores the
    structure tree plus components in memory. */
    if((sp->doc = xmlParseFile(fn)) == NULL) {
        xmlds_close(*state);
        return 0;
    }

    if((sp->ptr = xmlDocGetRootElement(sp->doc)) == NULL ||
     strcmp_1u(sp->ptr->name, "datasource") != 0) {
        xmlds_close(*state);
        return 0;
    }

    sp->ptr = sp->ptr->children;
    return 1;
}

static int xmlds_read(void *state, struct eds_entry *x) {
    /* The reading process is very simple, since libxml takes all the pain to
    parse the file down, and its API directly provides us with the stuff we
    want. */
    struct xmlds_state *sp = state;
    if(sp->ptr == NULL) { return 0; }
    if(strcmp_1u(sp->ptr->name, "user") == 0) {
        x->vname = xmlGetProp_2s(sp->ptr, "vname");

        if(x->vname == NULL) { // bogus entry
            sp->ptr = sp->ptr->next;
            return -EINVAL;
        }
        if(*x->vname == '\0') { // bogus entry
            free(x->vname);
            sp->ptr = sp->ptr->next;
            return -EINVAL;
        }

        x->nname  = xmlGetProp_2s(sp->ptr, "nname");
        x->sgroup = xmlGetProp_2s(sp->ptr, "sgroup");
        x->xuid   = xmlGetProp_2s(sp->ptr, "xuid");
    }

    sp->ptr = sp->ptr->next;
    return 1;
}

static void xmlds_close(void *state) {
    struct xmlds_state *sp = state;
    if(sp->doc != NULL) { xmlFreeDoc(sp->doc); }
    xmlCleanupParser();
    if(sp->cd != ICONV_NULL) { iconv_close(sp->cd); }
    free(sp);
    return;
}

//-----------------------------------------------------------------------------
static char *convert(iconv_t cd, char *in) {
    char *inp = in, *out, *outp;
    size_t inl, outl;

    if(in == NULL || *in == '\0') {
        return NULL;
    }

    inl  = strlen(inp) + 1;
    outl = 3 * inl;
    if((outp = out = malloc(outl)) == NULL) {
        return NULL;
    }
    iconv(cd, &inp, &inl, &outp, &outl);
    return out;
}

static inline int strcmp_1u(const unsigned char *a, const char *b) {
    return strcmp((const char *)a, b);
}

static inline char *xmlGetProp_2s(xmlNode *p, const char *v) {
    return (char *)xmlGetProp(p, (const xmlChar *)v);
}

//=============================================================================
