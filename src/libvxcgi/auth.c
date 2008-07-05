/*
 *	libvxcgi/auth.c - authentication helpers
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef _WIN32
#	include <sys/types.h>
#	include <sys/wait.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <security/pam_appl.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxcgi/libvxcgi.h>

/* Functions */
static int vxcgi_conv(int, const struct pam_message **,
	struct pam_response **, void *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int vxcgi_authenticate(const char *user,
    const char *password, const char *module)
{
	const struct pam_conv conv = {
		.conv        = vxcgi_conv,
		.appdata_ptr = static_cast(void *, const_cast(char *, password)),
	};
	pam_handle_t *ph;
	int ret;

	if (module == NULL)
		module = "login";

	if ((ret = pam_start(module, user, &conv, &ph)) != PAM_SUCCESS)
		return (ret < 0) ? ret : -ret;

	ret = pam_authenticate(ph, 0);
	pam_end(ph, ret);
	if (ret == PAM_SUCCESS)
		return 1;
	return (ret < 0) ? ret : -ret;
}

#ifndef _WIN32
EXPORT_SYMBOL int vxcgi_authenticate_ext(const char *user,
    const char *password, const char *prog)
{
	int fd[2], ret;
	long pid;

	if (pipe(fd) != 0) {
		perror("Unable to create pipe");
		return -errno;
	}

	if ((pid = fork()) < 0) {
		return -errno;
	} else if (pid == 0) {
		dup2(fd[0], STDIN_FILENO);
		close(fd[1]);
		close(fd[0]);
		execl(prog, prog, NULL);
		return 127;
	}

	write(fd[1], user, strlen(user));
	write(fd[1], "\n", 1);
	write(fd[1], password, strlen(password));
	write(fd[1], "\n", 1);
	close(fd[1]);
	waitpid(pid, &ret, 0);
	return WEXITSTATUS(ret) == EXIT_SUCCESS;
}
#endif

//-----------------------------------------------------------------------------
static int vxcgi_conv(int num_msg, const struct pam_message **msg_ap,
    struct pam_response **res_ap, void *ptr)
{
	int j;

	for (j = 0; j < num_msg; ++j) {
		switch (msg_ap[j]->msg_style) {
			case PAM_PROMPT_ECHO_ON:
				/* username given to pam already */
				return PAM_CONV_ERR;
			case PAM_PROMPT_ECHO_OFF:
				res_ap[j] = malloc(sizeof(struct pam_response));
				if (res_ap[j] == NULL)
					goto free_up;
				res_ap[j]->resp_retcode = PAM_SUCCESS;
				res_ap[j]->resp = HX_strdup(ptr);
				break;
			case PAM_TEXT_INFO:
				break;
			default:
				return PAM_CONV_ERR;
		}
	}
	return PAM_SUCCESS;

 free_up:
	while (j >= 0) {
		struct pam_response *r = res_ap[j];
		free(r->resp);
		free(r);
		--j;
	}
	errno = ENOMEM;
	return PAM_CONV_ERR;
}
