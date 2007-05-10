/*
    shadow/fsvxshadow.c - vxshadow file handling
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2002 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include "drivers/shadow/shadow.h"
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

// Functions
static inline int strcmp_1u(const xmlChar *, const char *);
static inline char *xmlGetProp_2s(xmlNode *, const char *);

//-----------------------------------------------------------------------------
void db_read_vxshadow(const char *file, struct HXdeque *dq) {
    char *name, *defer;
    struct vxpdb_user *u;
    xmlNode *ptr;
    xmlDoc *doc;

    if(dq->items == 0)
        return;
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

    fprintf(fp, "  <user lname=\"%s\"",
            vxutil_quote(u->pw_name, VXQUOTE_XML, &fm));

    if(u->vs_uuid != NULL && *u->vs_uuid != '\0')
        fprintf(fp, " uuid=\"%s\"",
                vxutil_quote(u->vs_uuid, VXQUOTE_XML, &fm));

    if(u->vs_pvgrp != NULL && *u->vs_pvgrp != '\0')
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
