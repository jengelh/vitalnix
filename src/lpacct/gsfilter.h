#ifndef LPACCT_GSFILTER_H
#define LPACCT_GSFILTER_H 1

#include <sys/types.h>

/*
 *      FUNCTIONS
 */
extern int ghostscript_init(const char *, pid_t *, int);
extern void ghostscript_exit(pid_t);

#endif // LPACCT_GSFILTER_H
