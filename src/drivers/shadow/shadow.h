/*
 *	shadow/shadow.h
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef VITALNIX_DRIVERS_SHADOW_SHADOW_H
#define VITALNIX_DRIVERS_SHADOW_SHADOW_H 1

#ifdef __cplusplus
#    include <cstdio>
#else
#    include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *      Definitions
 */
#define TOUCH_GROUP_TAG(i)      (state->fgroup.is_chg = (i))
#define TOUCH_USER_TAG(i)       \
    (state->fpasswd.is_chg = state->fshadow.is_chg = \
    state->fvxshadow.is_chg = (i))

enum {
    CONFIG_READ  = 0,
    CONFIG_FREE,
};

struct HXdeque;
struct vxpdb_group;
struct vxpdb_user;

struct spdb_file {
    char *path;
    FILE *fp;
    int fd;
    char is_chg;
};

struct shadow_state {
    unsigned long flags;
    struct HXdeque *dq_user, *dq_group;
    struct spdb_file fpasswd, fshadow, fvxpasswd, fvxshadow, fgroup;
    long uid_min, uid_max, gid_min, gid_max;
};

/*
 *      AUX.C
 */
extern long automatic_uid(struct shadow_state *, long);
extern long automatic_gid(struct shadow_state *, long);
extern void free_data(struct shadow_state *);
extern void free_single_group(struct vxpdb_group *);
extern void free_single_user(struct vxpdb_user *);
extern struct vxpdb_group *lookup_group(struct HXdeque *, const char *, long);
extern struct vxpdb_user *lookup_user(struct HXdeque *, const char *, long);
extern void read_config(struct shadow_state *, unsigned int, const char *);
extern struct HXdeque_node *skip_nis_users(struct HXdeque_node *);
extern struct HXdeque_node *skip_nis_groups(struct HXdeque_node *);
extern void truncate_here(FILE *);

/*
 *      DBOPS.C
 */
extern int db_open(struct shadow_state *, int);
extern void db_close(struct shadow_state *);
extern void db_flush(struct shadow_state *, int);

/*
 *      FSGROUP.C
 */
extern struct HXdeque *db_read_groups(FILE *);
extern void db_flush_groups(struct shadow_state *);

/*
 *      FSPASSWD.C
 */
extern struct HXdeque *db_read_passwd(FILE *);
extern void db_write_passwd(FILE *, const struct vxpdb_user *);

/*
 *      FSSHADOW.C
 */
extern void db_read_shadow(FILE *, struct HXdeque *);
extern void db_write_shadow(FILE *, const struct vxpdb_user *);

/*
 *      FSVXSHADOW.C
 */
extern void db_read_vxshadow(const char *, struct HXdeque *);
extern void db_write_vxshadow(FILE *, const struct vxpdb_user *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VITALNIX_DRIVERS_SHADOW_SHADOW_H
