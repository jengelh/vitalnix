/*
 *	ihlogon.c -
 *	Copyright © CC Computer Consultants GmbH, 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#define PAM_SM_SESSION 1
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <libHX.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxdb/libvxpdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#define MAXLNLEN 1024

enum slice_type {
	D_NONE = 0,
	D_ALLOW,
	D_DENY,
};

enum deny_reason {
	REASON_MULTILOGON = 1 << 0,
	REASON_TIMESLICE  = 1 << 1,
};

struct time_period {
	enum slice_type op;
	time_t from, to;
};

#define TABLE_END   {D_NONE}
#define D_ALLOW_ALL {D_ALLOW, TIMESPEC(0, 0, 0), TIMESPEC(24, 0, 0)}
#define D_DENY_ALL  {D_DENY,  TIMESPEC(0, 0, 0), TIMESPEC(24, 0, 0)}
#define TIMESPEC(__hour, __min, __sec) \
	((__hour) * 3600 + (__min) * 60 + (__sec))
const struct time_period timetable_sek1[] = {
	D_ALLOW_ALL,
	{D_DENY, TIMESPEC( 6,  0, 0), TIMESPEC( 7, 50, 0)},
	{D_DENY, TIMESPEC( 9, 25, 0), TIMESPEC( 9, 37, 0)},
	{D_DENY, TIMESPEC(11, 15, 0), TIMESPEC(11, 27, 0)},
	TABLE_END,
};
const struct time_period timetable_sek2[] = {
	D_ALLOW_ALL,
	{D_DENY, TIMESPEC( 9, 25, 0), TIMESPEC( 9, 37, 0)},
	{D_DENY, TIMESPEC(11, 15, 0), TIMESPEC(11, 27, 0)},
	TABLE_END,
};
#undef TABLE_END

#define PAM_DENY PAM_PERM_DENIED

//-----------------------------------------------------------------------------
static inline bool is_schueler(const struct vxpdb_user *u)
{
	return u->pw_gid == 600 /* schueler */ || u->pw_gid == 604 /* extsc */;
/*	return g != NULL && (strcmp(g, "schueler") == 0 ||
	       strcmp(g, "extsc") == 0); */
}

static unsigned int login_times(const char *user)
{
	char buf[MAXLNLEN];
	FILE *fp;

	snprintf(buf, sizeof(buf),
		"w -h | "
		/* filter user */
		"grep \"^%s \" | "
		/* only show samba sessions */
		"awk '{print $2}' | grep ^lgn | "
		/* count logins */
		"wc -l",
		user
	);

	fp = popen(buf, "r");
	fgets(buf, sizeof(buf), fp);
	return strtol(buf, NULL, 0);
}

static time_t ourtime(void)
{
	/* Returns the current time in seconds since beginning of the day */
	time_t now = time(NULL);
	struct tm n;
	localtime_r(&now, &n);
	return n.tm_hour * 3600 + n.tm_min * 60 + n.tm_sec;
}

static unsigned int get_vitalnixgroup(const struct vxpdb_user *user)
{
	if (user->vs_pvgrp != NULL)
		return strtoul(user->vs_pvgrp, NULL, 0);
	return 0;
}

static bool ts_allowed(time_t now, const struct time_period *tbp)
{
	/* Returns whether logins are allowed in the current timeslice */
	bool allow = true;

	while (tbp->op != D_NONE) {
		if ((tbp->from <= tbp->to && now >= tbp->from &&
		    now <= tbp->to) || (tbp->from > tbp->to &&
		    (now >= tbp->from || now <= tbp->to))) {
			if (tbp->op == D_ALLOW)
				allow = true;
			if (tbp->op == D_DENY)
				allow = false;
		}
		++tbp;
	}

	return allow;
}

static int ihlogon(const char *user, const char *rhost,
    struct vxpdb_user *info, struct vxpdb_state *db, unsigned int mask)
{
	time_t now = ourtime();
	unsigned int pgrp;
	int ret;

