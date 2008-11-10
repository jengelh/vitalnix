/*
 *	lpacct/lpacct.c - Print accounting
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libHX/defs.h>
#include <libHX/option.h>
#include <libHX/string.h>
#include <vitalnix/libvxutil/defines.h>
#include "acct.h"
#include "global.h"
#include "image.h"
#include "lpacct.h"
#define strnacmp(dai, sai) strncmp((dai), (sai), sizeof(sai)-1)

/* Functions */
static int lpacct_analyze_main(int, const char **);
static int lpacct_filter_main(int, const char **);
static int ghostscript_init(const char *, pid_t *, const struct options *);
static void ghostscript_exit(pid_t);

static int fnopen(int, const char *);
static int generic_tee(int, int, int);
static int generic_tee_named(const char *, int, int);
static enum colorspace get_colorspace(void);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	const char *base = HX_basename(*argv);

	if (strcmp(base, "lpacct_analyze") == 0)
		return lpacct_analyze_main(argc, argv);
	else
		return lpacct_filter_main(argc, argv);
}

void pr_exit(const char *func, const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	if (func == NULL)
		fprintf(stderr, PREFIX);
	else
		fprintf(stderr, PREFIX "%s: ", func);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	exit(EXIT_FAILURE);
}

void pr_warn(const char *func, const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	fprintf(stderr, PREFIX "%s: ", func);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
}

//-----------------------------------------------------------------------------
/**
 * lpacct_analyze_main -
 * @argc:	argument count
 * @argv:	argument vector
 *
 * Used for console operation.
 */
