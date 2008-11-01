/*
 *	libvxeds/fm_xml.c - XML parser
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/libxml_helper.h>
#include <libxml/parser.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include "libvxeds/static-build.h"
#include <vitalnix/libvxeds/vtable.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Definitions */
struct xml_state {
	xmlDoc *doc;
	xmlNode *ptr;
};

/* Functions */
static void xml_close(void *);

//-----------------------------------------------------------------------------
static int xml_open(const char *filename, void **state_pptr)
{
	struct xml_state *state;

	if ((state = *state_pptr = calloc(1, sizeof(struct xml_state))) == NULL)
		return -errno;

	/*
	 * xmlParseFile() already scans the file completely and stores the
	 * structure tree plus components in memory.
	 */
	if ((state->doc = xmlParseFile(filename)) == NULL) {
		xml_close(*state_pptr);
		return 0;
	}

	if ((state->ptr = xmlDocGetRootElement(state->doc)) == NULL ||
	    xml_strcmp(state->ptr->name, "VX3_xmlds") != 0) {
		xml_close(*state_pptr);
		return 0;
	}

	state->ptr = state->ptr->children;
	return 1;
}

static int xml_read(void *state_ptr, struct vxeds_entry *e)
{
	/*
	 * The reading process is very simple, since libxml takes all the pain
	 * to parse the file down, and its API directly provides us with the
	 * stuff we want.
	 */
	struct xml_state *state = state_ptr;

	if (state->ptr == NULL)
		return 0;

	if (state->ptr->type == XML_ELEMENT_NODE &&
	    xml_strcmp(state->ptr->name, "user") == 0) {
		char *bd;

		memset(e, 0, sizeof(*e));
		e->first_name = xml_getprop(state->ptr, "firstname");

		if (e->first_name == NULL) {
			/* bogus entry */
			state->ptr = state->ptr->next;
			return -EINVAL;
		}

		e->surname   = xml_getprop(state->ptr, "surname");
		e->full_name = vxeds_bfullname(e->first_name, e->surname);
		e->pvgrp     = xml_getprop(state->ptr, "pvgrp");

		if ((e->uuid = xml_getprop(state->ptr, "uuid")) == NULL)
			if ((bd = xml_getprop(state->ptr, "bday")) != NULL)
				e->uuid = vxuuid_vx3(e->full_name,
				          vxutil_string_xday(bd));
	}

	state->ptr = state->ptr->next;
	return 1;
}

static void xml_close(void *state_ptr)
{
	struct xml_state *state = state_ptr;
	if (state->doc != NULL)
		xmlFreeDoc(state->doc);
	free(state);
}

//-----------------------------------------------------------------------------
static const struct edsformat_vtable THIS_FORMAT = {
	.desc   = "Vitalnix XML Input Format",
	.ext    = "xml",
	.open   = xml_open,
	.read   = xml_read,
	.close  = xml_close,
};

REGISTER_MODULE(xml, &THIS_FORMAT);
