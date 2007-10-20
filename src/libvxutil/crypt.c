/*
 *	libvxutil/crypt.c - Password encryption
 *	Copyright © CC Computer Consultants GmbH, 2003 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#define _GNU_SOURCE 1
#include <sys/types.h>
#include <crypt.h>
#include <iconv.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX.h>
#include <openssl/md4.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>
#include "libvxutil/blowfish.h"
#define crypt3 1
#define ICONV_NULL reinterpret_cast(iconv_t, -1)

/* Functions */
static char *vxutil_crypt_des(const char *, const char *);
static char *vxutil_crypt_smbnt(const char *, const char *);
static char *vxutil_crypt_md5(const char *, const char *);
static char *vxutil_crypt_blowfish(const char *, const char *);
static inline void gensalt_az(char *, ssize_t);
static inline void gensalt_bin(char *, ssize_t);

/* Variables */
static char *(*const cryptor[])(const char *, const char *) = {
	[CRYPW_DES]      = vxutil_crypt_des,
	[CRYPW_MD5]      = vxutil_crypt_md5,
	[CRYPW_BLOWFISH] = vxutil_crypt_blowfish,
	[CRYPW_SMBNT]    = vxutil_crypt_smbnt,
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

static char *vxutil_crypt_smbnt(const char *key, const char *salt)
{
	unsigned char md[MD4_DIGEST_LENGTH];
	char mdhex[2*MD4_DIGEST_LENGTH+1];
	size_t inl, outl, outl_orig;
	char *outp, *out;
	iconv_t cd;

	if (key == NULL || *key == '\0')
		return NULL;
	if ((cd = iconv_open("ucs-2le", "utf-8")) == ICONV_NULL)
		return NULL;
	inl  = strlen(key);
	outl = outl_orig = 2 * inl;
	if ((outp = out = malloc(outl + 1)) == NULL) {
		iconv_close(cd);
		return NULL;
	}
	iconv(cd, static_cast(char **, static_cast(void *, &key)), &inl, &outp, &outl);
	iconv_close(cd);

	MD4(signed_cast(const unsigned char *, out), outl_orig - outl, md);
#define B "%02X"
#define D B B B B
	snprintf(mdhex, sizeof(mdhex), D D D D, md[0], md[1], md[2], md[3],
	         md[4], md[5], md[6], md[7], md[8], md[9], md[10], md[11],
	         md[12], md[13], md[14], md[15]);
#undef B
#undef D
	return HX_strdup(mdhex);
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
