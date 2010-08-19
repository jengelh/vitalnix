/*
 *	tcrypt - Test encryption
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2007 - 2010
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <libHX/defs.h>
#include <libHX/string.h>
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
	const char *password, *salt = NULL;
	unsigned int i;
	char pbuf[256];
	char *hash;

	if (argc == 1) {
		*pbuf = '\0';
		printf("Password: ");
		fflush(stdout);
		fgets(pbuf, sizeof(pbuf), stdin);
		HX_chomp(pbuf);
		password = pbuf;
	} else if (argc == 2) {
		password = argv[1];
	} else if (argc >= 3) {
		salt = argv[2];
	}

	printf("%-16s: %s\n", "Original", password);

	for (i = 0; i < ARRAY_SIZE(algo_id); ++i) {
		vxutil_phash(password, salt, algo_id[i], &hash);
		printf("%-16s: %s\n", algo_name[i], hash);
		free(hash);
	}

	return EXIT_SUCCESS;
}
