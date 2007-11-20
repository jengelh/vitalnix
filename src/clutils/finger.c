/*
 *	vxfinger - Search for user and display info
 *	Copyright © CC Computer Consultants GmbH, 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <libHX.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>
#ifdef HAVE_LASTLOG_H
#	include <lastlog.h>
#endif
#ifdef HAVE_PATHS_H
#	include <paths.h>
#endif
#ifdef HAVE_UTMPX_H
#	include <utmpx.h>
#endif
#ifndef _PATH_LASTLOG
#	define _PATH_LASTLOG "/var/log/lastlog"
#endif

/* Functions */
static bool get_options(int *, const char ***);
static void finger_grep(struct vxpdb_state *, const char *);
static bool check_utmp(const char *);
static void check_lastlog(long);
static void check_mail(const char *);

/* Variables */
static char *Database         = "*";
static unsigned int Icase     = true;
static unsigned int Fullgecos = false;
static const char *grep_color = "1;31";
static const char *stop_color = "0";

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct vxpdb_state *db;
	char *s;
	int ret;

	if (!get_options(&argc, &argv))
		return EXIT_FAILURE;

	s = getenv("GREP_COLOR");
	if (s != NULL)
		grep_color = s;

	if ((db = vxpdb_load(Database)) == NULL) {
		perror("Error loading database");
		return EXIT_FAILURE;
	}

	if ((ret = vxpdb_open(db, 0)) <= 0) {
		vxpdb_unload(db);
		fprintf(stderr, "Error opening database: %s\n",
		        strerror(-ret));
		return EXIT_FAILURE;
	}

	while (*++argv != NULL) {
		if (!Icase) {
			finger_grep(db, *argv);
			continue;
		}
		s = HX_strdup(*argv);
		HX_strlower(s);
		finger_grep(db, s);
		free(s);
	}

	vxpdb_close(db);
	vxpdb_unload(db);
	return EXIT_SUCCESS;
}

