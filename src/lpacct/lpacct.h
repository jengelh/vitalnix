/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006 - 2007
    This code is released under version 2.1 of the GNU LGPL.
*/
#ifndef LPACCT_LPACCT_H
#define LPACCT_LPACCT_H 1

#include <mysql.h>

/*
 *      DEFINITIONS
 */
enum colorspace {
    COLORSPACE_CMYK,
    COLORSPACE_CMYPK,
    COLORSPACE_CMY,
    COLORSPACE_GRAY,
};

enum {
    ARGP_QUEUE = 0,
    ARGP_JOBID,
    ARGP_USER,
    ARGP_TITLE,
    ARGP_COPIES,
    ARGP_OPTIONS,
    ARGP_FILE,
};

struct options {
    // general options
    unsigned int dpi;
    enum colorspace colorspace;

    // analyzer mode specific options
    unsigned int per_doc_stats, per_page_stats, rasterize;

    // filter mode specific options/objects
    unsigned int acct_mysql, acct_syslog;
    char *sqlhost, *sqluser, *sqlpw, *sqldb;
    const char **cups_args;
};

/*
 *      LPACCT.C
 */
extern void pr_exit(const char *, const char *, ...);
extern void pr_warn(const char *, const char *, ...);

/*
 *      SHARED.C
 */
extern const char *const lpacct_sql_fields;

extern int lpacct_readconfig(struct options *);
extern MYSQL *lpacct_sql_start(const struct options *);

#endif // LPACCT_LPACCT_H
