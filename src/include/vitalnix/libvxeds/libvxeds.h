#ifndef _VITALNIX_LIBVXEDS_LIBVXEDS_H
#define _VITALNIX_LIBVXEDS_LIBVXEDS_H 1

#ifdef __cplusplus
extern "C" {
#endif

// Definitions
struct edsformat_vtable;
struct vxeds_entry {
    char *username, *first_name, *surname, *full_name, *pvgrp, *uuid;
};

/*
 *      EDS.C
 */
extern int vxeds_open(const char *, const char *, void **);
extern int vxeds_read(void *, struct vxeds_entry *);
extern void vxeds_close(void *);
extern const char *vxeds_derivefromname(const char *);
extern void vxeds_free_entry(struct vxeds_entry *);
extern const struct edsformat_vtable *vxeds_formats_trav(void **);

/*
 *      INTERNAL USE
 */
extern char *vxeds_bfullname(const char *, const char *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXEDS_LIBVXEDS_H
