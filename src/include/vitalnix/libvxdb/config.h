#ifndef _VITALNIX_LIBVXPDB_CONFIG_H
#define _VITALNIX_LIBVXPDB_CONFIG_H 1

#include <vitalnix/libvxdb/libvxpdb.h>

#ifdef __cplusplus
extern "C" {
#endif

struct vxconfig_useradd {
	char *master_preadd, *master_postadd, *user_preadd, *user_postadd;
	char *default_group, *home_base, *skel_dir;
	unsigned int create_home, split_level, umask;
	struct vxpdb_user defaults;
};

struct vxconfig_usermod {
	char *master_premod, *master_postmod, *user_premod, *user_postmod;
};

struct vxconfig_userdel {
	char *master_predel, *master_postdel, *user_predel, *user_postdel;
};

/*
 *	CONFIG.C
 */
extern int vxconfig_read_useradd(const char *, struct vxconfig_useradd *);
extern int vxconfig_read_usermod(const char *, struct vxconfig_usermod *);
extern int vxconfig_read_userdel(const char *, struct vxconfig_userdel *);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _VITALNIX_LIBVXPDB_CONFIG_H */
