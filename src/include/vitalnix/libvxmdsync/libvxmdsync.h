#ifndef _VITALNIX_LIBVXMDSYNC_LIBVXMDSYNC_H
#define _VITALNIX_LIBVXMDSYNC_LIBVXMDSYNC_H 1

#ifndef __cplusplus
#	include <stdio.h>
#else
#	include <cstdio>
#endif
#include <libHX/string.h>
#include <vitalnix/libvxdb/config.h>
#include <vitalnix/libvxdb/libvxdb.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HXbtree;
struct HXdeque;

struct mdsync_config {
	int new_pw_length;
	unsigned int genpw_type, phash_type;
	unsigned int postadd_flush; /* used by HXoption, so no bool */
	struct vxconfig_useradd add_opts;
	struct vxconfig_usermod mod_opts;
	struct vxconfig_userdel del_opts;
};

enum {
	MDREP_ADD,
	MDREP_UPDATE,
	MDREP_DSTART,
	MDREP_DSTOP,
	MDREP_DELETE,
	MDREP_DWAIT,
	MDREP_COMPARE,
	MDREP_FIXUP,
};

struct mdsync_workspace {
	void *user_private;
	struct mdsync_config config;
	void (*report)(unsigned int, const struct mdsync_workspace *,
		unsigned int, unsigned int);

	struct vxdb_state *database;
	hxmc_t *output_data;
	unsigned int num_grp;

	/* private to mdsync */
	struct vxdb_group dest_group;
	FILE *logfile;
	struct HXdeque *defer_start, *defer_wait, *defer_stop, *delete_now;
	struct HXbtree *add_req;
	struct HXbtree *lnlist, *update_req;
};

/*
 *	FIXUP.C
 */
extern void mdsync_fixup(struct mdsync_workspace *);

/*
 *	GEN.C
 */
extern struct mdsync_workspace *mdsync_init(void);
extern int mdsync_prepare_group(struct mdsync_workspace *, const char *);
extern int mdsync_open_log(struct mdsync_workspace *, const char *);
extern void mdsync_free(struct mdsync_workspace *);

/*
 *	PROC.C
 */
extern void mdsync_compare(struct mdsync_workspace *);
extern void mdsync_compare_simple(struct mdsync_workspace *);
extern int mdsync_add(struct mdsync_workspace *);
extern int mdsync_mod(struct mdsync_workspace *);
extern int mdsync_del(struct mdsync_workspace *);

/*
 *	READ_FILE.C
 */
extern int mdsync_read_file(struct mdsync_workspace *, const char *,
	const char *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXMDSYNC_LIBVXMDSYNC_H */
