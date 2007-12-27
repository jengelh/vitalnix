/*
 *	libvxutil/phash.c - Password hashing
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
#include <iconv.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX.h>
#include <openssl/md4.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>
#ifdef HAVE_CRYPT_H
#	include <crypt.h>
#endif
#include "libvxutil/blowfish.h"
#define ICONV_NULL reinterpret_cast(iconv_t, -1)

static const char base64_set[] =
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static inline void vxutil_gensalt_base64(char *salt, int len)
{
	while (len-- > 0)
		*salt++ = base64_set[HX_irand(0, sizeof(base64_set) - 1)];
}

static char *vxutil_phash_des(const char *key, const char *salt)
{
#ifdef HAVE_CRYPT_H
	struct crypt_data cd = {};
	char auto_salt[3], *rx;

	if (salt == NULL) {
		vxutil_gensalt_base64(auto_salt, 2);
		auto_salt[sizeof(auto_salt)-1] = '\0';
		salt = auto_salt;
	}
	rx = crypt_r(key, salt, &cd);
	return (rx != NULL) ? HX_strdup(rx) : NULL;
#else
	fprintf(stderr, "%s: DES crypt not supported\n", __func__);
	return NULL;
#endif
}

static char *vxutil_phash_md5(const char *key, const char *salt)
{
#ifdef HAVE_CRYPT_H
	struct crypt_data cd = {};
	char auto_salt[12], *rx;

	if (salt == NULL) {
		strcpy(auto_salt, "$1$");
		vxutil_gensalt_base64(&auto_salt[3], 8);
		auto_salt[sizeof(auto_salt)-1] = '\0';
		salt = auto_salt;
	}
	rx = crypt_r(key, salt, &cd);
	return (rx != NULL) ? HX_strdup(rx) : NULL;
#else
	fprintf(stderr, "%s: MD5 crypt not supported\n", __func__);
	return NULL;
#endif
}

static char *vxutil_phash_blowfish(const char *key, const char *salt)
{
	char auto_salt[30], res[64], *rx;

	if (salt == NULL) {
		strcpy(auto_salt, "$2a$05$");
		vxutil_gensalt_base64(&auto_salt[7], 22);
		auto_salt[sizeof(auto_salt)-1] = '\0';
		salt = auto_salt;
	}
	rx = _crypt_blowfish_rn(key, salt, res, sizeof(res));
	return (rx != NULL) ? HX_strdup(rx) : NULL;
}

static char *vxutil_phash_smbnt(const char *key, const char *salt)
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

static char *(*const hashfn[])(const char *, const char *) = {
	[VXPHASH_DES]      = vxutil_phash_des,
	[VXPHASH_MD5]      = vxutil_phash_md5,
	[VXPHASH_BLOWFISH] = vxutil_phash_blowfish,
	[VXPHASH_SMBNT]    = vxutil_phash_smbnt,
};

EXPORT_SYMBOL bool vxutil_phash(const char *key, const char *salt,
    unsigned int meth, char **result)
{
	if (meth >= ARRAY_SIZE(hashfn) || hashfn[meth] == NULL)
		return false;
	*result = hashfn[meth](key, salt);
	return *result != NULL;
}
