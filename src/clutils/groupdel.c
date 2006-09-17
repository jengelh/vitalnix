/*=============================================================================
Vitalnix User Management Suite
clutils/groupdel.c - Delete a group
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2006
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "vitalnix-config.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"
#include "libvxutil/libvxutil.h"

enum {
    E_SUCCESS = 0,
    E_OTHER,      // other error, see errno
    E_OPEN,       // unable to open back-end module or DB
    E_NOEXIST,    // group does not exist
    E_UPDATE,     // db->groupdel() did not return ok
    E_CLOSE,      // db->close() did not return ok
    E_PRIMARY,    // will not delete a user's primary group, unless -F is given
};

static struct {
    char *gname;
    // groupdel internal
    char *ac_after, *ac_before;
    int force;
} Opt = {};

// Functions
static int groupdel_check_pri_group(struct vxpdb_state *, struct vxpdb_group *);
static int groupdel_get_options(int *, const char ***);
static int groupdel_read_config(void);

// Variables
static char *Module_path = "*";

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct vxpdb_state *db;
    struct vxpdb_group mmask = {};
    int ret, rv = E_SUCCESS;
    struct HXoption ext_catalog[] = {
        {.sh = 'G', .type = HXTYPE_STRING, .ptr = &mmask.gr_name},
        HXOPT_TABLEEND,
    };

    if(groupdel_read_config() <= 0 || groupdel_get_options(&argc, &argv) <= 0)
        return E_OTHER;

    // ----------------------------------------
    if((db = vxpdb_load(Module_path)) == NULL) {
        fprintf(stderr, "Could not load PDB back-end module \"%s\": %s\n",
                Module_path, strerror(errno));
        return E_OPEN;
    }

    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open PDB back-end: %s\n", strerror(-ret));
        rv = E_OPEN;
        goto __main__close_pdb;
    }

    mmask.gr_gid  = PDB_NOGID;
    mmask.gr_name = Opt.gname = argv[1];

    // ----------------------------------------
    if((ret = vxpdb_groupinfo(db, &mmask, NULL, 0)) < 0) {
        fprintf(stderr, "Error querying the PDB: %s\n", strerror(-ret));
        rv = E_OTHER;
        goto __main__close_backend;
    } else if(ret == 0) {
        fprintf(stderr, "Group \"%s\" does not exist\n", mmask.gr_name);
        rv = E_NOEXIST;
        goto __main__close_backend;
    }

    if(!Opt.force && groupdel_check_pri_group(db, &mmask)) {
        fprintf(stderr, "Will not remove a user's primary group\n");
        rv = E_PRIMARY;
        goto __main__close_backend;
    }

    // ----------------------------------------
    if(Opt.ac_before != NULL)
        vxutil_replace_run(Opt.ac_before, ext_catalog);

    if((ret = vxpdb_groupdel(db, &mmask)) <= 0) {
        fprintf(stderr, "Error: Deleting group failed: %s\n", strerror(-ret));
        rv = E_UPDATE;
    } else if(Opt.ac_after != NULL) {
        vxutil_replace_run(Opt.ac_after, ext_catalog);
    }

    // ----------------------------------------
 __main__close_backend:
    vxpdb_close(db);
 __main__close_pdb:
    vxpdb_unload(db);
    return rv;
}

//-----------------------------------------------------------------------------
static int groupdel_check_pri_group(struct vxpdb_state *db,
  struct vxpdb_group *mm)
{
    struct vxpdb_user user;
    int pg = 0; // some user has this as primary group
    void *travp;

    if((travp = vxpdb_usertrav_init(db)) == NULL)
        return 0;

    while(vxpdb_usertrav_walk(db, travp, &user) > 0)
        if((user.pw_gid != PDB_NOGID && user.pw_gid == mm->gr_gid) ||
         (user.pw_igrp != NULL && mm->gr_name != NULL &&
         strcmp(user.pw_igrp, mm->gr_name) == 0)) {
            pg = 1;
            break;
        }

    vxpdb_usertrav_free(db, travp);
    return pg;
}

static int groupdel_get_options(int *argc, const char ***argv) {
    static const struct HXoption options_table[] = {
        // New, Vitalnix-userdel options
        {.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_after,
         .help = "Program to run after group deletion", .htyp = "cmd"},
        {.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL, .ptr = &Opt.ac_before,
         .help = "Program to run before group deletion", .htyp = "cmd"},
        {.sh = 'F', .type = HXTYPE_NONE, .ptr = &Opt.force,
         .help = "Force deletion of group even if users have it as primary group"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &Module_path,
         .help = "Use a different module than \"*\" (the default)", .htyp = "name"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;

    if(argv[1] == NULL) {
        // Group name is mandatory
        fprintf(stderr, "You need to specify a group name.\n");
        return 0;
    }

    return 1;
}

static int groupdel_read_config(void) {
    static const struct HXoption config_table[] = {
        {.ln = "GROUP_PREDEL", .type = HXTYPE_STRING, .ptr = &Opt.ac_before},
        {.ln = "GROUP_PREDEL", .type = HXTYPE_STRING, .ptr = &Opt.ac_after},
        HXOPT_TABLEEND,
    };
    return HX_shconfig(CONFIG_ETC_VITALNIX "/groupdel.conf", config_table);
}

//=============================================================================
