/*
 *	randpw - Generate a random password
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2004 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX/option.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxutil/libvxutil.h>

/* Functions */
static bool get_options(int *, const char ***);
static void show_version(const struct HXoptcb *);

/* Variables */
static unsigned int
	Length     = 9,
	Cr_meth    = VXPHASH_BLOWFISH,
	Gen_meth   = GENPW_ZH,
	Num_pw     = 1,
	With_case  = 0,
	With_digit = 0;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	char *plain_pw, *out_cr;

	if (!get_options(&argc, &argv))
		return EXIT_FAILURE;
	if ((plain_pw = malloc(Length + 1)) == NULL) {
		perror("malloc()");
		return EXIT_FAILURE;
	}

	while (Num_pw--) {
		vxutil_genpw(plain_pw, Length + 1,
		             With_case | With_digit | Gen_meth);
		if (!vxutil_phash(plain_pw, NULL, Cr_meth, &out_cr))
			fprintf(stderr, "ERROR: vxutil_cryptpw returned false\n");
		printf("%s\n" "%s\n", plain_pw, out_cr);
		free(out_cr);
	}

	return EXIT_SUCCESS;
}

static bool get_options(int *argc, const char ***argv)
{
	static const struct HXoption options_table[] = {
		{.sh = '0', .type = HXTYPE_VAL,
		 .ptr = &With_digit, .val = GENPW_O1DIGIT,
		 .help = "Possibly have a digit in the password"},
		{.sh = '1', .type = HXTYPE_VAL,
		 .ptr = &With_digit, .val = GENPW_1DIGIT,
		 .help = "Always have a digit in the password"},
		{.sh = 'B', .type = HXTYPE_VAL,
		 .ptr = &Cr_meth, .val = VXPHASH_BLOWFISH,
		 .help = "Use Blowfish encryption"},
		{.sh = 'C', .type = HXTYPE_VAL,
		 .ptr = &With_case, .val = GENPW_1CASE,
		 .help = "Always have an uppercase character in the password"},
		{.sh = 'D', .type = HXTYPE_VAL,
		 .ptr = &Cr_meth, .val = VXPHASH_DES,
		 .help = "Use DES encryption"},
		{.sh = 'J', .type = HXTYPE_VAL,
		 .ptr = &Gen_meth, .val = GENPW_JP,
		 .help = "Use \"GENPW_JP\" generation method"},
		{.sh = 'M', .type = HXTYPE_VAL,
		 .ptr = &Cr_meth, .val = VXPHASH_MD5,
		 .help = "Use MD5 encryption"},
		{.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
		 .help = "Show version information"},
		{.sh = 'Z', .type = HXTYPE_VAL,
		 .ptr = &Gen_meth, .val = GENPW_ZH,
		 .help = "Use \"GENPW_ZH\" generation method"},
		{.sh = 'c', .type = HXTYPE_VAL,
		 .ptr = &With_case, .val = GENPW_O1CASE,
		 .help = "Possibly have an uppercase character in the password"},
		{.sh = 'l', .type = HXTYPE_UINT, .ptr = &Length,
		 .help = "Password length"},
		{.sh = 'n', .type = HXTYPE_UINT, .ptr = &Num_pw,
		 .help = "Number of passwords to generate"},
		{.sh = 'r', .type = HXTYPE_VAL, .ptr = &Gen_meth, .val = 0,
		 .help = "Use \"random\" generation method"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " randpw\n");
	exit(EXIT_SUCCESS);
}
