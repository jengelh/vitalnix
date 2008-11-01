/*
 *	tcrypt - Test encryption
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2007 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <libHX/defs.h>
#include <vitalnix/libvxutil/libvxutil.h>

static const unsigned int algo_id[] = {
	VXPHASH_DES, VXPHASH_MD5, VXPHASH_BLOWFISH, VXPHASH_SMBNT,
	VXPHASH_SHA256, VXPHASH_SHA512,
};
static const char *const algo_name[] = {
	"VXPHASH_DES", "VXPHASH_MD5", "VXPHASH_BLOWFISH", "VXPHASH_SMBNT",
	"VXPHASH_SHA256", "VXPHASH_SHA512",
};

int main(int argc, const char **argv)
{
	const char *salt = NULL;
	unsigned int i;
	char *p;

	if (argc >= 3)
		salt = argv[2];

	printf("%-16s: %s\n", "Original", argv[1]);

	for (i = 0; i < ARRAY_SIZE(algo_id); ++i) {
		vxutil_phash(argv[1], salt, algo_id[i], &p);
		printf("%-16s: %s\n", algo_name[i], p);
		free(p);
	}

	return EXIT_SUCCESS;
}
