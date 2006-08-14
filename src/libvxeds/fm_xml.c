/*=============================================================================
Vitalnix User Management Suite
libvxeds/fm_xml.c - XML parsing module
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include "compiler.h"
#include "libvxeds/libvxeds.h"
#include "libvxeds/static-build.h"
#include "libvxeds/vtable.h"
#include "libvxutil/libvxutil.h"

// Structures
struct xml_state {
    xmlDoc *doc;
    xmlNode *ptr;
};

// Functions
static int xml_open(const char *, void **);
static int xml_read(void *, struct vxeds_entry *);
static void xml_close(void *);
static inline int strcmp_1u(const xmlChar *, const char *);
static inline char *xmlGetProp_2s(xmlNode *, const char *);

// Variables
static const struct edsformat_vtable THIS_FORMAT = {
    .desc  = "Vitalnix XML Input Format",
    .ext   = "xml",
    .open  = xml_open,
    .read  = xml_read,
    .close = xml_close,
};

//-----------------------------------------------------------------------------
REGISTER_MODULE(xml, &THIS_FORMAT);

static int xml_open(const char *filename, void **state_pptr) {
    struct xml_state *state;

    if((state = *state_pptr = calloc(1, sizeof(struct xml_state))) == NULL)
        return -errno;

    //xmlKeepBlanksDefault(0);

    /* xmlParseFile() already scans the file completely and stores the
    structure tree plus components in memory. */
    if((state->doc = xmlParseFile(filename)) == NULL) {
        xml_close(*state_pptr);
        return 0;
    }

    if((state->ptr = xmlDocGetRootElement(state->doc)) == NULL ||
     strcmp_1u(state->ptr->name, "VX3_xmlds") != 0) {
        xml_close(*state_pptr);
        return 0;
    }

    state->ptr = state->ptr->children;
    return 1;
}

static int xml_read(void *state_ptr, struct vxeds_entry *e) {
    /* The reading process is very simple, since libxml takes all the pain to
    parse the file down, and its API directly provides us with the stuff we
    want. */
    struct xml_state *state = state_ptr;

    if(state->ptr == NULL)
        return 0;

    if(strcmp_1u(state->ptr->name, "user") == 0) {
        char *bd;

        memset(e, 0, sizeof(*e));
        e->first_name = xmlGetProp_2s(state->ptr, "firstname");

        if(e->first_name == NULL) { // bogus entry
            state->ptr = state->ptr->next;
            return -EINVAL;
        }

        e->surname   = xmlGetProp_2s(state->ptr, "surname");
        e->full_name = vxeds_bfullname(e->first_name, e->surname);
        e->pvgrp     = xmlGetProp_2s(state->ptr, "pvgrp");

        if((e->uuid = xmlGetProp_2s(state->ptr, "uuid")) == NULL)
            if((bd = xmlGetProp_2s(state->ptr, "bday")) != NULL)
                e->uuid = vxuuid_vx3(e->full_name, vxutil_string_iday(bd));
    }

    state->ptr = state->ptr->next;
    return 1;
}

static void xml_close(void *state_ptr) {
    struct xml_state *state = state_ptr;
    if(state->doc != NULL)
        xmlFreeDoc(state->doc);
    free(state);
    return;
}

//-----------------------------------------------------------------------------
static inline int strcmp_1u(const xmlChar *a, const char *b) {
    return strcmp(reinterpret_cast(const char *, a), b);
}

static inline char *xmlGetProp_2s(xmlNode *p, const char *v) {
    return reinterpret_cast(char *, xmlGetProp(p,
           reinterpret_cast(const xmlChar *, v)));
}

//=============================================================================
