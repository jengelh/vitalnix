/*
 *	lpacct/acct.h
 *	Copyright © CC Computer Consultants GmbH, 2006 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
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
 *	FUNCTIONS
 */
extern void acct_syslog(const struct options *, const struct costf *);
extern void acct_mysql(const struct options *, const struct costf *);

#endif /* LPACCT_ACCT_H */
