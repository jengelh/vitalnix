/*
 *	ntactiv - activate NT password
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2007 - 2009
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <libHX/map.h>
#include <vitalnix/libvxcgi/libvxcgi.h>
#include <vitalnix/libvxutil/libvxutil.h>

static void header(void)
{
	printf(
	"Content-Type: text/html\n\n"
	"<html>\n"
	"<head>\n"
	"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
	"<link rel=\"stylesheet\" href=\"/intranet.css\" type=\"text/css\">\n"
	"</head>\n"
	"<body>\n"
	"<div align=\"center\">\n"
	"<p>&nbsp;</p>\n"
	"<p>&nbsp;</p>\n"
	"<p style=\"font-size: 16pt;\"><b>Update NT password from UNIX password</b></p>\n"
	);
}

static void form(const char *username)
{
	printf(
		"<form action=\"%s\" method=\"POST\">\n"
		"<table>\n"
		"	<tr>\n"
		"		<td>Username:</td>\n"
		"		<td><input type=\"text\" name=\"username\"\n"
		"		value=\"%s\" /></td>\n"
		"	</tr>\n"
		"	<tr>\n"
		"		<td>Password:</td>\n"
		"		<td><input type=\"password\" name=\"password\" /></td>\n"
		"	</tr>\n"
		"	<tr>\n"
		"		<td>&nbsp;</td>\n"
		"		<td><input type=\"submit\"\n"
		"		value=\"Activate NT logon\" /></td>\n"
		"	</tr>\n"
		"</table>\n"
		"</form>\n"
		"</body>\n"
		"</html>\n", getenv("REQUEST_URI"), username);
}

static bool update_ntpassword(const char *username, const char *password)
{
	char command[64];
	uid_t old_uid;
	FILE *fp;
	int ret;

	if (!vxutil_valid_username(username) ||
	    vxcgi_authenticate(username, password, NULL) <= 0) {
		printf("<p>Username and/or password incorrect.</p>\n");
		return false;
	}

	snprintf(command, sizeof(command), "smbpasswd -as %s >&2", username);
	old_uid = getuid();
	setreuid(geteuid(), -1);
	fp = popen(command, "w");
	setreuid(old_uid, old_uid);
	if (fp == NULL) {
		printf("<p>Could not start <code>smbpasswd</code></p>\n");
		return false;
	}

	fprintf(fp, "%s\n%s\n", password, password);
	ret = pclose(fp);
	if (ret != 0) {
		printf("<p>Setting NT password failed. Error code: %d\n", ret);
		return false;
	}

	printf("<p>Success.</p>");
	return true;
}

int main(int argc, const char **argv)
{
	struct HXmap *data = vxcgi_split(vxcgi_read_data(argc, argv));
	const char *username = vxutil_azstr(HXmap_get(data, "username"));
	const char *password = vxutil_azstr(HXmap_get(data, "password"));

	header();

	if (*username != '\0' && *password != '\0')
		if (update_ntpassword(username, password))
			username = "";

	form(username);
	HXmap_free(data);
	return EXIT_SUCCESS;
}
