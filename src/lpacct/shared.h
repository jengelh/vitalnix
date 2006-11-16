#ifndef LPACCT_SHARED_H
#define LPACCT_SHARED_H 1

#include <mysql.h>

struct costf;

extern const char *const lpacct_sql_fields;

/*
 *      CONN.C
 */
extern void lpacct_costf_add(struct costf *, const struct costf *);
extern MYSQL *lpacct_sql_start(const char *);

#endif // LPACCT_SHARED_H
