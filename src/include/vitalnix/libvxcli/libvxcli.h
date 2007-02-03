#ifndef _VITALNIX_LIBVXCLI_LIBVXCLI_H
#define _VITALNIX_LIBVXCLI_LIBVXCLI_H 1

#include <sys/types.h>
#ifdef __cplusplus
#    include <cstdio>
#else
#    include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#    define VXCQ_TABLE_END {NULL, NULL}
#else
#    define VXCQ_TABLE_END {.msg = NULL, .prompt = NULL}
#endif

enum {
    VXCQ_NONE  = 0,
    VXCQ_ABORT = 1 << 0, // allow abort
    VXCQ_EMPTY = 1 << 1, // allow empty
    VXCQ_ZNULL = 1 << 2, // empty answer will put NULL into *ptr
};

struct vxcq_entry {
    const char *msg, *prompt, *defl;
    int type;
    void *ptr, *uptr;
    long flags;
    int (*validate)(const struct vxcq_entry *);
};

extern char *vxcli_query(const char *, const char *, const char *, long,
    char *, size_t);
extern int vxcli_query_v(const struct vxcq_entry *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXCLI_LIBVXCLI_H
