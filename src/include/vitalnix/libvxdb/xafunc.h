#ifndef _VITALNIX_LIBVXDB_XAFUNC_H
#define _VITALNIX_LIBVXDB_XAFUNC_H 1

#include <sys/types.h>
#include <vitalnix/libvxdb/libvxdb.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	INLINE FUNCTIONS
 */
static inline int vxdb_open(struct vxdb_state *state, unsigned int flags)
{
	return state->vtable->open(state, flags);
}

static inline int vxdb_lock(struct vxdb_state *state)
{
	return state->vtable->lock(state);
}

static inline int vxdb_unlock(struct vxdb_state *state)
{
	return state->vtable->unlock(state);
}

static inline void vxdb_close(struct vxdb_state *state)
{
	state->vtable->close(state);
	return;
}

#define vxdb_modctl(state, command, args...) \
	((state)->vtable->modctl(state, command, ## args))

static inline int vxdb_useradd(struct vxdb_state *state,
    const struct vxdb_user *user)
{
	return state->vtable->useradd(state, user);
}

static inline int vxdb_usermod(struct vxdb_state *state, const char *name,
    const struct vxdb_user *newstuff)
{
	return state->vtable->usermod(state, name, newstuff);
}

static inline int vxdb_userdel(struct vxdb_state *state, const char *name)
{
	return state->vtable->userdel(state, name);
}

static inline void *vxdb_usertrav_init(struct vxdb_state *state)
{
	return state->vtable->usertrav_init(state);
}

static inline int vxdb_usertrav_walk(struct vxdb_state *state, void *data,
    struct vxdb_user *result)
{
	return state->vtable->usertrav_walk(state, data, result);
}

static inline void vxdb_usertrav_free(struct vxdb_state *state, void *data)
{
	return state->vtable->usertrav_free(state, data);
}

static inline int vxdb_groupadd(struct vxdb_state *state,
    const struct vxdb_group *group)
{
	return state->vtable->groupadd(state, group);
}

static inline int vxdb_groupmod(struct vxdb_state *state, const char *name,
    const struct vxdb_group *newstuff)
{
	return state->vtable->groupmod(state, name, newstuff);
}

static inline int vxdb_groupdel(struct vxdb_state *state, const char *name)
{
	return state->vtable->groupdel(state, name);
}

static inline void *vxdb_grouptrav_init(struct vxdb_state *state)
{
	return state->vtable->grouptrav_init(state);
}

static inline int vxdb_grouptrav_walk(struct vxdb_state *state, void *data,
    struct vxdb_group *result)
{
	return state->vtable->grouptrav_walk(state, data, result);
}

static inline void vxdb_grouptrav_free(struct vxdb_state *state, void *data)
{
	return state->vtable->grouptrav_free(state, data);
}

static inline int vxdb_getpwnam(struct vxdb_state *state, const char *user,
    struct vxdb_user *result)
{
	return state->vtable->getpwnam(state, user, result);
}

static inline int vxdb_getpwuid(struct vxdb_state *state, unsigned int uid,
    struct vxdb_user *result)
{
	return state->vtable->getpwuid(state, uid, result);
}

static inline int vxdb_getgrnam(struct vxdb_state *state, const char *group,
    struct vxdb_group *result)
{
	return state->vtable->getgrnam(state, group, result);
}

static inline int vxdb_getgrgid(struct vxdb_state *state, unsigned int gid,
    struct vxdb_group *result)
{
	return state->vtable->getgrgid(state, gid, result);
}

static inline int vxdb_sgmapadd(struct vxdb_state *state, const char *user,
    const char *group)
{
	return state->vtable->sgmapadd(state, user, group);
}

static inline int vxdb_sgmapget(struct vxdb_state *state, const char *user,
    char ***result)
{
	return state->vtable->sgmapget(state, user, result);
}

static inline int vxdb_sgmapdel(struct vxdb_state *state, const char *user,
    const char *group)
{
	return state->vtable->sgmapdel(state, user, group);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXDB_XAFUNC_H */
