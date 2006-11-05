/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.
*/
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libHX.h>
#include "global.h"
#include "image.h"
#include "lpacct.h"

// Definitions
#define DEFAULT_GS_DPI  96

// Functions
static int lpacct_analyze_main(int argc, const char **);
static int lpacct_filter_main(int argc, const char **);
static int ghostscript_init(const char *, pid_t *, int);
static void ghostscript_exit(pid_t);
static int generic_tee(int, int, int);
static int generic_tee_named(const char *, int, int);

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
        .dpi        = DEFAULT_GS_DPI,
        .colorspace = COLORSPACE_CMYK,
    };
    struct options *p = &proc_opt;
    char *input_file = NULL;
    pid_t pid;
    int ret;
    struct HXoption options_table[] = {
        {.ln = "cmyk", .type = HXTYPE_VAL, .val = COLORSPACE_CMYK, .ptr = &p->colorspace, .help = "Calculate for CMYK colorspace"},
        {.ln = "cmy",  .type = HXTYPE_VAL, .val = COLORSPACE_CMY,  .ptr = &p->colorspace, .help = "Calculate for CMY colorspace"},
        {.ln = "gray", .type = HXTYPE_VAL, .val = COLORSPACE_GRAY, .ptr = &p->colorspace, .help = "Calculate for grayscale colorspace"},

	{.sh = 'd', .ln = "dpi",  .type = HXTYPE_UINT,   .ptr = &proc_opt.dpi, .help = "Dots per inch"},
	{.sh = 'f', .ln = "file", .type = HXTYPE_STRING, .ptr = &input_file,   .help = "File to analyze"},
        {.sh = 'p', .type = HXTYPE_NONE, .ptr = &p->per_page_stats, .help = "Display per-page statistics"},
        {.sh = 's', .type = HXTYPE_NONE, .ptr = &p->per_doc_stats,  .help = "Display statistics"},
	HXOPT_AUTOHELP,
	HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, &argc, &argv, HXOPT_USAGEONERR) <= 0)
        return EXIT_FAILURE;

    if(input_file == NULL) {
        fprintf(stderr, "Specify a filename with the -f option,"
                " and preferably the DPI with -d\n");
        return EXIT_FAILURE;
    }

    ret = ghostscript_init(input_file, &pid, p->dpi);
    if(ret < 0) {
        fprintf(stderr, PREFIX "GhostScript init failed: %s\n", strerror(ret));
        return EXIT_FAILURE;
    }
    ret = (mpxm_process(ret, p) > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    ghostscript_exit(pid);
    return ret;
}

