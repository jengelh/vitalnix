/*=============================================================================
Vitalnix User Management Suite
drivers/shadow/fsvxshadow.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2002 - 2006
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libHX.h>
#include "compiler.h"
#include "drivers/shadow/shadow.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxutil/defines.h"
#include "libvxutil/libvxutil.h"

// Functions
static inline int strcmp_1u(const xmlChar *, const char *);
static inline char *xmlGetProp_2s(xmlNode *, const char *);

//-----------------------------------------------------------------------------
void db_read_vxshadow(const char *file, struct HXdeque *dq) {
    char *name, *defer;
    struct vxpdb_user *u;
    xmlNode *ptr;
    xmlDoc *doc;

    if(dq->itemcount == 0)
        return;
    xmlKeepBlanksDefault(0);
    if((doc = xmlParseFile(file)) == NULL)
        return;
    if((ptr = xmlDocGetRootElement(doc)) == NULL ||
     strcmp_1u(ptr->name, "VX3_vxshadow") != 0)
        goto out;

    for(ptr = ptr->xmlChildrenNode; ptr != NULL; ptr = ptr->next) {
        if(ptr->type != XML_ELEMENT_NODE || strcmp_1u(ptr->name, "user") != 0)
            continue;
        if((name = xmlGetProp_2s(ptr, "lname")) == NULL)
            continue;
        if((u = lookup_user(dq, name, PDB_NOUID)) == NULL)
            continue;
        u->vs_uuid  = xmlGetProp_2s(ptr, "uuid");
        u->vs_pvgrp = xmlGetProp_2s(ptr, "pvgrp");
        if((defer = xmlGetProp_2s(ptr, "defer")) != NULL) {
            u->vs_defer = strtol(defer, NULL, 0);
            free(defer);
        }
    }

 out:
    xmlFreeDoc(doc);
    return;
}

void db_write_vxshadow(FILE *fp, const struct vxpdb_user *u) {
    char *fm = NULL;
    if(u->vs_uuid == NULL && u->vs_pvgrp == NULL && u->vs_defer == 0)
        return;

    fprintf(fp, "  <user name=\"%s\"",
            vxutil_quote(u->pw_name, VXQUOTE_XML, &fm));

    if(u->vs_uuid != NULL)
        fprintf(fp, " uuid=\"%s\"",
                vxutil_quote(u->vs_uuid, VXQUOTE_XML, &fm));

    if(u->vs_pvgrp != NULL)
        fprintf(fp, " pvgrp=\"%s\"",
                vxutil_quote(u->vs_pvgrp, VXQUOTE_XML, &fm));

    if(u->vs_defer != 0)
        fprintf(fp, " defer=\"%ld\"", u->vs_defer);

    fprintf(fp, " />\n");
    free(fm);
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
