/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.

    Accounting boilerplate for lpacct, to a simple syslog or a more
    advanced mysql target.
*/

#define _GNU_SOURCE 1 // for asprintf
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <libHX.h>
#include <mysql.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxutil/libvxutil.h>
#include "acct.h"
#include "drop.h"
#include "global.h"
#include "lpacct.h"

// Functions
static MYSQL *acct_mysql_init(const char *);

//-----------------------------------------------------------------------------
void acct_syslog(const struct options *op, const struct costf *cost)
{
    openlog(PREFIX, LOG_NDELAY | LOG_PID, LOG_LPR);
    syslog(LOG_INFO, "q/id=%s/%s user=%s +cmyk(%.6f, %.6f, %.6f, %.6f) i*m^2\n",
           op->cups_args[ARGP_QUEUE], op->cups_args[ARGP_JOBID],
           op->cups_args[ARGP_USER], cost->c, cost->m, cost->y, cost->k);
    closelog();
    return;
}

void acct_mysql(const struct options *op, const struct costf *cost)
{
    char *qqueue, *quser, *qtitle, *sql_query;
    MYSQL *conn;

    if((conn = acct_mysql_init(CONFIG_SYSCONFDIR "/lpacct.conf")) == NULL)
        return;

    vxutil_quote(op->cups_args[ARGP_QUEUE], VXQUOTE_SINGLE, &qqueue);
    vxutil_quote(op->cups_args[ARGP_TITLE], VXQUOTE_SINGLE, &qtitle);
    vxutil_quote(op->cups_args[ARGP_USER],  VXQUOTE_SINGLE, &quser);
    asprintf(&sql_query,
        "insert into printlog "
        "(queue, jid, user, title, cyan, magenta, yellow, black, total) values "
        "('%s', %ld, '%s', '%s', %.6f, %.6f, %.6f, %.6f, %.6f);",
        qqueue, strtol(op->cups_args[ARGP_JOBID], NULL, 0),
        quser, qtitle, cost->c, cost->m, cost->y, cost->k, cost->t
    );
    if(mysql_query(conn, sql_query) != 0)
        fprintf(stderr, "mysql_query: %s\n", mysql_error(conn));
    mysql_close(conn);
    free(qqueue);
    free(quser);
    free(qtitle);
    free(sql_query);
    return;
}

//-----------------------------------------------------------------------------
static MYSQL *acct_mysql_init(const char *file)
{
    char *host = NULL, *user = NULL, *pass = NULL, *db = NULL;
    struct HXoption options_table[] = {
        {.ln = "HOST", .type = HXTYPE_STRING, .ptr = &host},
        {.ln = "USER", .type = HXTYPE_STRING, .ptr = &user},
        {.ln = "PASS", .type = HXTYPE_STRING, .ptr = &pass},
        {.ln = "DB",   .type = HXTYPE_STRING, .ptr = &db},
        HXOPT_TABLEEND,
    };
    MYSQL *conn;
    void *chk;
    int ret;

    if((ret = HX_shconfig(file, options_table)) <= 0) {
        fprintf(stderr, PREFIX "%s: Warning: Error parsing %s: %s\n",
                __func__, file, strerror(ret));
    }

    if((conn = acct_mysql_init(NULL)) == NULL) {
        fprintf(stderr, PREFIX "mysql_init: %s\n", strerror(errno));
        return NULL;
    }

    chk = mysql_real_connect(conn, host, user, pass, db, 0, NULL, 0);
    if(chk == NULL) {
        fprintf(stderr, PREFIX "mysql_real_connect: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

//=============================================================================
