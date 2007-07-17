#ifndef _VITALNIX_LIBVXPDB_XWFUNC_H
#define _VITALNIX_LIBVXPDB_XWFUNC_H 1

#include <sys/types.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	INLINE FUNCTIONS
 */
static inline int vxpdb_getpwnam(struct vxpdb_state *state, const char *user,
    struct vxpdb_user *result)
{
	struct vxpdb_user mask;
	vxpdb_user_clean(&mask);
	mask.pw_name = const_cast(char *, user);
	return vxpdb_userinfo(state, &mask, result, result != NULL);
}

static inline int vxpdb_getpwuid(struct vxpdb_state *state, long uid,
    struct vxpdb_user *result)
{
	struct vxpdb_user mask;
	vxpdb_user_clean(&mask);
	mask.pw_uid = uid;
	return vxpdb_userinfo(state, &mask, result, result != NULL);
}

static inline int vxpdb_getgrnam(struct vxpdb_state *state, const char *group,
    struct vxpdb_group *result)
{
	struct vxpdb_group mask;
	mask.gr_name = const_cast(char *, group);
	mask.gr_gid  = PDB_NOGID;
	return vxpdb_groupinfo(state, &mask, result, result != NULL);
}

static inline int vxpdb_getgrgid(struct vxpdb_state *state, long gid,
    struct vxpdb_group *result)
{
	struct vxpdb_group mask;
	mask.gr_name = NULL;
	mask.gr_gid  = gid;
	return vxpdb_groupinfo(state, &mask, result, result != NULL);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXPDB_XWFUNC_H */
