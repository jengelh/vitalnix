/*
 *	libvxutil/phash.c - Password hashing
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 *
 *	Blowfish phash imported from http://freshmeat.net/p/crypt_blowfish/.
 *	MD5 and SHA phash based upon Ulrich Drepper's paper on SHA password
 *	hashing. http://people.redhat.com/drepper/SHA-crypt.txt
 */
#define _GNU_SOURCE 1
#include <sys/types.h>
#include <iconv.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/misc.h>
#include <libHX/string.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>
#ifdef HAVE_CRYPT_H
#	include <crypt.h>
#endif
#include "libvxutil/blowfish.h"
#define ICONV_NULL reinterpret_cast(iconv_t, -1)

/* Definitions */
struct crypto_work {
	const char *prefix, *key, *salt;
	unsigned int prefix_len, key_len, salt_len, buffer_size;
	union {
		struct { /* SHA */
			void *p, *s;
			unsigned int rounds;
			bool rounds_sel;
		};
	};
};

/* Variables */
static const char s_rounds[] = "rounds=";
static const char base64_set[] =
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

//-----------------------------------------------------------------------------
static inline void vxutil_gensalt_base64(char *salt, int len)
{
	while (len-- > 0)
		*salt++ = base64_set[HX_irand(0, sizeof(base64_set) - 1)];
}

