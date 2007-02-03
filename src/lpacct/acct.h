/*
    lpacct/acct.h
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
*/
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
