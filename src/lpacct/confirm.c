/*
 *	lpacct/confirm.c - confirm SQL entry
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#define _GNU_SOURCE 1 /* asprintf */
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <cups/backend.h>
#include <vitalnix/libvxutil/libvxutil.h>
#include "global.h"
#include "lpacct.h"

static void confirm_syslog(const char **argv)
{
	openlog(SYSLPREFIX, LOG_NDELAY | LOG_PID, LOG_LPR);
	syslog(LOG_INFO, "id=%s user=%s confirmed\n",
	       argv[ARGP_JOBID], argv[ARGP_USER]);
	closelog();
	return;
}

static void confirm_mysql(MYSQL *conn, const char **argv)
{
	char *quser = NULL, *sql_query;

	asprintf(&sql_query,
		"update printlog (confirmed) values (true) where "
		"jid='%lu' and user='%s'",
		strtoul(argv[ARGP_JOBID], NULL, 0),
		vxutil_quote(argv[ARGP_USER], VXQUOTE_SINGLE, &quser)
	);
	if (mysql_query(conn, sql_query) != 0)
		fprintf(stderr, "mysql_query: %s\n", mysql_error(conn));
	mysql_close(conn);
	free(quser);
	free(sql_query);
	return;
}

int main(int argc, const char **argv)
{
	struct options options = {};
	MYSQL *sql = NULL;
	pid_t pid;
	char *p;
	int ret;

	if (argc == 1) {
		printf("Vitalnix lpacct confirmation wrapper\n");
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

	if (lpacct_readconfig(&options) <= 0)
		return CUPS_BACKEND_FAILED;

	if (options.acct_mysql)
		if ((sql = lpacct_sql_start(&options)) == NULL)
			return CUPS_BACKEND_FAILED;

	if ((pid = fork()) < 0) {
		perror("fork");
		return CUPS_BACKEND_FAILED;
	} else if (pid == 0) {
		execv(*argv, (char *const *)argv);
		perror("execv");
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
