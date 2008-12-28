/*
 *	libvxcgi/cgi.c - CGI parameter handling
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2005 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/arbtree.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxcgi/libvxcgi.h>

EXPORT_SYMBOL char *vxcgi_read_data(int argc, const char **argv)
{
	const char *meth = getenv("REQUEST_METHOD");

	if (meth == NULL) {
		if (argc >= 2)
			return HX_strdup(argv[1]);
		return NULL;
	}

	if (strcmp(meth, "GET") == 0) {
		return HX_strdup(getenv("QUERY_STRING"));
	} else if (strcmp(meth, "POST") == 0) {
		char *len_s = getenv("CONTENT_LENGTH");
		size_t len  = strtoul(len_s, NULL, 0);
		char *new;

		if ((new = malloc(len + 1)) != NULL) {
			new[len]    = '\0';
			fread(new, len, 1, stdin);
		}
		return new;
	}

	return NULL;
}

EXPORT_SYMBOL struct HXbtree *vxcgi_split(char *str)
{
	struct HXbtree *h = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_CDATA);
	char *bufp = str, *ptr;

	if (h == NULL || str == NULL)
		return h;

	while ((ptr = HX_strsep(&bufp, "&")) != NULL) {
		const char *key = ptr;
		char *val = strchr(ptr, '=');

		if (val != NULL) {
			*val++ = '\0';
			if (*val == '\0')
				val = NULL;
		}
		HXbtree_add(h, key, val);
	}

	free(str);
	return h;
}
