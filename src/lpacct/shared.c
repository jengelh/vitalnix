/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.

    vxlpacct - Shared functions between lpacct_filter and lpaview
*/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/config.h>
#include <mysql.h>
#include "drop.h"
#include "global.h"
#include "lpacct.h"

// Variables
const char *const lpacct_sql_fields =
    "queue, jid, user, title, cyan, magenta, yellow, black, total, pages";

//-----------------------------------------------------------------------------
/*  lpacct_readconfig
    @p: config structure

    Fills @p with default values and then tries to update it with settings from
    the lpacct configuration file. Returns >0 on success, otherwise the error
    code.
*/
int lpacct_readconfig(struct options *p)
{
    struct HXoption options_table[] = {
        {.ln = "ACCT_MYSQL",  .type = HXTYPE_BOOL,   .ptr = &p->acct_mysql},
        {.ln = "ACCT_SYSLOG", .type = HXTYPE_BOOL,   .ptr = &p->acct_syslog},
        {.ln = "DPI",         .type = HXTYPE_UINT,   .ptr = &p->dpi},
        {.ln = "SQLHOST",     .type = HXTYPE_STRING, .ptr = &p->sqlhost},
        {.ln = "SQLUSER",     .type = HXTYPE_STRING, .ptr = &p->sqluser},
        {.ln = "SQLPW",       .type = HXTYPE_STRING, .ptr = &p->sqlpw},
        {.ln = "SQLDB",       .type = HXTYPE_STRING, .ptr = &p->sqldb},
        HXOPT_TABLEEND,
    };
    int ret;

    memset(p, 0, sizeof(*p));
    p->acct_syslog = 1;
    p->colorspace  = COLORSPACE_GRAY;
    p->dpi         = 300;
    
#define file CONFIG_SYSCONFDIR "/lpacct.conf"
    ret = HX_shconfig(file, options_table);
    if(ret <= 0) {
        fprintf(stderr, PREFIX "%s: Warning: Error parsing %s: %s\n",
                __func__, file, strerror(ret));
#undef file
        return ret;
    }

    if(p->sqlhost != NULL && *p->sqlhost == '\0')
        p->sqlhost = NULL;

    return 1;
}

MYSQL *lpacct_sql_start(const struct options *p)
{
    MYSQL *conn;
    void *chk;

    if((conn = mysql_init(NULL)) == NULL) {
        fprintf(stderr, PREFIX "mysql_init: %s\n", strerror(errno));
        return NULL;
    }

    chk = mysql_real_connect(conn, p->sqlhost, p->sqluser,
          p->sqlpw, p->sqldb, 0, NULL, 0);
    if(chk == NULL) {
        fprintf(stderr, PREFIX "mysql_real_connect: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

//=============================================================================
