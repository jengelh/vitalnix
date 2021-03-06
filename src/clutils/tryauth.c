/*
 *	tryauth - Helper program for authentication
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2011
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX/init.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <security/pam_appl.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxcgi/libvxcgi.h>

/* Functions */
static bool get_options(int *, const char ***);

/* Variables */
static unsigned int Verbose = 0;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	hxmc_t *user = NULL, *pass = NULL;
	int ret;

	if ((ret = HX_init()) <= 0)
		abort();
	if (!get_options(&argc, &argv))
		return 127;

	HX_getl(&user, stdin);
	HX_getl(&pass, stdin);
	HX_chomp(user);
	HX_chomp(pass);
	ret = vxcgi_authenticate(user, pass, NULL);
	HXmc_free(user);
	HXmc_free(pass);

	if (ret < 0 && Verbose)
		fprintf(stderr, "PAM error: %s\n", pam_strerror(NULL, -ret));

	HX_exit();
	return (ret > 0) ? EXIT_SUCCESS : -ret;
}

static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " tryauth\n");
	exit(EXIT_SUCCESS);
}

static bool get_options(int *argc, const char ***argv)
{
	static const struct HXoption options_table[] = {
		{.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
		 .help = "Show version information"},
		{.sh = 'v', .type = HXTYPE_NONE, .ptr = &Verbose,
		 .help = "Verbose error reporting"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) ==
	       HXOPT_ERR_SUCCESS;
}
