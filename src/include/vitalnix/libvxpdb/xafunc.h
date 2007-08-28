#ifndef _VITALNIX_LIBVXPDB_XAFUNC_H
#define _VITALNIX_LIBVXPDB_XAFUNC_H 1

#include <sys/types.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxpdb/libvxpdb.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	INLINE FUNCTIONS
 */
static inline int vxpdb_open(struct vxpdb_state *state, long flags)
{
	return state->vtable->open(state, flags);
}

static inline int vxpdb_lock(struct vxpdb_state *state)
{
	return state->vtable->lock(state);
}

static inline int vxpdb_unlock(struct vxpdb_state *state)
{
	return state->vtable->unlock(state);
}

static inline void vxpdb_close(struct vxpdb_state *state)
{
	state->vtable->close(state);
	return;
}

#define vxpdb_modctl(state, command, args...) \
	((state)->vtable->modctl(state, command, ## args))

static inline int vxpdb_useradd(struct vxpdb_state *state,
    const struct vxpdb_user *user)
{
	return state->vtable->useradd(state, user);
}

static inline int vxpdb_usermod(struct vxpdb_state *state,
    const struct vxpdb_user *mask, const struct vxpdb_user *repl)
{
	return state->vtable->usermod(state, mask, repl);
}

static inline int vxpdb_userdel(struct vxpdb_state *state,
    const struct vxpdb_user *hint)
{
	return state->vtable->userdel(state, hint);
}

static inline int vxpdb_userinfo(struct vxpdb_state *state,
    const struct vxpdb_user *mask, struct vxpdb_user *result, size_t size)
{
	return state->vtable->userinfo(state, mask, result, size);
}

static inline void *vxpdb_usertrav_init(struct vxpdb_state *state)
{
	return state->vtable->usertrav_init(state);
}

static inline int vxpdb_usertrav_walk(struct vxpdb_state *state, void *data,
    struct vxpdb_user *result)
{
	return state->vtable->usertrav_walk(state, data, result);
}

static inline void vxpdb_usertrav_free(struct vxpdb_state *state, void *data)
{
	return state->vtable->usertrav_free(state, data);
}

static inline int vxpdb_groupadd(struct vxpdb_state *state,
    const struct vxpdb_group *group)
{
	return state->vtable->groupadd(state, group);
}

static inline int vxpdb_groupmod(struct vxpdb_state *state,
    const struct vxpdb_group *mask, const struct vxpdb_group *repl)
{
	return state->vtable->groupmod(state, mask, repl);
}

static inline int vxpdb_groupdel(struct vxpdb_state *state,
    const struct vxpdb_group *hint)
{
	return state->vtable->groupdel(state, hint);
}

static inline int vxpdb_groupinfo(struct vxpdb_state *state,
    const struct vxpdb_group *mask, struct vxpdb_group *result, size_t size)
{
	return state->vtable->groupinfo(state, mask, result, size);
}

static inline void *vxpdb_grouptrav_init(struct vxpdb_state *state)
{
	return state->vtable->grouptrav_init(state);
}

static inline int vxpdb_grouptrav_walk(struct vxpdb_state *state, void *data,
    struct vxpdb_group *result)
{
	return state->vtable->grouptrav_walk(state, data, result);
}

static inline void vxpdb_grouptrav_free(struct vxpdb_state *state, void *data)
{
	return state->vtable->grouptrav_free(state, data);
}

static inline int vxpdb_getpwnam(struct vxpdb_state *state, const char *user,
    struct vxpdb_user *result)
{
	struct vxpdb_user mask;

	if (state->vtable->getpwnam != NULL)
		return state->vtable->getpwnam(state, user, result);

	vxpdb_user_clean(&mask);
	mask.pw_name = const_cast(char *, user);
	return vxpdb_userinfo(state, &mask, result, result != NULL);
}

static inline int vxpdb_getpwuid(struct vxpdb_state *state, long uid,
    struct vxpdb_user *result)
{
	struct vxpdb_user mask;

	if (state->vtable->getpwuid != NULL)
		return state->vtable->getpwuid(state, uid, result);

	vxpdb_user_clean(&mask);
	mask.pw_uid = uid;
	return vxpdb_userinfo(state, &mask, result, result != NULL);
}

static inline int vxpdb_getgrnam(struct vxpdb_state *state, const char *group,
    struct vxpdb_group *result)
{
	struct vxpdb_group mask;

	if (state->vtable->getgrnam != NULL)
		return state->vtable->getgrnam(state, group, result);

	mask.gr_name = const_cast(char *, group);
	mask.gr_gid  = PDB_NOGID;
	return vxpdb_groupinfo(state, &mask, result, result != NULL);
}

static inline int vxpdb_getgrgid(struct vxpdb_state *state, long gid,
    struct vxpdb_group *result)
{
	struct vxpdb_group mask;

	if (state->vtable->getgrgid != NULL)
		return state->vtable->getgrgid(state, gid, result);

	mask.gr_name = NULL;
	mask.gr_gid  = gid;
	return vxpdb_groupinfo(state, &mask, result, result != NULL);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXPDB_XAFUNC_H */