static int lpacct_analyze_main(int argc, const char **argv)
{
	struct options proc_opt, *p = &proc_opt;
	char *input_file = NULL;
	pid_t pid;
	int fd, ret;
	struct HXoption options_table[] = {
		{.ln = "cmyk",  .type = HXTYPE_VAL, .val = COLORSPACE_CMYK,  .ptr = &p->colorspace, .help = "Calculate for CMYK colorspace"},
		{.ln = "cmypk", .type = HXTYPE_VAL, .val = COLORSPACE_CMYPK, .ptr = &p->colorspace, .help = "Calculate for CMY+K colorspace"},
		{.ln = "cmy",   .type = HXTYPE_VAL, .val = COLORSPACE_CMY,   .ptr = &p->colorspace, .help = "Calculate for CMY colorspace"},
		{.ln = "gray",  .type = HXTYPE_VAL, .val = COLORSPACE_GRAY,  .ptr = &p->colorspace, .help = "Calculate for grayscale colorspace"},

		{.sh = 'B', .type = HXTYPE_VAL, .val = 0, .ptr = &p->rasterize, .help = "Do not run rasterizer (debug)"},
		{.sh = 'd', .ln = "dpi",  .type = HXTYPE_UINT,   .ptr = &p->dpi,     .help = "Dots per inch"},
		{.sh = 'f', .ln = "file", .type = HXTYPE_STRING, .ptr = &input_file, .help = "File to analyze"},
		{.sh = 'p', .type = HXTYPE_NONE, .ptr = &p->per_page_stats, .help = "Display per-page statistics"},
		{.sh = 's', .type = HXTYPE_NONE, .ptr = &p->per_doc_stats,  .help = "Display statistics"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	lpacct_readconfig(p);
	if (HX_getopt(options_table, &argc, &argv, HXOPT_USAGEONERR) <= 0)
		return EXIT_FAILURE;

	if (input_file == NULL)
		pr_exit(NULL, "Specify a filename with the -f option,"
		        " and preferably the DPI with -d\n");

	if (p->rasterize)
		fd = ghostscript_init(input_file, &pid, p);
	else if ((fd = open(input_file, O_RDONLY)) < 0)
		pr_exit(NULL, "Could not open %s: %s\n",
		        input_file, strerror(errno));

	ret = (mpxm_process(fd, p) > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
	if (p->rasterize)
		ghostscript_exit(pid);
	return ret;
}

/**
 * lpacct_filter_main -
 * @argc:	argument count
 * @argv:	argument vector
 *
 * Called when used as a CUPS filter.
 */
static int lpacct_filter_main(int argc, const char **argv)
{
	const char *input_file = NULL;
	char input_tmp[] = "/tmp/lpacctXXXXXX";
	struct options op;
	pid_t pid;
	int fd, ret;

	lpacct_readconfig(&op);
	op.colorspace = get_colorspace();
	op.cups_args  = argv;

	if (argc == 7) {
		input_file = argv[6];
		ret = generic_tee_named(input_file, STDOUT_FILENO, -1);
		if (ret < 0)
			pr_exit(NULL, "generic_named_tee: %s\n", strerror(ret));
	} else if (argc == 6) {
		if ((fd = mkstemp(input_tmp)) < 0)
			pr_exit(NULL, "mkstemp: %s\n", strerror(errno));
		if ((ret = generic_tee(STDIN_FILENO, STDOUT_FILENO, fd)) < 0)
			pr_exit(NULL, "generic_tee: %s\n", strerror(ret));
		input_file = input_tmp;
		close(fd);
	} else {
		fprintf(stderr, "Usage: %s JOB_ID USER TITLE COPIES "
		        "OPTIONS [FILE]\n", *argv);
		return EXIT_FAILURE;
	}

	/*
	 * There is a case where pipe() returns {%STDOUT_FILENO, @any} - but
	 * dup2()ing @any onto %STDOUT_FILENO does not do anything, even though the
	 * two filedescriptors should be considered different.
	 * Just keep %STDOUT_FILENO open.
	 */
	fnopen(STDOUT_FILENO, "/dev/null");
	fd  = ghostscript_init(input_file, &pid, &op);
	ret = (mpxm_process(fd, &op) > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
	ghostscript_exit(pid);
	if (input_file == input_tmp)
		unlink(input_tmp);

	return EXIT_SUCCESS;
}

/**
 * ghostscript_init -
 * @input_file:	gs param -- postscript file to parse
 * @pid:	store point for PID
 * @dpi:	gs param -- dots per inch
 *
 * Starts the GhostScript interpreter on @input_file with @dpi x @dpi
 * resolution. Puts the PID of the subprocess into @pid and returns the
 * file descriptor for it. Aborts on error.
 */
static int ghostscript_init(const char *input_file, pid_t *pid,
    const struct options *op)
{
	char device_string[sizeof("-sDEVICE=ppmraw")] = "-sDEVICE=ppmraw";
	char dpi_string[sizeof("-r3600")];
	const char *argv[] = {
		"gs", "-dBATCH", "-dNOPAUSE", "-dQUIET", "-dSAFER", dpi_string,
		device_string, "-sOutputFile=-", "-sstdout=/dev/null",
		input_file, NULL,
	};
	int fd, ret, output_pipe[2];

	if (pipe(output_pipe) != 0)
		pr_exit(__func__, "pipe() failed: %s\n", strerror(errno));
	if ((fd = open(input_file, O_RDONLY)) < 0)
		pr_exit(__func__, "Could not open %s: %s\n",
		        input_file, strerror(errno));

	snprintf(dpi_string, sizeof(dpi_string), "-r%d",
	         (op->dpi < 2) ? 2 : op->dpi);
	if (op->colorspace == COLORSPACE_GRAY)
		snprintf(device_string, sizeof(device_string),
		         "-sDEVICE=pgmraw");

	if ((*pid = fork()) < 0) {
		pr_exit(__func__, "fork() failed: %s\n", strerror(errno));
	} else if (*pid == 0) {
		dup2(fd, STDIN_FILENO);
		dup2(output_pipe[1], STDOUT_FILENO);
		close(fd);
		close(output_pipe[0]);
		close(output_pipe[1]);
		/* keep %STDERR_FILENO -- user or cups pick it up */
		execvp(*argv, reinterpret_cast(char * const *, argv));
		pr_exit(__func__, "execvp() failed: %s\n", strerror(errno));
		exit(127);
	}

	if (waitpid(*pid, &ret, WNOHANG) > 0)
		pr_exit(__func__, "Ghostscript terminated%s, exit status %d\n",
		        !WIFEXITED(ret) ? " abnormally" : "", WEXITSTATUS(ret));

	close(fd);
	close(output_pipe[1]);
	return output_pipe[0];
}

static void ghostscript_exit(pid_t pid)
{
	int status;
	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		return;
	if (WIFSIGNALED(status))
		pr_warn(__func__, "GhostScript terminated abnormally, "
		        "signal %d\n", WTERMSIG(status));
	else
		pr_warn(__func__, "GhostScript terminated abnormally\n");
}

//-----------------------------------------------------------------------------
static int fnopen(int to_fd, const char *file)
{
	int cur_fd;
	if ((cur_fd = open(file, O_RDWR)) < 0)
		return -errno;
	if (cur_fd != to_fd) {
		dup2(cur_fd, to_fd);
		close(cur_fd);
	}
	return 1;
}

static int generic_tee(int fi, int fa, int fb)
{
	int ret = 0, ret2 = 0, ret3 = 0;
	char buf[8192];

	if (fa == -1)
		while ((ret = read(fi, buf, sizeof(buf))) > 0 &&
		    (ret2 = write(fb, buf, ret)) > 0)
			;
	else if (fb == -1)
		while ((ret = read(fi, buf, sizeof(buf))) > 0 &&
		    (ret2 = write(fa, buf, ret)) > 0)
			;
	else
		while ((ret = read(fi, buf, sizeof(buf))) > 0) {
			ret2 = write(fa, buf, ret);
			ret3 = write(fb, buf, ret);
			if (ret2 <= 0 && ret3 <= 0)
				break;
		}

	if (ret < 0)
		return ret;
	if (ret2 < 0)
		return ret2;
	if (ret3 < 0)
		return ret3;

	return 1;
}

static int generic_tee_named(const char *file, int fa, int fb)
{
	int fi, ret;
	if ((fi = open(file, O_RDONLY)) < 0)
		return -errno;
	ret = generic_tee(fi, fa, fb);
	close(fi);
	return 0;
}

static enum colorspace get_colorspace(void)
{
	enum colorspace ret = COLORSPACE_GRAY;
	const char *printer, *p;
	char buf[MAXFNLEN];
	hxmc_t *ln = NULL;
	FILE *fp;

	if ((printer = getenv("PRINTER")) == NULL) {
		pr_warn(__func__, "PRINTER not defined\n");
		return ret;
	}

	snprintf(buf, sizeof(buf), "/etc/cups/ppd/%s.ppd", printer);
	if ((fp = fopen(buf, "r")) == NULL) {
		pr_warn(__func__, "Could not open %s: %s\n",
		        printer, strerror(errno));
		return ret;
	}

	while (HX_getl(&ln, fp) != NULL) {
		if (strnacmp(ln, "*DefaultColorMode:") == 0) {
			HX_chomp(ln);
			p = strchr(ln, ':') + 1;
			while (isspace(*p))
				++p;
			if (strcasecmp(p, "CMYK") == 0 ||
			    strcasecmp(p, "Color") == 0 ||
			    strcasecmp(p, "Colour") == 0 ||
			    strcasecmp(p, "RGB") == 0 ||
			    strcasecmp(p, "Contone") == 0)
				ret = COLORSPACE_CMYK;
			else if (strcasecmp(p, "CMY+K") == 0)
				ret = COLORSPACE_CMYPK;
			else if (strcasecmp(p, "CMY") == 0)
				ret = COLORSPACE_CMY;
			else /* "Gray" "Grayscale" */
				ret = COLORSPACE_GRAY;
			/* What is "NormalColor"? gray or color? */
		}
	}

	HXmc_free(ln);
	return ret;
}
