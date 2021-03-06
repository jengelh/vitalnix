#ifndef VITALNIX_DRIVERS_SHADOW_SHADOW_H
#define VITALNIX_DRIVERS_SHADOW_SHADOW_H 1

#include <stdbool.h>
#include <stdio.h>

/*
 *	Definitions
 */
#define TOUCH_GROUP_TAG(i) (state->fgroup.is_chg = (i))
#define TOUCH_USER_TAG(i)  \
	(state->fpasswd.is_chg = state->fshadow.is_chg = \
	state->fvxshadow.is_chg = (i))

enum {
	CONFIG_READ  = 0,
	CONFIG_FREE,
};

struct HXdeque;
struct vxdb_group;
struct vxdb_user;

struct spdb_file {
	char *path;
	FILE *fp;
	int fd;
	bool is_chg;
};

struct shadow_state {
	unsigned long flags;
	struct HXdeque *dq_user, *dq_group;
	struct spdb_file fpasswd, fshadow, fvxpasswd, fvxshadow, fgroup;
	unsigned int uid_min, uid_max, gid_min, gid_max;
};

/*
 *	AUX.C
 */
extern unsigned int automatic_uid(struct shadow_state *, unsigned int);
extern unsigned int automatic_gid(struct shadow_state *, unsigned int);
extern void free_data(struct shadow_state *);
extern void free_single_group(struct vxdb_group *);
extern void free_single_user(struct vxdb_user *);
extern struct vxdb_group *lookup_group(const struct HXdeque *, const char *, unsigned int);
extern struct vxdb_user *lookup_user(const struct HXdeque *, const char *, unsigned int);
extern void read_config(struct shadow_state *, unsigned int, const char *);
extern struct HXdeque_node *skip_nis_users(struct HXdeque_node *);
extern struct HXdeque_node *skip_nis_groups(struct HXdeque_node *);
extern void truncate_here(FILE *);

/*
 *	DBOPS.C
 */
extern int db_open(struct shadow_state *, unsigned int);
extern void db_close(struct shadow_state *);
extern void db_flush(struct shadow_state *, bool);

/*
 *	FSGROUP.C
 */
extern struct HXdeque *db_read_groups(FILE *);
extern void db_flush_groups(struct shadow_state *);

/*
 *	FSPASSWD.C
 */
extern struct HXdeque *db_read_passwd(FILE *);
extern void db_write_passwd(FILE *, const struct vxdb_user *);

/*
 *	FSSHADOW.C
 */
extern void db_read_shadow(FILE *, struct HXdeque *);
extern void db_write_shadow(FILE *, const struct vxdb_user *);

/*
 *	FSVXSHADOW.C
 */
extern void db_read_vxshadow(const char *, struct HXdeque *);
extern void db_write_vxshadow(FILE *, const struct vxdb_user *);

#endif /* VITALNIX_DRIVERS_SHADOW_SHADOW_H */