	if ((ret = vxpdb_getpwnam(db, user, info)) < 0) {
		syslog(LOG_INFO, "vxpdb_userinfo(): %s\n", strerror(ret));
		return PAM_IGNORE;
	} else if (ret == 0) {
		syslog(LOG_INFO, "User \"%s\" does not exist\n", user);
		return PAM_IGNORE;
	}

	if (!is_schueler(info)) {
		syslog(LOG_INFO, "\"%s\" does not have \"schueler\" as "
		       "primary group - skipping\n", user);
		return PAM_SUCCESS;
	}

	pgrp = get_vitalnixgroup(info);
	if ((mask & REASON_MULTILOGON) && login_times(user) >= 1) {
		syslog(LOG_INFO, "\"%s\" (pgrp=%u) already logged in once (this rhost=%s)\n",
		       user, pgrp, rhost);
		return PAM_DENY;
	}

	if (mask & REASON_TIMESLICE)
		if ((pgrp <= 10 && !ts_allowed(now, timetable_sek1)) ||
		    (pgrp >= 11 && !ts_allowed(now, timetable_sek2))) {
			/* Block timeslices */
			syslog(LOG_INFO, "\"%s\" blocked due to timeslice "
			       "(pgrp=%u)\n", user, pgrp);
			return PAM_DENY;
		}

	syslog(LOG_INFO, "Accepted %s (pgrp=%u)\n", user, pgrp);
	return PAM_SUCCESS;
}

static int ihlogon_init(const char *user, const char *rhost, unsigned int mask)
{
	struct vxpdb_user uinfo = {};
	struct vxpdb_state *db;
	int ret;

	if ((db = vxpdb_load("*")) == NULL) {
		syslog(LOG_INFO, "vxpdb_load() failed: %s\n", strerror(errno));
		return PAM_SUCCESS;
	}

	if ((ret = vxpdb_open(db, 0)) <= 0) {
		syslog(LOG_INFO, "vxpdb_open(): %s\n", strerror(ret));
		vxpdb_unload(db);
		return PAM_SUCCESS;
	}

	ret = ihlogon(user, rhost, &uinfo, db, mask);
	vxpdb_close(db);
	vxpdb_unload(db);
	vxpdb_user_free(&uinfo, false);
	return ret;
}

PAM_EXTERN EXPORT_SYMBOL int pam_sm_acct_mgmt(pam_handle_t *pamh,
    int flags, int argc, const char **argv)
{
	const char *user = NULL, *rhost = NULL;
	unsigned int mask = REASON_MULTILOGON;
	int ret;

	openlog("pam_ihlogon", LOG_PERROR | LOG_PID, LOG_AUTHPRIV);

	ret = pam_get_item(pamh, PAM_USER, static_cast(const void **,
	      static_cast(const void *, &user)));
	if (ret != PAM_SUCCESS) {
		syslog(LOG_ERR, "pam_get_item(PAM_USER): %s\n", pam_strerror(pamh, ret));
		return PAM_SERVICE_ERR;
	}

	ret = pam_get_item(pamh, PAM_RHOST, static_cast(const void **,
	      static_cast(const void *, &rhost)));
	if (ret != PAM_SUCCESS) {
		syslog(LOG_ERR, "pam_get_item(PAM_RHOST): %s\n", pam_strerror(pamh, ret));
		return PAM_SERVICE_ERR;
	}

	if (user == NULL || rhost == NULL)
		return PAM_IGNORE;

	if (tolower(rhost[0]) == 'b' && isdigit(rhost[1]) &&
	    isdigit(rhost[2]) && strcmp(&rhost[3], ".site") == 0)
		mask |= REASON_TIMESLICE;

	ret = ihlogon_init(user, rhost, mask);
	closelog();
	return ret;
}

#ifdef STANDALONE_DEBUG
int main(int argc, const char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "%s: Need an username\n", *argv);
		return 127;
	}
	return ihlogon_init(argv[1], "localhost", ~0U);
}
#endif
