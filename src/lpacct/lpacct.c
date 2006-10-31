/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.
*/
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libHX.h>
#include "global.h"
#include "gsfilter.h"
#include "image.h"
#include "util.h"

// Definitions
#define DEFAULT_GS_DPI  96

// Functions
static int lpacct_analyze_main(int argc, const char **);
static int lpacct_filter_main(int argc, const char **);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
    const char *base = HX_basename(*argv);

    if(strcmp(base, "lpacct_analyze") == 0)
        return lpacct_analyze_main(argc, argv);
    else
        return lpacct_filter_main(argc, argv);
}

//-----------------------------------------------------------------------------
/*  lpacct_analyze_main
    @argc:      argument count
    @argv:      argument vector

    Used for console operation.
*/
static int lpacct_analyze_main(int argc, const char **argv)
{
    struct options proc_opt = {
        .dpi        = 96,
        .colorspace = COLORSPACE_CMYK,
        .verbose    = 1,
    };
    struct options *parg = &proc_opt;
    struct HXoption options_table[] = {
        {.ln = "a4",   .type = HXTYPE_NONE, .ptr = &proc_opt.unit_a4,      .help = "Display results measured in FI A4"},
        {.ln = "drop", .type = HXTYPE_NONE, .ptr = &proc_opt.unit_droplet, .help = "Display results in droplets (1/255 FI pixel)"},
        {.ln = "sqcm", .type = HXTYPE_NONE, .ptr = &proc_opt.unit_i_sqcm,  .help = "Display results in i*cm²"},
        {.ln = "sqm",  .type = HXTYPE_NONE, .ptr = &proc_opt.unit_i_sqm,   .help = "Display results in i*m²"},
        {.ln = "sqin", .type = HXTYPE_NONE, .ptr = &proc_opt.unit_i_sqin,  .help = "Display results in i*in²"},

        {.ln = "cmyk", .type = HXTYPE_VAL, .val = COLORSPACE_CMYK, .help = "CMYK colorspace",      .ptr = &proc_opt.colorspace},
        {.ln = "cmy",  .type = HXTYPE_VAL, .val = COLORSPACE_CMY,  .help = "CMY colorspace",       .ptr = &proc_opt.colorspace},
        {.ln = "gray", .type = HXTYPE_VAL, .val = COLORSPACE_GRAY, .help = "Grayscale colorspace", .ptr = &proc_opt.colorspace},

	{.sh = 'd', .ln = "dpi",  .type = HXTYPE_UINT,   .ptr = &proc_opt.dpi,	    .help = "Dots per inch"},
	{.sh = 'f', .ln = "file", .type = HXTYPE_STRING, .ptr = &proc_opt.filename, .help = "File to analyze"},
	HXOPT_AUTOHELP,
	HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, &argc, &argv, HXOPT_USAGEONERR) <= 0)
        return EXIT_FAILURE;

    if(parg->filename == NULL) {
        fprintf(stderr, PREFIX "Specify a filename with the -f option,"
                " and preferably the DPI with -D\n");
        return EXIT_FAILURE;
    }

    // Set some default display options if none was given
    if(!(parg->unit_droplet | parg->unit_i_sqcm | parg->unit_i_sqm |
      parg->unit_i_sqin | parg->unit_a4))
            parg->unit_a4 = parg->unit_i_sqcm = 1;

    parg->unit_metric = parg->unit_i_sqcm | parg->unit_i_sqm | parg->unit_a4;

    return proc_image(parg) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*  lpacct_filter_main
    @argc:      argument count
    @argv:      argument vector

    Called when used as a CUPS filter.
*/
static int lpacct_filter_main(int argc, const char **argv)
{
    char gs_input_file[256]  = "/tmp/lpacctXXXXXX",
         gs_output_file[256] = "/tmp/lpacctXXXXXX";
    int data_input_fd, gs_input_fd, gs_output_fd;

/*    FILE *f = fopen("/dev/tty2", "w");
    fprintf(f, "[%d]\n", getpid());
    fclose(f);
    sleep(10);*/

    if(argc == 7) {
        if((data_input_fd = open(argv[6], O_RDONLY)) < 0) {
            fprintf(stderr, PREFIX "Could not open %s: %s\n",
                    argv[6], strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else if(argc == 6) {
        data_input_fd = STDIN_FILENO;
    } else {
        fprintf(stderr, "Usage: %s JOB_ID USER TITLE COPIES OPTIONS [FILE]\n", *argv);
        return EXIT_FAILURE;
    }

    umask(~(S_IRUSR | S_IWUSR));
    if((gs_input_fd = mkstemp(gs_input_file)) < 0)
        fprintf(stderr, PREFIX "mkstemp() failed with %d\n", errno);
    if((gs_output_fd = mkstemp(gs_output_file)) < 0)
        fprintf(stderr, PREFIX "mkstemp() failed with %d\n", errno);

    fd_tee(data_input_fd, STDOUT_FILENO, gs_input_fd);
    /* Closing STDOUT here allows the next filter to asynchronously start
    processing the data. @gs_output_fd is also closed because we do not need
    the fd, just the filename. */
    close(STDOUT_FILENO);
    close(gs_output_fd);

    if(gs_input_fd > 0 && gs_output_fd > 0) {
        // Accounting starts here.
        struct options op = {
            .do_account = 1,
            .dpi        = DEFAULT_GS_DPI,
            .colorspace = COLORSPACE_CMYK,
            .filename   = gs_output_file,
            .cups_args  = argv,
        };
        if(proc_gs(gs_input_file, gs_output_file))
            proc_image(&op);
    }

    if(gs_input_fd > 0)
        unlink(gs_input_file);
    if(gs_output_fd > 0)
        unlink(gs_output_file);

    return EXIT_SUCCESS;
}

//=============================================================================
