/*
 *	libvxutil/crypt.c - Password encryption
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2003 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>
#include "libvxutil/blowfish.h"

#ifdef __gnu_linux__ /* I think this may be used to detect GNU libc */
#	define crypt3
#	ifndef __USE_GNU
#		define GNU_WAS_NOT_IN 1 /* dirty hack */
#		define __USE_GNU 1
#	endif
#	define __USE_GNU 1 /* dirty hack */
#	include <crypt.h>
#	ifdef GNU_WAS_NOT_IN
#		undef __USE_GNU
#	endif
#	undef GNU_WAS_NOT_IN
#endif

/* Functions */
static char *vxutil_crypt_des(const char *, const char *);
static char *vxutil_crypt_md5(const char *, const char *);
static char *vxutil_crypt_blowfish(const char *, const char *);
static inline void gensalt_az(char *, ssize_t);
static inline void gensalt_bin(char *, ssize_t);

/* Variables */
static char *(*const cryptor[])(const char *, const char *) = {
	[CRYPW_DES]      = vxutil_crypt_des,
	[CRYPW_MD5]      = vxutil_crypt_md5,
	[CRYPW_BLOWFISH] = vxutil_crypt_blowfish,
};

//-----------------------------------------------------------------------------
EXPORT_SYMBOL bool vxutil_cryptpw(const char *key, const char *salt,
    unsigned int meth, char **result)
{
	if (meth >= ARRAY_SIZE(cryptor) || cryptor[meth] == NULL ||
	    (*result = cryptor[meth](key, salt)) == NULL)
		return 0;

	return 1;
}

//-----------------------------------------------------------------------------
static char *vxutil_crypt_des(const char *key, const char *salt)
{
#ifdef crypt3
	struct crypt_data cd = {};
	char my_salt[3], *rx;

	if (salt == NULL) {
		gensalt_az(my_salt, sizeof(my_salt));
		salt = my_salt;
	}
	rx = crypt_r(key, salt, &cd);
	return (rx != NULL) ? HX_strdup(rx) : NULL;
#else
	fprintf(stderr, "%s: DES crypt not supported on non-Glibc\n", __func__);
	return NULL;
#endif
}

static char *vxutil_crypt_md5(const char *key, const char *salt)
{
#ifdef crypt3
	struct crypt_data cd = {};
	char my_salt[12], *rx;

	if (salt == NULL) {
		my_salt[0] = '$';
		my_salt[1] = '1';
		my_salt[2] = '$';
		gensalt_az(&my_salt[3], 8+1);
		my_salt[11] = '$';
		salt = my_salt;
	}
	rx = crypt_r(key, salt, &cd);
	return (rx != NULL) ? HX_strdup(rx) : NULL;
#else
	fprintf(stderr, "%s: MD5 crypt not supported on non-Glibc\n", __func__);
	return NULL;
#endif
}

static char *vxutil_crypt_blowfish(const char *key, const char *salt)
{
	char my_salt[64], res[64], *rx;

	if (salt == NULL) {
		strncpy(my_salt, "$2a$05$", sizeof(my_salt));
		gensalt_az(&my_salt[7], 22+1);
		salt = my_salt;
	}
	rx = _crypt_blowfish_rn(key, salt, res, sizeof(res));
	return (rx != NULL) ? HX_strdup(rx) : NULL;
}

//-----------------------------------------------------------------------------
static inline void gensalt_az(char *salt, ssize_t len)
{
	static const char az_pool[] =
		"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	while (--len > 0)
		*salt++ = az_pool[HX_irand(0, sizeof(az_pool) - 1)];
	*salt++ = '\0';
	return;
}

static inline void gensalt_bin(char *salt, ssize_t len)
{
	while (len-- > 0)
		*salt++ = HX_irand(0, 256);
	return;
}

//=============================================================================
