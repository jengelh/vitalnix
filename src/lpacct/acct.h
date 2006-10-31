#ifndef LPACCT_ACCT_H
#define LPACCT_ACCT_H 1

struct options;
struct costf;

/*
 *      FUNCTIONS
 */
extern void acct_syslog(const struct options *, const struct costf *);
extern void acct_mysql(const struct options *, const struct costf *);

#endif // LPACCT_ACCT_H