static inline char *vxutil_phash_base64(char *hash, unsigned int i,
    unsigned char a, unsigned char b, unsigned char c)
{
	unsigned int w = (a << 16) | (b << 8) | c;
	for (; i > 0; --i, w >>= 6)
		*hash++ = base64_set[w & 0x3F];
	return hash;
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

static bool vxutil_phash_md5init(struct crypto_work *w)
{
	w->prefix_len = strlen(w->prefix);

	if (strncmp(w->salt, w->prefix, w->prefix_len) != 0)
		return false;

	w->key_len  = strlen(w->key);
	w->salt    += w->prefix_len;
	w->salt_len = strcspn(w->salt, "$");
	if (w->salt_len > 8)
		w->salt_len = 8;

	/* +1 for '$', +1 for final '\0' */
	w->buffer_size += w->prefix_len + w->salt_len + 2;
	return true;
}

static void vxutil_phash_md5xfrm(const struct crypto_work *w, char *hash)
{
	unsigned char alt_result[MD5_DIGEST_LENGTH];
	MD5_CTX ctx, alt_ctx;
	char *p;
	int i;

	MD5_Init(&ctx);
	MD5_Update(&ctx, w->key, w->key_len);
	MD5_Update(&ctx, w->prefix, w->prefix_len);
	MD5_Update(&ctx, w->salt, w->salt_len);

	MD5_Init(&alt_ctx);
	MD5_Update(&alt_ctx, w->key, w->key_len);
	MD5_Update(&alt_ctx, w->salt, w->salt_len);
	MD5_Update(&alt_ctx, w->key, w->key_len);
	MD5_Final(alt_result, &alt_ctx);

	for (i = w->key_len; i > sizeof(alt_result); i -= sizeof(alt_result))
		MD5_Update(&ctx, alt_result, sizeof(alt_result));
	MD5_Update(&ctx, alt_result, i);

	*alt_result = '\0';
	for (i = w->key_len; i > 0; i >>= 1)
		if (i & 1)
			MD5_Update(&ctx, alt_result, 1);
		else
			MD5_Update(&ctx, w->key, 1);

	MD5_Final(alt_result, &ctx);

	for (i = 0; i < 1000; ++i) {
		MD5_Init(&ctx);
		if (i & 1)
			MD5_Update(&ctx, w->key, w->key_len);
		else
			MD5_Update(&ctx, alt_result, sizeof(alt_result));
		if (i % 3 != 0)
			MD5_Update(&ctx, w->salt, w->salt_len);
		if (i % 7 != 0)
			MD5_Update(&ctx, w->key, w->key_len);
		if (i & 1)
			MD5_Update(&ctx, alt_result, sizeof(alt_result));
		else
			MD5_Update(&ctx, w->key, w->key_len);
		MD5_Final(alt_result, &ctx);
	}

	strcpy(hash, w->prefix);
	strncat(hash, w->salt, w->salt_len);
	strcat(hash, "$");
#define b64(a, b, c, i) p = vxutil_phash_base64(p, (i), (a), (b), (c))
	p = hash + strlen(hash);
	b64(alt_result[0], alt_result[6], alt_result[12], 4);
	b64(alt_result[1], alt_result[7], alt_result[13], 4);
	b64(alt_result[2], alt_result[8], alt_result[14], 4);
	b64(alt_result[3], alt_result[9], alt_result[15], 4);
	b64(alt_result[4], alt_result[10], alt_result[5], 4);
	b64(0, 0, alt_result[11], 2);
	*p = '\0';
#undef b64

	memset(&ctx, 0, sizeof(ctx));
	memset(&alt_ctx, 0, sizeof(alt_ctx));
}

static char *vxutil_phash_md5(const char *key, const char *salt)
{
	struct crypto_work w = {
		.prefix      = "$1$",
		.key         = key,
		.salt        = salt,
		.buffer_size = 5 * 4 + 2,
	};
	char auto_salt[12];
	void *buffer;

	if (salt == NULL) {
		strcpy(auto_salt, "$1$");
		vxutil_gensalt_base64(&auto_salt[3], 8);
		auto_salt[sizeof(auto_salt)-1] = '\0';
		w.salt = auto_salt;
	}
	if (!vxutil_phash_md5init(&w))
		return false;
	if ((buffer = malloc(w.buffer_size)) != NULL)
		vxutil_phash_md5xfrm(&w, buffer);
	return buffer;
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

static bool vxutil_phash_shainit(struct crypto_work *w)
{
	unsigned int r;
	char *end;

	w->prefix_len = strlen(w->prefix);

	if (strncmp(w->salt, w->prefix, w->prefix_len) != 0)
		return false;

	w->key_len = strlen(w->key);
	w->salt   += w->prefix_len;

	if (strncmp(w->salt, s_rounds, sizeof(s_rounds) - 1) == 0) {
		r = strtoul(w->salt + sizeof(s_rounds) - 1, &end, 10);
		if (*end == '$') {
			w->salt = end + 1;
			if (r < 1000)
				w->rounds = 1000;
			else if (r > 999999999)
				w->rounds = 999999999;
			else
				w->rounds = r;
			w->rounds_sel   = true;
			w->buffer_size += end - w->salt + 1;
		}
	}

	w->salt_len = strcspn(w->salt, "$");
	if (w->salt_len > 16)
		w->salt_len = 16;

	/* +1 for '$', +1 for final '\0' */
	w->buffer_size += w->prefix_len + w->salt_len + 2;
	return true;
}

static void vxutil_phash_sha256xfrm(const struct crypto_work *w, char *hash)
{
	unsigned char alt_result[SHA256_DIGEST_LENGTH];
	unsigned char temp_result[SHA256_DIGEST_LENGTH];
	char buf[10+sizeof(s_rounds)], *p;
	SHA256_CTX ctx, alt_ctx;
	int i;

	/* Step 1-3 */
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, w->key, w->key_len);
	SHA256_Update(&ctx, w->salt, w->salt_len);

	/* Step 4-8 */
	SHA256_Init(&alt_ctx);
	SHA256_Update(&alt_ctx, w->key, w->key_len);
	SHA256_Update(&alt_ctx, w->salt, w->salt_len);
	SHA256_Update(&alt_ctx, w->key, w->key_len);
	SHA256_Final(alt_result, &alt_ctx);

	/* Step 9-10 */
	for (i = w->key_len; i > sizeof(alt_result); i -= sizeof(alt_result))
		SHA256_Update(&ctx, alt_result, sizeof(alt_result));
	SHA256_Update(&ctx, alt_result, i);

	/* Step 11-12 */
	for (i = w->key_len; i > 0; i >>= 1)
		if (i & 1)
			SHA256_Update(&ctx, alt_result, sizeof(alt_result));
		else
			SHA256_Update(&ctx, w->key, w->key_len);

	SHA256_Final(alt_result, &ctx);

	/* Step 13-15 */
	SHA256_Init(&alt_ctx);
	for (i = 0; i < w->key_len; ++i)
		SHA256_Update(&alt_ctx, w->key, w->key_len);
	SHA256_Final(temp_result, &alt_ctx);

	/* Step 16 */
	p = w->p;
	for (i = w->key_len; i >= sizeof(temp_result);
	    i -= sizeof(temp_result), p += sizeof(temp_result))
		memcpy(p, temp_result, sizeof(temp_result));
	memcpy(p, temp_result, i);

	/* Step 17-19 */
	SHA256_Init(&alt_ctx);
	for (i = 0; i < 16 + alt_result[0]; ++i)
		SHA256_Update(&alt_ctx, w->salt, w->salt_len);
	SHA256_Final(temp_result, &alt_ctx);

	/* Step 20 */
	p = w->s;
	for (i = w->salt_len; i >= sizeof(temp_result);
	    i -= sizeof(temp_result), p += sizeof(temp_result))
		memcpy(p, temp_result, sizeof(temp_result));
	memcpy(p, temp_result, i);

	/* Step 21 */
	for (i = 0; i < w->rounds; ++i) {
		SHA256_Init(&ctx);
		if (i & 1)
			SHA256_Update(&ctx, w->p, w->key_len);
		else
			SHA256_Update(&ctx, alt_result, sizeof(alt_result));
		if (i % 3 != 0)
			SHA256_Update(&ctx, w->s, w->salt_len);
		if (i % 7 != 0)
			SHA256_Update(&ctx, w->p, w->key_len);
		if (i & 1)
			SHA256_Update(&ctx, alt_result, sizeof(alt_result));
		else
			SHA256_Update(&ctx, w->p, w->key_len);
		SHA256_Final(alt_result, &ctx);
	}

	strcpy(hash, w->prefix);
	if (w->rounds_sel) {
		snprintf(buf, sizeof(buf), "rounds=%u$", w->rounds);
		strcat(hash, buf);
	}
	strncat(hash, w->salt, w->salt_len);
	strcat(hash, "$");

#define b64(a, b, c, i) p = vxutil_phash_base64(p, (i), (a), (b), (c))
	p = hash + strlen(hash);
	b64(alt_result[0], alt_result[10], alt_result[20], 4);
	b64(alt_result[21], alt_result[1], alt_result[11], 4);
	b64(alt_result[12], alt_result[22], alt_result[2], 4);
	b64(alt_result[3], alt_result[13], alt_result[23], 4);
	b64(alt_result[24], alt_result[4], alt_result[14], 4);
	b64(alt_result[15], alt_result[25], alt_result[5], 4);
	b64(alt_result[6], alt_result[16], alt_result[26], 4);
	b64(alt_result[27], alt_result[7], alt_result[17], 4);
	b64(alt_result[18], alt_result[28], alt_result[8], 4);
	b64(alt_result[9], alt_result[19], alt_result[29], 4);
	b64(0, alt_result[31], alt_result[30], 3);
	*p = '\0';
#undef b64
	memset(alt_result, 0, sizeof(alt_result));
	memset(temp_result, 0, sizeof(temp_result));
	memset(&ctx, 0, sizeof(ctx));
	memset(&alt_ctx, 0, sizeof(alt_ctx));
}

static char *vxutil_phash_sha256(const char *key, const char *salt)
{
	struct crypto_work w = {
		.prefix      = "$5$",
		.key         = key,
		.salt        = salt,
		.buffer_size = 10 * 4 + 3,
	};
	char auto_salt[20];
	void *buffer;

	if (salt == NULL) {
		strcpy(auto_salt, "$5$");
		vxutil_gensalt_base64(&auto_salt[3], 16);
		auto_salt[sizeof(auto_salt)-1] = '\0';
		w.salt = auto_salt;
	}
	w.rounds = 5000;
	if (!vxutil_phash_shainit(&w))
		return false;
	if ((buffer = malloc(w.buffer_size)) == NULL)
		goto out;
	if ((w.p = malloc(w.key_len)) == NULL)
		goto out;
	if ((w.s = malloc(w.salt_len)) == NULL)
		goto out;
	vxutil_phash_sha256xfrm(&w, buffer);
 out:
	free(w.p);
	free(w.s);
	return buffer;
}

static void vxutil_phash_sha512xfrm(const struct crypto_work *w, char *hash)
{
	unsigned char alt_result[SHA512_DIGEST_LENGTH];
	unsigned char temp_result[SHA512_DIGEST_LENGTH];
	char buf[10+sizeof(s_rounds)], *p;
	SHA512_CTX ctx, alt_ctx;
	int i;

	/* Step 1-3 */
	SHA512_Init(&ctx);
	SHA512_Update(&ctx, w->key, w->key_len);
	SHA512_Update(&ctx, w->salt, w->salt_len);

	/* Step 4-8 */
	SHA512_Init(&alt_ctx);
	SHA512_Update(&alt_ctx, w->key, w->key_len);
	SHA512_Update(&alt_ctx, w->salt, w->salt_len);
	SHA512_Update(&alt_ctx, w->key, w->key_len);
	SHA512_Final(alt_result, &alt_ctx);

	/* Step 9-10 */
	for (i = w->key_len; i > sizeof(alt_result); i -= sizeof(alt_result))
		SHA512_Update(&ctx, alt_result, sizeof(alt_result));
	SHA512_Update(&ctx, alt_result, i);

	/* Step 11-12 */
	for (i = w->key_len; i > 0; i >>= 1)
		if (i & 1)
			SHA512_Update(&ctx, alt_result, sizeof(alt_result));
		else
			SHA512_Update(&ctx, w->key, w->key_len);

	SHA512_Final(alt_result, &ctx);

	/* Step 13-15 */
	SHA512_Init(&alt_ctx);
	for (i = 0; i < w->key_len; ++i)
		SHA512_Update(&alt_ctx, w->key, w->key_len);
	SHA512_Final(temp_result, &alt_ctx);

	/* Step 16 */
	p = w->p;
	for (i = w->key_len; i >= sizeof(temp_result);
	    i -= sizeof(temp_result), p += sizeof(temp_result))
		memcpy(p, temp_result, sizeof(temp_result));
	memcpy(p, temp_result, i);

	/* Step 17-19 */
	SHA512_Init(&alt_ctx);
	for (i = 0; i < 16 + alt_result[0]; ++i)
		SHA512_Update(&alt_ctx, w->salt, w->salt_len);
	SHA512_Final(temp_result, &alt_ctx);

	/* Step 20 */
	p = w->s;
	for (i = w->salt_len; i >= sizeof(temp_result);
	    i -= sizeof(temp_result), p += sizeof(temp_result))
		memcpy(p, temp_result, sizeof(temp_result));
	memcpy(p, temp_result, i);

	/* Step 21 */
	for (i = 0; i < w->rounds; ++i) {
		SHA512_Init(&ctx);
		if (i & 1)
			SHA512_Update(&ctx, w->p, w->key_len);
		else
			SHA512_Update(&ctx, alt_result, sizeof(alt_result));
		if (i % 3 != 0)
			SHA512_Update(&ctx, w->s, w->salt_len);
		if (i % 7 != 0)
			SHA512_Update(&ctx, w->p, w->key_len);
		if (i & 1)
			SHA512_Update(&ctx, alt_result, sizeof(alt_result));
		else
			SHA512_Update(&ctx, w->p, w->key_len);
		SHA512_Final(alt_result, &ctx);
	}

	strcpy(hash, w->prefix);
	if (w->rounds_sel) {
		snprintf(buf, sizeof(buf), "rounds=%u$", w->rounds);
		strcat(hash, buf);
	}
	strncat(hash, w->salt, w->salt_len);
	strcat(hash, "$");

#define b64(a, b, c, i) p = vxutil_phash_base64(p, (i), (a), (b), (c))
	p = hash + strlen(hash);
	b64(alt_result[0], alt_result[21], alt_result[42], 4);
	b64(alt_result[22], alt_result[43], alt_result[1], 4);
	b64(alt_result[44], alt_result[2], alt_result[23], 4);
	b64(alt_result[3], alt_result[24], alt_result[45], 4);
	b64(alt_result[25], alt_result[46], alt_result[4], 4);
	b64(alt_result[47], alt_result[5], alt_result[26], 4);
	b64(alt_result[6], alt_result[27], alt_result[48], 4);
	b64(alt_result[28], alt_result[49], alt_result[7], 4);
	b64(alt_result[50], alt_result[8], alt_result[29], 4);
	b64(alt_result[9], alt_result[30], alt_result[51], 4);
	b64(alt_result[31], alt_result[52], alt_result[10], 4);
	b64(alt_result[53], alt_result[11], alt_result[32], 4);
	b64(alt_result[12], alt_result[33], alt_result[54], 4);
	b64(alt_result[34], alt_result[55], alt_result[13], 4);
	b64(alt_result[56], alt_result[14], alt_result[35], 4);
	b64(alt_result[15], alt_result[36], alt_result[57], 4);
	b64(alt_result[37], alt_result[58], alt_result[16], 4);
	b64(alt_result[59], alt_result[17], alt_result[38], 4);
	b64(alt_result[18], alt_result[39], alt_result[60], 4);
	b64(alt_result[40], alt_result[61], alt_result[19], 4);
	b64(alt_result[62], alt_result[20], alt_result[41], 4);
	b64(0, 0, alt_result[63], 2);
	*p = '\0';
#undef b64
	memset(alt_result, 0, sizeof(alt_result));
	memset(temp_result, 0, sizeof(temp_result));
	memset(&ctx, 0, sizeof(ctx));
	memset(&alt_ctx, 0, sizeof(alt_ctx));
}

static char *vxutil_phash_sha512(const char *key, const char *salt)
{
	struct crypto_work w = {
		.prefix      = "$6$",
		.key         = key,
		.salt        = salt,
		.buffer_size = 21 * 4 + 2,
	};
	char auto_salt[20];
	void *buffer;

	if (salt == NULL) {
		strcpy(auto_salt, "$6$");
		vxutil_gensalt_base64(&auto_salt[3], 16);
		auto_salt[sizeof(auto_salt)-1] = '\0';
		w.salt = auto_salt;
	}
	w.rounds = 5000;
	if (!vxutil_phash_shainit(&w))
		return false;
	if ((buffer = malloc(w.buffer_size)) == NULL)
		goto out;
	if ((w.p = malloc(w.key_len)) == NULL)
		goto out;
	if ((w.s = malloc(w.salt_len)) == NULL)
		goto out;
	vxutil_phash_sha512xfrm(&w, buffer);
 out:
	free(w.p);
	free(w.s);
	return buffer;
}

static char *(*const hashfn[])(const char *, const char *) = {
	[VXPHASH_DES]      = vxutil_phash_des,
	[VXPHASH_MD5]      = vxutil_phash_md5,
	[VXPHASH_BLOWFISH] = vxutil_phash_blowfish,
	[VXPHASH_SMBNT]    = vxutil_phash_smbnt,
	[VXPHASH_SHA256]   = vxutil_phash_sha256,
	[VXPHASH_SHA512]   = vxutil_phash_sha512,
};

EXPORT_SYMBOL bool vxutil_phash(const char *key, const char *salt,
    unsigned int meth, char **result)
{
	if (meth >= ARRAY_SIZE(hashfn) || hashfn[meth] == NULL)
		return false;
	*result = hashfn[meth](key, salt);
	return *result != NULL;
}
