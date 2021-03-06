/*
 *	shadow/fsvxshadow.c - vxshadow file handling
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2002 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/deque.h>
#include <libHX/libxml_helper.h>
#include <libxml/parser.h>
#include <vitalnix/compiler.h>
#include "drivers/shadow/shadow.h"
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxutil/libvxutil.h>

//-----------------------------------------------------------------------------
void db_read_vxshadow(const char *file, struct HXdeque *dq)
{
	char *name, *defer;
	struct vxdb_user *u;
	xmlNode *ptr;
	xmlDoc *doc;

	if (dq->items == 0)
		return;
	if ((doc = xmlParseFile(file)) == NULL)
		return;
	if ((ptr = xmlDocGetRootElement(doc)) == NULL ||
	    xml_strcmp(ptr->name, "VX3_vxshadow") != 0)
		goto out;

	for (ptr = ptr->xmlChildrenNode; ptr != NULL; ptr = ptr->next) {
		if (ptr->type != XML_ELEMENT_NODE ||
		    xml_strcmp(ptr->name, "user") != 0)
			continue;
		if ((name = xml_getprop(ptr, "lname")) == NULL)
			continue;
		if ((u = lookup_user(dq, name, VXDB_NOUID)) == NULL)
			continue;
		u->vs_uuid  = xml_getprop(ptr, "uuid");
		u->vs_pvgrp = xml_getprop(ptr, "pvgrp");
		if ((defer = xml_getprop(ptr, "defer")) != NULL) {
			u->vs_defer = strtoul(defer, NULL, 0);
			free(defer);
		}
	}

 out:
	xmlFreeDoc(doc);
}

void db_write_vxshadow(FILE *fp, const struct vxdb_user *u)
{
	char *fm = NULL;
	if (u->vs_uuid == NULL && u->vs_pvgrp == NULL && u->vs_defer == 0)
		return;

	fprintf(fp, "  <user lname=\"%s\"",
			vxutil_quote(u->pw_name, VXQUOTE_XML, &fm));

	if (u->vs_uuid != NULL && *u->vs_uuid != '\0')
		fprintf(fp, " uuid=\"%s\"",
				vxutil_quote(u->vs_uuid, VXQUOTE_XML, &fm));

	if (u->vs_pvgrp != NULL && *u->vs_pvgrp != '\0')
		fprintf(fp, " pvgrp=\"%s\"",
				vxutil_quote(u->vs_pvgrp, VXQUOTE_XML, &fm));

	if (u->vs_defer != 0)
		fprintf(fp, " defer=\"%u\"", u->vs_defer);

	fprintf(fp, " />\n");
	free(fm);
}
