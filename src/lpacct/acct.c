/*
 *	lpacct/acct.c - Accounting
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#define _GNU_SOURCE 1 /* for asprintf */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <mysql.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxutil/libvxutil.h>
#include "acct.h"
#include "drop.h"
#include "global.h"
#include "lpacct.h"

void acct_syslog(const struct options *op, const struct costf *cost)
{
	openlog(SYSLPREFIX, LOG_NDELAY | LOG_PID, LOG_LPR);
	syslog(LOG_INFO, "id=%s/%s/%s +cmyktp(%f, %f, %f, %f, %f)"
	       " i*A4, %u pages\n",
	       op->cups_args[ARGP_QUEUE], op->cups_args[ARGP_JOBID],
	       op->cups_args[ARGP_USER], cost->c, cost->m, cost->y, cost->k,
	       cost->t, cost->p);
	closelog();
}

void acct_mysql(const struct options *op, const struct costf *cost)
{
	char *qqueue = NULL, *quser = NULL, *qtitle = NULL, *sql_query;
	MYSQL *conn;

	if ((conn = lpacct_sql_start(op)) == NULL)
		return;

	asprintf(&sql_query,
		"insert into printlog (%s) values"
		" ('%s', %ld, '%s', '%s', %f, %f, %f, %f, %f, %u);",
		lpacct_sql_fields,
		vxutil_quote(op->cups_args[ARGP_QUEUE], VXQUOTE_SINGLE, &qqueue),
		strtol(op->cups_args[ARGP_JOBID], NULL, 0),
		vxutil_quote(op->cups_args[ARGP_USER],  VXQUOTE_SINGLE, &quser),
		vxutil_quote(op->cups_args[ARGP_TITLE], VXQUOTE_SINGLE, &qtitle),
		cost->c, cost->m, cost->y, cost->k, cost->t, cost->p
	);
	if (mysql_query(conn, sql_query) != 0)
		fprintf(stderr, "mysql_query: %s\n", mysql_error(conn));
	mysql_close(conn);
	free(qqueue);
	free(quser);
	free(qtitle);
	free(sql_query);
}
