/*
 *	libvxutil/crypt.c - Password encryption
 *	Copyright © CC Computer Consultants GmbH, 2003 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdio.h>
#include <vitalnix/libvxutil/libvxutil.h>

int main(int argc, const char **argv)
{
	char *out;

	while (*++argv != NULL) {
		vxutil_cryptpw(*argv, NULL, CRYPW_SMBNT, &out);
		printf("%s\n", out);
		free(out);
	}

	return EXIT_SUCCESS;
}
