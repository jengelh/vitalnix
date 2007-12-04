#ifndef _VITALNIX_LIBVXDB_LIBVXDB_H
#define _VITALNIX_LIBVXDB_LIBVXDB_H 1

#include <sys/types.h>
#ifndef __cplusplus
#	include <stdbool.h>
#	include <stdlib.h>
#else
#	include <cstdlib>
#endif
#include <libHX.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions */
enum {
	VXDB_NO_CHANGE   = -2,
	VXDB_AUTOUID     = -1,
	VXDB_AUTOUID_SYS = -2,
	VXDB_AUTOGID     = -1,
	VXDB_AUTOGID_SYS = -2,
	VXDB_NOUID       = -1,
	VXDB_NOGID       = -1,
	VXDB_NO_EXPIRE   = -1,
	VXDB_NO_INACTIVE = -1,
	VXDB_DFL_KEEPMIN = 0,
	VXDB_DFL_KEEPMAX = 10000,
	VXDB_DFL_WARNAGE = 21,

	/* Flags for vxdb_open() */
	VXDB_SYNC   = 1 << 0,
	VXDB_WRLOCK = 1 << 1,

	/* vxdb_modctl() commands */
	VXDB_FLUSH,
	VXDB_COUNT_USERS,
	VXDB_COUNT_GROUPS,
	VXDB_NEXTUID_SYS,
	VXDB_NEXTUID,
	VXDB_NEXTGID_SYS,
	VXDB_NEXTGID,
};

struct vxconfig_useradd;
struct vxconfig_usermod;
struct vxconfig_userdel;

/*
 * When vxdb functions return something, the string
 * fields must be handled like "hmc_t"s.
 */
/*
 * @vs_defer: day on which tagging occurred, not when deletion is due
 */
struct vxdb_user {
	/* passwd part */
	char *pw_name, *pw_real, *pw_home, *pw_shell, *pw_igrp;
	unsigned int pw_uid, pw_gid;
	struct HXdeque *pw_sgrp;

	/* shadow part */
	char *sp_passwd, *sp_ntpasswd;
	long sp_lastchg, sp_min, sp_max, sp_warn, sp_expire, sp_inact;

	/* vxshadow part (vs_), used by libvxmdsync */
	char *vs_uuid, *vs_pvgrp;
	unsigned int vs_defer;

	/* backend-specific private stuff */
	void *be_priv;
};

struct vxdb_group {
	char *gr_name;
	unsigned int gr_gid;
	void *be_priv;
};

struct vxdb_driver;
struct vxdb_state {
	void *handle, *state;
	const struct vxdb_driver *vtable;
};

struct vxdb_driver {
	const char *name, *desc;

	int (*init)(struct vxdb_state *, const char *);
	int (*open)(struct vxdb_state *, unsigned int);
	void (*close)(struct vxdb_state *);
	void (*exit)(struct vxdb_state *);

	long (*modctl)(struct vxdb_state *, unsigned int, ...);
	int (*lock)(struct vxdb_state *);
	int (*unlock)(struct vxdb_state *);

	int (*useradd)(struct vxdb_state *, const struct vxdb_user *);
	int (*usermod)(struct vxdb_state *, const char *, const struct vxdb_user *);
	int (*userdel)(struct vxdb_state *, const char *);
	int (*getpwuid)(struct vxdb_state *, unsigned int, struct vxdb_user *);
	int (*getpwnam)(struct vxdb_state *, const char *, struct vxdb_user *);
	void *(*usertrav_init)(struct vxdb_state *);
	int (*usertrav_walk)(struct vxdb_state *, void *, struct vxdb_user *);
	void (*usertrav_free)(struct vxdb_state *, void *);

	int (*groupadd)(struct vxdb_state *, const struct vxdb_group *);
	int (*groupmod)(struct vxdb_state *, const char *, const struct vxdb_group *);
	int (*groupdel)(struct vxdb_state *, const char *);
	int (*getgrgid)(struct vxdb_state *, unsigned int, struct vxdb_group *);
	int (*getgrnam)(struct vxdb_state *, const char *, struct vxdb_group *);
	void *(*grouptrav_init)(struct vxdb_state *);
	int (*grouptrav_walk)(struct vxdb_state *, void *, struct vxdb_group *);
	void (*grouptrav_free)(struct vxdb_state *, void *);

	int (*sgmapadd)(struct vxdb_state *, const char *, const char *);
	int (*sgmapget)(struct vxdb_state *, const char *, char ***);
	int (*sgmapdel)(struct vxdb_state *, const char *, const char *);
};

/*
 *	AUX.C
 */
extern void *vxdb_user_alloc(struct vxdb_user *, size_t);
extern void vxdb_user_clean(struct vxdb_user *);
extern void vxdb_user_copy(struct vxdb_user *, const struct vxdb_user *);
extern struct vxdb_user *vxdb_user_dup(const struct vxdb_user *);
extern void vxdb_user_free(struct vxdb_user *, bool);
extern void vxdb_user_nomodify(struct vxdb_user *);

extern void *vxdb_group_alloc(struct vxdb_group *, size_t);
extern void vxdb_group_clean(struct vxdb_group *);
extern void vxdb_group_copy(struct vxdb_group *, const struct vxdb_group *);
extern struct vxdb_group *vxdb_group_dup(const struct vxdb_group *);
extern void vxdb_group_free(struct vxdb_group *, bool);
extern void vxdb_group_nomodify(struct vxdb_group *);

/*
 *	DUMMY.C
 */
extern void vxdb_fix_vtable(struct vxdb_driver *);

/*
 *	LOADER.C
 */
extern struct vxdb_state *vxdb_load(const char *);
extern void vxdb_unload(struct vxdb_state *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXDB_LIBVXDB_H */
