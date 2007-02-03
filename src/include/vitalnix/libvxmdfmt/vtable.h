#ifndef _VITALNIX_LIBVXMDFMT_VTABLE_H
#define _VITALNIX_LIBVXMDFMT_VTABLE_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct pwl_data;
struct pwlfmt_workspace;
typedef void (printfunc_t)(struct pwlfmt_workspace *, const struct pwl_data *);

struct pwlstyle_vtable {
    const char *name, *desc;
    int require_template;
    int (*init)(struct pwlfmt_workspace *);
    void (*exit)(struct pwlfmt_workspace *);
    printfunc_t *file_header, *tbl_header, *tbl_entry,
                *tbl_footer, *file_footer;
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXMDFMT_VTABLE_H
