#ifndef _VITALNIX_LIBVXPDB_XAFUNC_H
#define _VITALNIX_LIBVXPDB_XAFUNC_H 1

#include <sys/types.h>
#include <vitalnix/libvxdb/libvxpdb.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	INLINE FUNCTIONS
 */
static inline int vxpdb_open(struct vxpdb_state *state, unsigned int flags)
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

static inline int vxpdb_usermod(struct vxpdb_state *state, const char *name,
    const struct vxpdb_user *newstuff)
{
	return state->vtable->usermod(state, name, newstuff);
}

static inline int vxpdb_userdel(struct vxpdb_state *state, const char *name)
{
	return state->vtable->userdel(state, name);
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

static inline int vxpdb_groupmod(struct vxpdb_state *state, const char *name,
    const struct vxpdb_group *newstuff)
{
	return state->vtable->groupmod(state, name, newstuff);
}

static inline int vxpdb_groupdel(struct vxpdb_state *state, const char *name)
{
	return state->vtable->groupdel(state, name);
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
	return state->vtable->getpwnam(state, user, result);
}

static inline int vxpdb_getpwuid(struct vxpdb_state *state, unsigned int uid,
    struct vxpdb_user *result)
{
	return state->vtable->getpwuid(state, uid, result);
}

static inline int vxpdb_getgrnam(struct vxpdb_state *state, const char *group,
    struct vxpdb_group *result)
{
	return state->vtable->getgrnam(state, group, result);
}

static inline int vxpdb_getgrgid(struct vxpdb_state *state, unsigned int gid,
    struct vxpdb_group *result)
{
	return state->vtable->getgrgid(state, gid, result);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXPDB_XAFUNC_H */
