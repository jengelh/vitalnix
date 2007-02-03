/*
    tryauth - Helper program for authentication
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
*/
#include <stdio.h>
#include <stdlib.h>
#include <libHX.h>
#include <security/pam_appl.h>
#include <vitalnix/libvxcgi/libvxcgi.h>

// Functions
static int get_options(int *, const char ***);

// Variables
static int Verbose = 0;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    hmc_t *user = NULL, *pass = NULL;
    int ret;

    if(!get_options(&argc, &argv))
        return 127;

    HX_getl(&user, stdin);
    HX_getl(&pass, stdin);
    HX_chomp(user);
    HX_chomp(pass);
    ret = vxcgi_authenticate(user, pass);
    hmc_free(user);
    hmc_free(pass);

    if(ret < 0 && Verbose)
        fprintf(stderr, "PAM error: %s\n", pam_strerror(NULL, -ret));

    return (ret > 0) ? EXIT_SUCCESS : -ret;
}

//-----------------------------------------------------------------------------
static int get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        {.sh = 'v', .type = HXTYPE_NONE, .ptr = &Verbose,
         .help = "Verbose error reporting"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };
    return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

//=============================================================================
