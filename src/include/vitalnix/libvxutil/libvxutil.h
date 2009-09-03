#ifndef _VITALNIX_LIBVXUTIL_LIBVXUTIL_H
#define _VITALNIX_LIBVXUTIL_LIBVXUTIL_H 1

#include <sys/types.h>
#ifndef __cplusplus
#	include <stdio.h>
#	include <stdlib.h>
#	include <stdbool.h>
#else
#	include <cstdio>
#	include <cstdlib>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct HXformat_map;

/*
 *	PHASH.C
 */
enum {
	VXPHASH_DES = 1,
	VXPHASH_MD5,
	VXPHASH_BLOWFISH,
	VXPHASH_SMBNT,
	VXPHASH_SHA256,
	VXPHASH_SHA512,
};

extern bool vxutil_phash(const char *, const char *, unsigned int, char **);

/*
 *	GENPW.C
 */
enum {
	GENPW_1DIGIT  = 1 << 0,
	GENPW_1CASE   = 1 << 1,
	GENPW_O1DIGIT = 1 << 2,
	GENPW_O1CASE  = 1 << 3,
	GENPW_RAND    = 1 << 4,
	GENPW_JP      = 1 << 5,
	GENPW_ZH      = 1 << 6,
};

extern void vxutil_genpw(char *, int, unsigned int);

/*
 *	UTIL.C
 */
enum {
	VXQUOTE_SINGLE,
	VXQUOTE_DOUBLE,
	VXQUOTE_XML,
	VXQUOTE_BASE64,
	_VXQUOTE_MAX,
};

extern unsigned int vxutil_now_iday(void);
extern bool vxutil_only_digits(const char *);
extern char *vxutil_propose_home(char *, size_t, const char *, const char *,
	unsigned int);
extern char *vxutil_propose_lname(char *, size_t, const char *, const char *);
extern char *vxutil_quote(const char *, unsigned int, char **);
extern int vxutil_replace_run(const char *, const struct HXformat_map *);
extern char *vxutil_slurp_file(const char *);
extern int vxutil_string_iday(const char *);
extern int vxutil_string_xday(const char *);
extern bool vxutil_valid_username(const char *);

/*
 *	UUID.C
 */
extern char *vxuuid_vx3(const char *, unsigned int);
extern unsigned int vxuuid_vx3_get_xday(const char *);

/*
 *	INLINE FUNCTIONS
 */
static inline const char *vxutil_azstr(const char *ptr)
{
	return (ptr != NULL) ? ptr : "";
}

static inline bool vxutil_have_display(void)
{
	const char *p;
	return (p = getenv("DISPLAY")) != NULL && *p != '\0';
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXUTIL_LIBVXUTIL_H */
