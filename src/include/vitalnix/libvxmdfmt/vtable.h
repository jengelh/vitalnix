#ifndef _VITALNIX_LIBVXMDFMT_VTABLE_H
#define _VITALNIX_LIBVXMDFMT_VTABLE_H 1

#ifdef __cplusplus
extern "C" {
#endif

struct pwl_data;
struct pwlfmt_workspace;

struct pwlstyle_vtable {
	const char *name, *desc;
	int require_template;
	int (*init)(struct pwlfmt_workspace *);
	void (*exit)(struct pwlfmt_workspace *);

	void (*file_header)(const struct pwlfmt_workspace *);
	void (*tbl_header)(const struct pwlfmt_workspace *, const struct pwl_data *);
	void (*tbl_entry)(const struct pwlfmt_workspace *, const struct pwl_data *);
	void (*tbl_footer)(const struct pwlfmt_workspace *, const struct pwl_data *);
	void (*file_footer)(const struct pwlfmt_workspace *);
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXMDFMT_VTABLE_H */
