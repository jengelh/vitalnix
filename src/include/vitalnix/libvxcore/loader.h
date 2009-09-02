#ifndef _VITALNIX_LIBVXCORE_LOADER_H
#define _VITALNIX_LIBVXCORE_LOADER_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct HXmap;

/*
 *	LOADER.C
 */
extern int vxcore_module_register(const char *, const char *, const void *);
extern void vxcore_module_unregister(const char *, const char *);
extern struct HXmap *vxcore_section_lookup(const char *);
extern void *vxcore_module_lookup(const char *, const char *);
extern const void *vxcore_section_trav(void **, const char *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXCORE_LOADER_H */
