#ifndef _VITALNIX_LIBVXMDFMT_LIBVXMDFMT_H
#define _VITALNIX_LIBVXMDFMT_LIBVXMDFMT_H 1

#ifndef __cplusplus
#    include <stdio.h>
#else
#    include <cstdio>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PWLFMT_ESUCCESS = 1500,
    PWLFMT_ENOSTYLE,
    PWLFMT_EREQTPL,
    PWLFMT_EEINPUT,
    PWLFMT_EEOUTPUT,
    PWLFMT_EINPUTVER,
    PWLFMT_ENOUSERS,
};

struct pwlstyle_vtable;

struct pwlfmt_workspace {
    // Input (public)
    char *style_name, *input_file, *output_file, *template_file;
    void (*report)(const struct pwlfmt_workspace *, int, int);

    // Workspace (private)
    const struct pwlstyle_vtable *vtable;
    FILE *input_fh, *output_fh;
    void *style_data, *template_data;
    long num_entries;
};

/*
 *      PWLFMT.C
 */
extern int pwlfmt_new(struct pwlfmt_workspace *);
extern int pwlfmt_process(struct pwlfmt_workspace *);
extern const char *pwlfmt_strerror(int);
extern const struct pwlstyle_vtable *pwlstyles_trav(void **);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXMDFMT_LIBVXMDFMT_H
