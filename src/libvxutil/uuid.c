/*
 *	libvxutil/uuid.c - UUID-related functions
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2005 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <libHX/defs.h>
#include <libHX/misc.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxutil/libvxutil.h>

#define CURRENT_TAG             "{VX3A}"
#define CURRENT_TAG_SIZE        (sizeof(CURRENT_TAG) - 1)

EXPORT_SYMBOL char *vxuuid_vx3(const char *full_name, unsigned int xday)
{
	unsigned char md[MD5_DIGEST_LENGTH];
	char tmp[48];

	MD5(signed_cast(const unsigned char *, full_name), strlen(full_name), md);
#define B "%02x"
#define D B B B B
	snprintf(tmp, sizeof(tmp), CURRENT_TAG "%06x" D D D D,
	         xday, md[0], md[1], md[2], md[3], md[4], md[5], md[6], md[7],
	         md[8], md[9], md[10], md[11], md[12], md[13], md[14], md[15]);
#undef B
#undef D
	return HX_strdup(tmp);
}

EXPORT_SYMBOL unsigned int vxuuid_vx3_get_xday(const char *s)
{
	char tmp[5];
	if (strncmp(s, CURRENT_TAG, CURRENT_TAG_SIZE) != 0)
		return -1;
	HX_strlcpy(tmp, s + CURRENT_TAG_SIZE, sizeof(tmp));
	return strtoul(tmp, NULL, 16);
}
