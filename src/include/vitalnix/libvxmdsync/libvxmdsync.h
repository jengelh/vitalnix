/*=============================================================================
Vitalnix User Management Suite
libvxmdsync/libvxmdsync.h
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2007
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#ifndef _VITALNIX_LIBVXMDSYNC_LIBVXMDSYNC_H
#define _VITALNIX_LIBVXMDSYNC_LIBVXMDSYNC_H 1

#ifndef __cplusplus
#    include <stdio.h>
#else
#    include <cstdio>
#endif
#include <vitalnix/libvxpdb/config.h>
#include <vitalnix/libvxpdb/libvxpdb.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HXbtree;
struct HXdeque;

struct mdsync_config {
    int new_pw_length, genpw_type, crypw_type;
    unsigned int postadd_flush;
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
        unsigned long, unsigned long);

    struct vxpdb_state *database;
    hmc_t *output_data;
    unsigned long num_grp;

    // private to mdsync
    struct vxpdb_group dest_group;
    FILE *logfile;
    struct HXdeque *defer_start, *defer_wait, *defer_stop, *delete_now;
    struct HXbtree *add_req;
    struct HXbtree *lnlist, *update_req;
};

/*
 *      FIXUP.C
 */
extern void mdsync_fixup(struct mdsync_workspace *);

/*
 *      GEN.C
 */
extern struct mdsync_workspace *mdsync_init(void);
extern int mdsync_prepare_group(struct mdsync_workspace *, const char *);
extern int mdsync_open_log(struct mdsync_workspace *, const char *);
extern void mdsync_free(struct mdsync_workspace *);

/*
 *      PROC.C
 */
extern void mdsync_compare(struct mdsync_workspace *);
extern void mdsync_compare_simple(struct mdsync_workspace *);
extern int mdsync_add(struct mdsync_workspace *);
extern int mdsync_mod(struct mdsync_workspace *);
extern int mdsync_del(struct mdsync_workspace *);

/*
 *      READ_FILE.C
 */
extern int mdsync_read_file(struct mdsync_workspace *, const char *,
    const char *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _VITALNIX_LIBVXMDSYNC_LIBVXMDSYNC_H

//=============================================================================
