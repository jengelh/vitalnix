#ifndef _VITALNIX_LIBVXCGI_LIBVXCGI_H
#define _VITALNIX_LIBVXCGI_LIBVXCGI_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct HXbtree;

/*
 *	AUTH.C
 */
extern int vxcgi_authenticate(const char *, const char *, const char *);
extern int vxcgi_authenticate_ext(const char *, const char *, const char *);

/*
 *	CGI.C
 */
extern char *vxcgi_read_data(int, const char **);
extern struct HXbtree *vxcgi_split(char *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXCGI_LIBVXCGI_H */
