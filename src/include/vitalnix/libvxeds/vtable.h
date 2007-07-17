#ifndef _VITALNIX_LIBVXEDS_VTABLE_H
#define _VITALNIX_LIBVXEDS_VTABLE_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct vxeds_entry;
struct edsformat_vtable {
	const char *desc, *ext;
	int (*open)(const char *, void **);
	int (*read)(void *, struct vxeds_entry *);
	void (*close)(void *);
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXEDS_VTABLE_H */
