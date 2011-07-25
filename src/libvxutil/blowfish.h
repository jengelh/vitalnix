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
 *      BLOWFISH.H
 */
extern char *_crypt_blowfish_rn(const char *, const char *, char *, int);
extern char *_crypt_gensalt_blowfish_rn(unsigned long, const char *, int,
	char *, int);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXUTIL_BLOWFISH_H */