static bool get_options(int *argc, const char ***argv)
{
	static struct HXoption options_table[] = {
		{.sh = 'I', .type = HXTYPE_VAL, .val = false, .ptr = &Icase,
		 .help = "Use case-sensitive matching"},
		{.sh = 'g', .type = HXTYPE_NONE, .ptr = &Fullgecos,
		 .help = "Display and match the full GECOS field"},
		{.sh = 'i', .type = HXTYPE_VAL, .val = true, .ptr = &Icase,
		 .help = "Use case-insensitive matching (default)"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void finger_grep(struct vxpdb_state *db, const char *keyword)
{
	const unsigned int keyword_len = strlen(keyword);
	const char *name_ptr, *real_ptr;
	struct vxpdb_group group = {};
	struct vxpdb_user user = {};
	hmc_t *lc_name, *lc_real;
	void *trav;
	char *p;

	if ((trav = vxpdb_usertrav_init(db)) == NULL) {
		fprintf(stderr, "vxpdb_usertrav_init: %s\n", strerror(errno));
		return;
	}

	lc_name = hmc_sinit("abcdefgh");
	lc_real = hmc_sinit("abcdefgh");
	hmc_trunc(&lc_name, 0);
	hmc_trunc(&lc_real, 0);

	while (vxpdb_usertrav_walk(db, trav, &user) > 0) {
		if (!Fullgecos && user.pw_real != NULL &&
		    (p = strchr(user.pw_real, ',')) != NULL)
			*p = '\0';

		if (user.pw_real == NULL)
			user.pw_real = hmc_sinit("");

		if (Icase) {
			hmc_strasg(&lc_name, user.pw_name);
			hmc_strasg(&lc_real, user.pw_real);
			HX_strlower(lc_name);
			HX_strlower(lc_real);
			name_ptr = strstr(lc_name, keyword);
			real_ptr = strstr(lc_real, keyword);
		} else {
			name_ptr = strstr(user.pw_name, keyword);
			real_ptr = strstr(user.pw_real, keyword);
		}

		if (name_ptr == NULL && real_ptr == NULL)
			continue;

		if (Icase) {
			if (name_ptr != NULL)
				name_ptr = user.pw_name + (name_ptr - lc_name);
			else
				name_ptr = user.pw_name + strlen(user.pw_name);
			if (real_ptr != NULL)
				real_ptr = user.pw_real + (real_ptr - lc_real);
			else
				real_ptr = user.pw_real + strlen(user.pw_real);
		}


		if (isatty(STDOUT_FILENO)) {
			unsigned int z0 = name_ptr - user.pw_name;
			unsigned int z1 = (*name_ptr != '\0') ? keyword_len : 0;
			unsigned int z2 = (*real_ptr != '\0') ? keyword_len : 0;
			unsigned int z3 = real_ptr - user.pw_real;

			printf(
				"Login: %-.*s\e[%sm%-.*s\e[%sm%-*s  ", // ]
				z0, user.pw_name,
				grep_color,
				z1, name_ptr, stop_color,
				24 - z0 - z1, &name_ptr[z1]);
			printf(
				"Name: %-.*s\e[%sm%-.*s\e[%sm%s\n", // ]
				z3, user.pw_real,
				grep_color,
				z2, real_ptr, stop_color,
				&real_ptr[z2]);
		} else {
			printf("Login: %-24s  Name: %s\n",
			       user.pw_name, user.pw_real);
		}

		printf(
			"Group: %-24s  Private Group: %s\n"
			"Shell: %-24s  Directory: %s\n",
			(vxpdb_getgrgid(db, user.pw_gid, &group) < 0) ?
			"-" : group.gr_name,
			(user.vs_pvgrp == NULL) ? "-" : user.vs_pvgrp,
			user.pw_shell, user.pw_home
		);

		if (!check_utmp(user.pw_name))
			check_lastlog(user.pw_uid);
		check_mail(user.pw_name);
		printf("\n");
	}

	hmc_free(lc_name);
	hmc_free(lc_real);
	vxpdb_user_free(&user, false);
	vxpdb_usertrav_free(db, trav);
	return;
}

static bool check_utmp(const char *user)
{
	bool ret = false;
#ifdef HAVE_UTMPX_H
	struct utmpx *ut;
	struct stat sb;
	char buf[32];
	time_t now;

	setutxent();
	while ((ut = getutxent()) != NULL) {
		if (ut->ut_type != USER_PROCESS ||
		    strcmp(ut->ut_user, user) != 0)
			continue;
		if (!(kill(ut->ut_pid, 0) == 0 || errno != ESRCH))
			continue;

		ret = true;
		now = ut->ut_tv.tv_sec;
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
		printf("On since %s on %s", buf, ut->ut_line);

		snprintf(buf, sizeof(buf), "/dev/%s", ut->ut_line);
		now = time(NULL);


		if (stat(buf, &sb) == 0 && now - sb.st_atime > 60) {
			if (access(buf, W_OK) < 0)
				printf(" (messages off)");

			printf(", idle %uh:%02um",
			       static_cast(unsigned int, now - sb.st_atime) / 3600,
			       static_cast(unsigned int, ((now - sb.st_atime) / 60) % 60));
			if (*ut->ut_host != '\0')
				printf(",");
		}

		if (*ut->ut_host != '\0')
			printf(" from %s", ut->ut_host);
		printf("\n");
	}
	endutxent();
#endif
	return ret;
}

static void check_lastlog(long uid)
{
#ifdef HAVE_LASTLOG_H
	struct lastlog entry;
	char timestamp[32];
	time_t tmp;
	int fd;

	if ((fd = open(_PATH_LASTLOG, O_RDONLY)) < 0)
		goto never;
	if (lseek(fd, uid * sizeof(struct lastlog), SEEK_SET) !=
	    uid * sizeof(struct lastlog) ||
	    read(fd, &entry, sizeof(entry)) < sizeof(entry)) {
		close(fd);
		goto never;
	}

	tmp = entry.ll_time;
	strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S",
	         localtime(&tmp));
	printf("Last login %s", timestamp);
	if (*entry.ll_line != '\0')
		printf(" on %s", entry.ll_line);
	if (*entry.ll_host != '\0')
		printf(" from %s", entry.ll_host);
	printf("\n");

	close(fd);
	return;
#endif

 never:
	printf("Never logged in.\n");
	return;
}

static void check_mail(const char *user)
{
	char filename[256], astr[32], mstr[32];
	struct stat sb;
	time_t tmp;

	snprintf(filename, sizeof(filename), "/var/mail/%s", user);
	if (stat(filename, &sb) != 0) {
		printf("No mail in %s\n", filename);
		return;
	}

	tmp = sb.st_atime;
	strftime(astr, sizeof(astr), "%Y-%m-%d %H:%M", localtime(&tmp));
	tmp = sb.st_mtime;
	strftime(mstr, sizeof(mstr), "%Y-%m-%d %H:%M", localtime(&tmp));

	if (sb.st_mtime + 60 >= sb.st_atime)
		printf("Mail last read on %s\n", astr);
	else
		printf("New mail received %s\n" "     Unread since %s\n",
		       mstr, astr);
	return;
}
