/*
 *	lpacct/confirm.c - confirm SQL entry
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#define _GNU_SOURCE 1 /* asprintf */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <libHX/ctype_helper.h>
#include <libHX/defs.h>
#include <libHX/misc.h>
#include <libHX/string.h>
#include <cups/backend.h>
#include <vitalnix/libvxutil/libvxutil.h>
#include "global.h"
#include "lpacct.h"
#define PROTOCOL_PREFIX "scv:" /* sql confirm for vitalnix */

static hxmc_t *backend_dir;

static bool next_word(const char **w, const char **s, const char **e)
{
	const char *i = *w;

	if (*i == '\0')
		return false;
	*s = i;
	while (!HX_isspace(*i))
		++i;
	*e = i;
	while (HX_isspace(*i))
		++i;
	*w = i;
	return true;
}

static bool next_wordq(const char **w, const char **s, const char **e)
{
	const char *i = *w;

	if (*i != '"')
		return false;
	*s = ++i;
	while (*i != '\0') {
		if (*i == '\\') {
			if (*++i == '\0')
				return false;
		} else if (*i == '"') {
			break;
		}
		++i;
	}
	*e = i++;
	while (HX_isspace(*i))
		++i;
	*w = i;
	return true;
}

static void run_other_backend(const hxmc_t *prog)
{
	const char *class_start, *class_end;
	const char *uri_start, *uri_end;
	const char *model_start, *model_end;
	const char *desc_start, *desc_end;
	const char *devid_start, *devid_end, *w;
	hxmc_t *ln = NULL;
	bool do_devid;
	FILE *fp;

	fp = popen(prog, "r");
	if (fp == NULL)
		return;

	while (HX_getl(&ln, fp) != NULL) {
		w = ln;
		if (!next_word(&w, &class_start, &class_end))
			continue;
		if (!next_word(&w, &uri_start, &uri_end))
			continue;
		if (!next_wordq(&w, &model_start, &model_end))
			continue;
		if (!next_wordq(&w, &desc_start, &desc_end))
			continue;
		do_devid = next_wordq(&w, &devid_start, &devid_end);

		printf("%.*s " PROTOCOL_PREFIX "%.*s \"%.*s\" "
		       "\"Vitalnix SCV: %.*s\"",
		       static_cast(unsigned int, class_end - class_start),
		       class_start,
		       static_cast(unsigned int, uri_end - uri_start),
		       uri_start,
		       static_cast(unsigned int, model_end - model_start),
		       model_start,
		       static_cast(unsigned int, desc_end - desc_start),
		       desc_start);
		if (do_devid)
			printf(" \"%.*s\"",
			       static_cast(unsigned int, devid_end - devid_start),
			       devid_start);
		printf("\n");
	}

	pclose(fp);
	HXmc_free(ln);
}

static void find_other_backends(void)
{
	hxmc_t *backend_prog;
	const char *de;
	struct stat sb;
	void *dirptr;

	dirptr = HXdir_open(backend_dir);
	if (dirptr == NULL)
		return;

	while ((de = HXdir_read(dirptr)) != NULL) {
		if (*de == '.')
			continue;
		backend_prog = HXmc_strinit(backend_dir);
		HXmc_strcat(&backend_prog, de);
		if (stat(backend_prog, &sb) < 0) /* dereference symlinks */
			continue;
		if (!S_ISREG(sb.st_mode))
			continue;
		run_other_backend(backend_prog);
		HXmc_free(backend_prog);
	}

	HXdir_close(dirptr);
}

static void confirm_syslog(const char **argv)
{
	const char *queue = getenv("PRINTER");

	openlog(SYSLPREFIX, LOG_NDELAY | LOG_PID, LOG_LPR);
	syslog(LOG_INFO, "id=%s/%s/%s confirmed\n",
	       queue, argv[ARGP_JOBID], argv[ARGP_USER]);
	closelog();
}

static void confirm_mysql(MYSQL *conn, const char **argv)
{
	char *quser = NULL, *sql_query;

	asprintf(&sql_query,
		"update printlog set confirmed=true where "
		"jid=%lu and user='%s'",
		strtoul(argv[ARGP_JOBID], NULL, 0),
		vxutil_quote(argv[ARGP_USER], VXQUOTE_SINGLE, &quser)
	);
	if (mysql_query(conn, sql_query) != 0)
		fprintf(stderr, "mysql_query: %s\n", mysql_error(conn));
	mysql_close(conn);
	free(quser);
	free(sql_query);
}

int main(int argc, const char **argv)
{
	struct options options = {};
	MYSQL *sql = NULL;
	pid_t pid;
	char *p, *q;
	int ret;

	if ((p = getenv("CUPS_SERVERBIN")) == NULL)
		return CUPS_BACKEND_FAILED;
	backend_dir = HXmc_strinit(p);
	HXmc_strcat(&backend_dir, "/backend/");

	if (argc == 1) {
		/* Detect recursion */
		if (getenv("LPACCT") != NULL)
			return CUPS_BACKEND_OK;
		setenv("LPACCT", "1", true);
		find_other_backends();
		return CUPS_BACKEND_OK;
	} else if (argc != 6 && argc != 7) {
		fprintf(stderr, "Usage: %s JOB_ID USER TITLE COPIES OPTIONS "
		        "[FILE]\n", *argv);
		return CUPS_BACKEND_FAILED;
	}

	if ((p = strchr(*argv, ':')) == NULL) {
		fprintf(stderr, "Invalid URI\n");
		return CUPS_BACKEND_FAILED;
	}

	*argv = p + 1;

	q = getenv("DEVICE_URI");
	if (q == NULL) {
		fprintf(stderr, "Invalid DEVICE_URI (= NULL)\n");
		return CUPS_BACKEND_FAILED;
	}

	if ((p = strchr(q, ':')) == NULL) {
		fprintf(stderr, "Invalid DEVICE_URI: %s\n", q);
		return CUPS_BACKEND_FAILED;
	}

	setenv("DEVICE_URI", p + 1, true);

	if (lpacct_readconfig(&options) <= 0)
		return CUPS_BACKEND_FAILED;

	if (options.acct_mysql)
		if ((sql = lpacct_sql_start(&options)) == NULL)
			return CUPS_BACKEND_FAILED;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return CUPS_BACKEND_FAILED;
	} else if (pid == 0) {
		hxmc_t *backend_name = HXmc_strinit(backend_dir);
		const char *e = strchr(*argv, ':');

		HXmc_memcat(&backend_name, *argv, e - *argv);
		execv(backend_name, const_cast2(char *const *, argv));
		perror("execv");
		HXmc_free(backend_name);
		exit(CUPS_BACKEND_FAILED);
	}

	wait(&ret);
	if (!WIFEXITED(ret))
		return CUPS_BACKEND_FAILED;
	if (WEXITSTATUS(ret) != 0)
		return WEXITSTATUS(ret);
	if (options.acct_syslog)
		confirm_syslog(argv);
	if (options.acct_mysql)
		confirm_mysql(sql, argv);
	return CUPS_BACKEND_OK;
}
