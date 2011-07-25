/*=============================================================================
Vitalnix User Management Suite
libvxutil/blowfish.h - header file for blowfish.c
=============================================================================*/
#ifndef _VITALNIX_LIBVXUTIL_BLOWFISH_H
#define _VITALNIX_LIBVXUTIL_BLOWFISH_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	blowfish.c
 */
extern char *_crypt_blowfish_rn(const char *, const char *, char *, int);
extern char *_crypt_gensalt_blowfish_rn(const char *, unsigned long,
	const char *, int, char *, int);
extern int _crypt_output_magic(const char *, char *, int);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXUTIL_BLOWFISH_H */