/*  lpacct_filter_main
    @argc:      argument count
    @argv:      argument vector

    Called when used as a CUPS filter.
*/
static int lpacct_filter_main(int argc, const char **argv)
{
    const char *input_file = NULL;
    char input_tmp[] = "/tmp/lpacctXXXXXX";
    pid_t pid;
    int fd, ret;

    if(argc == 7) {
        input_file = argv[6];
        if((ret = generic_tee_named(input_file, STDOUT_FILENO, -1)) < 0) {
            fprintf(stderr, PREFIX "generic_named_tee: %s\n", strerror(ret));
            return EXIT_FAILURE;
        }
    } else if(argc == 6) {
        if((fd = mkstemp(input_tmp)) < 0) {
            fprintf(stderr, PREFIX "mkstemp: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        if((ret = generic_tee(STDIN_FILENO, STDOUT_FILENO, fd)) < 0) {
            fprintf(stderr, PREFIX "generic_tee: %s\n", strerror(ret));
            return EXIT_FAILURE;
        }
        input_file = input_tmp;
        close(STDOUT_FILENO);
        close(fd);
    } else {
        fprintf(stderr, "Usage: %s JOB_ID USER TITLE COPIES OPTIONS [FILE]\n", *argv);
        return EXIT_FAILURE;
    }

    fd = ret = ghostscript_init(input_file, &pid, DEFAULT_GS_DPI);
    if(ret < 0) {
        fprintf(stderr, PREFIX "GhostScript init failed: %s\n", strerror(ret));
        ret = EXIT_FAILURE;
    } else {
        // Accounting starts here.
        struct options op = {
            .do_account = 1,
            .dpi        = DEFAULT_GS_DPI,
            .colorspace = COLORSPACE_CMYK,
            .cups_args  = argv,
        };
        ret = (mpxm_process(fd, &op) > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    ghostscript_exit(pid);
    if(input_file == input_tmp)
        unlink(input_tmp);

    return EXIT_SUCCESS;
}

/*  ghostscript_init
    @input_file:        gs param -- postscript file to parse
    @pid:               store point for PID
    @dpi:               gs param -- dots per inch

    Starts the GhostScript interpreter on @input_file with @dpi x @dpi
    resolution. Puts the PID of the subprocess into @pid and returns the
    file descriptor for it. Returns -errno on error.
*/
static int ghostscript_init(const char *input_file, pid_t *pid, int dpi)
{
    char dpi_string[sizeof("-r3600")];
    const char *argv[] = {
        "gs", "-dBATCH", "-dNOPAUSE", "-dQUIET", "-dSAFER", dpi_string,
        "-sDEVICE=ppmraw", "-sOutputFile=-", input_file, NULL,
    };
    int fd, ret, output_pipe[2];

    if(pipe(output_pipe) != 0) {
        ret = errno;
        fprintf(stderr, PREFIX "%s: pipe() failed: %s\n",
                __func__, strerror(ret));
        goto err;
    }

    if((fd = open(input_file, O_RDONLY)) < 0) {
        ret = errno;
        fprintf(stderr, PREFIX "%s: Could not open %s: %s\n",
                __func__, input_file, strerror(ret));
        goto err;
    }

    snprintf(dpi_string, sizeof(dpi_string), "-r%d", (dpi < 2) ? 2 : dpi);

    if((*pid = fork()) < 0) {
        ret = errno;
        fprintf(stderr, PREFIX "%s: fork() failed: %s\n",
                __func__, strerror(ret));
        goto err;
    } else if(*pid == 0) {
        dup2(fd, STDIN_FILENO);
        dup2(output_pipe[1], STDOUT_FILENO);
        close(fd);
        close(output_pipe[0]);
        close(output_pipe[1]);
        // keep STDERR_FILENO -- user or cups pick it up
        ret = execvp(*argv, (char * const *)argv);
        fprintf(stderr, "%s: execvp() failed: %s\n", __func__, strerror(ret));
        exit(127);
    }

    if(waitpid(*pid, &ret, WNOHANG) > 0) {
        fprintf(stderr, PREFIX "%s: Ghostscript terminated%s, exit status %d\n",
                __func__, !WIFEXITED(ret) ? " abnormally" : "",
                WEXITSTATUS(ret));
        goto err;
    }

    close(fd);
    close(output_pipe[1]);
    return output_pipe[0];

 err:
    close(fd);
    close(output_pipe[0]);
    close(output_pipe[1]);
    return -ret;
}

static void ghostscript_exit(pid_t pid)
{
    int status;
    waitpid(pid, &status, 0);
    if(WIFEXITED(status))
        return;
    if(WIFSIGNALED(status))
        fprintf(stderr, "%s: GhostScript terminated abnormally, signal %d\n",
                __func__, WTERMSIG(status));
    else
        fprintf(stderr, "%s: GhostScript terminated abnormally\n", __func__);

    return;
}

//-----------------------------------------------------------------------------
static int generic_tee(int fi, int fa, int fb)
{
    int ret = 0, ret2 = 0, ret3 = 0;
    char buf[8192];

    if(fa == -1)
        while((ret = read(fi, buf, sizeof(buf))) > 0 &&
              (ret2 = write(fb, buf, ret)) > 0);
    else if(fb == -1)
        while((ret = read(fi, buf, sizeof(buf))) > 0 &&
              (ret2 = write(fa, buf, ret)) > 0);
    else
        while((ret = read(fi, buf, sizeof(buf))) > 0) {
            ret2 = write(fa, buf, ret);
            ret3 = write(fb, buf, ret);
            if(ret2 <= 0 && ret3 <= 0)
                break;
        }

    if(ret < 0)
        return ret;
    if(ret2 < 0)
        return ret2;
    if(ret3 < 0)
        return ret3;

    return 1;
}

static int generic_tee_named(const char *file, int fa, int fb)
{
    int fi, ret;
    if((fi = open(file, O_RDONLY)) < 0)
        return -errno;
    ret = generic_tee(fi, fa, fb);
    close(fi);
    return 0;
}

//=============================================================================
