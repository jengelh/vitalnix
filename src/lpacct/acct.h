#ifndef LPACCT_ACCT_H
#define LPACCT_ACCT_H 1

struct options;
struct costf;

enum {
    ACCT_SYSLOG = 1 << 0,
    ACCT_MYSQL  = 1 << 1,
};

/*
 *      FUNCTIONS
 */
extern void acct_syslog(const struct options *, const struct costf *);
extern void acct_mysql(const struct options *, const struct costf *);

#endif // LPACCT_ACCT_H
