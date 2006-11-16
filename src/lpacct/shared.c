/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.

    vxlpacct - Shared functions between lpacct_filter and lpaview
*/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libHX.h>
#include <mysql.h>
#include "drop.h"
#include "global.h"
#include "shared.h"

// Variables
const char *const lpacct_sql_fields =
    "queue, jid, user, title, cyan, magenta, yellow, black, total, pages";

//-----------------------------------------------------------------------------
void lpacct_costf_add(struct costf *out, const struct costf *in)
{
    out->c += in->c;
    out->m += in->m;
    out->y += in->y;
    out->k += in->k;
    out->t += in->c + in->m + in->y + in->k;
    out->p += in->p;
    return;
}

MYSQL *lpacct_sql_start(const char *file)
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
        errno = -ret;
        return NULL;
    }

    if((conn = mysql_init(NULL)) == NULL) {
        fprintf(stderr, PREFIX "mysql_init: %s\n", strerror(errno));
        return NULL;
    }

    if(*host == '\0')
        host = NULL;

    chk = mysql_real_connect(conn, host, user, pass, db, 0, NULL, 0);
    if(chk == NULL) {
        fprintf(stderr, PREFIX "mysql_real_connect: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

//=============================================================================
