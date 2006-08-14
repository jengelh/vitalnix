/*=============================================================================
Vitalnix User Management Suite
clutils/pdbdump.c - Dump PDB contents
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2004 - 2006
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
// sorta like `getent passwd`
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <libHX.h>
#include "vitalnix-config.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"

// Functions
static void dump_as_ldap(struct vxpdb_state *);
static void dump_as_mysql(struct vxpdb_state *);
static void dump_as_shadow(struct vxpdb_state *);

static int get_options(int *, const char ***);
static void show_version(const struct HXoptcb *);

// Variables
enum output_type_e {
    OUTPUT_SHADOW = 0,
    OUTPUT_LDAP,
    OUTPUT_MYSQL,
} Output_type = OUTPUT_SHADOW;
static char *Module = "*";
static void (*Dump_functions[])(struct vxpdb_state *) = {
    [OUTPUT_LDAP]   = dump_as_ldap,
    [OUTPUT_MYSQL]  = dump_as_mysql,
    [OUTPUT_SHADOW] = dump_as_shadow,
};

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct vxpdb_state *db;

    if(!get_options(&argc, &argv))
        return EXIT_FAILURE;

    if((db = vxpdb_load(Module)) == NULL) {
        fprintf(stderr, "vxpdb_load(\"%s\"): %s\n", Module, strerror(errno));
        return EXIT_FAILURE;
    }
    if(vxpdb_open(db, 0) <= 0) {
        vxpdb_unload(db);
        perror("vxpdb_open()");
        return EXIT_FAILURE;
    }

    // Walk users
    Dump_functions[Output_type](db);
    vxpdb_close(db);
    vxpdb_unload(db);
    return 0;
}

//-----------------------------------------------------------------------------
static void dump_as_ldap(struct vxpdb_state *db) {
    return;
}

static void dump_as_mysql(struct vxpdb_state *db) {
    struct vxpdb_group group = {};
    struct vxpdb_user user   = {};
    void *trav;

    printf(
        "drop table if exists users;\n"
        "drop table if exists shadow;\n"
        "drop table if exists vxshadow;\n"
        "drop table if exists groups;\n"
        "drop table if exists groupmaps;\n"

        "create table users (\n"
        "    username varchar(64) not null default '',\n"
        "    uid int(11) not null default %d,\n"
        "    gid int(11) not null default %d,\n"
        "    realname varchar(255) not null default '',\n"
        "    homedir varchar(255) not null default '',\n"
        "    shell varchar(255) not null default '/bin/bash',\n"
        "    unused0 varchar(255) not null default 'x',\n"
        "    primary key (username),\n"
        "    index uid (uid)\n"
        ") default charset=utf8\n"

        "create table shadow (\n"
        "    username varchar(64) not null default '',\n"
        "    password varchar(255) not null default '!',\n"
        "    lastchange int(11) not null default 0,\n"
        "    minkeep int(11) not null default %d,\n"
        "    maxkeep int(11) not null default %d,\n"
        "    warnage int(11) not null default %d,\n"
        "    expire int(11) not null default %d,\n"
        "    inactive int(11) not null default %d,\n"
        "    primary key (username)\n"
        ") default charset=utf8\n"

        "create table vxshadow (\n"
        "    username varchar(64) not null default '',\n"
        "    uuid varchar(255) not null default '',\n"
        "    pvgrp varchar(64) not null default '',\n"
        "    defer int(11) not null default 0,\n"
        "    primary key (username)\n"
        ") default charset=utf8\n"

        "create table groups (\n"
        "    group_name varchar(64) not null default '',\n"
        "    gid int(11) not null default %d,\n"
        "    group_password varchar(255) not null default '*',\n"
        "    primary key (group_name),\n"
        "    index gid (gid)\n"
        ") default charset=utf8;\n"

        "create table groupmaps (\n"
        "    user_name varchar(64) not null default '',\n"
        "    group_name varchar(64) not null default '',\n"
        ") default charset=utf8;\n",
        PDB_NOUID, PDB_NOGID, PDB_DFL_KEEPMIN, PDB_DFL_KEEPMAX,
        PDB_DFL_WARNAGE, PDB_NO_EXPIRE, PDB_NO_INACTIVE, PDB_NOGID
    );

    printf("lock tables users write, shadow write, vxshadow write, "
           "groups write, groupmap write;\n");

    // User
    if((trav = vxpdb_usertrav_init(db)) == NULL)
        return;
    while(vxpdb_usertrav_walk(db, trav, &user) > 0) {
        printf("insert into users values ('%s',%ld,%ld,'%s','%s','%s');\n",
               user.pw_name, user.pw_uid, user.pw_gid, user.pw_real,
               user.pw_home, user.pw_shell);
        if(user.sp_passwd != NULL)
            printf("insert into shadow values ('%s','%s',%ld,%ld,%ld,%ld,%ld,"
                   "%ld);\n", user.pw_name, user.sp_passwd, user.sp_lastchg,
                   user.sp_min, user.sp_max, user.sp_warn, user.sp_expire,
                   user.sp_inact);
        if(user.vs_uuid != NULL || user.vs_pvgrp != NULL || user.vs_defer != 0)
            printf("insert into vxshadow values ('%s','%s','%s',%ld);\n",
                   user.pw_name, user.vs_uuid, user.vs_pvgrp, user.vs_defer);
    }
    vxpdb_usertrav_free(db, trav);

    // Group
    if((trav = vxpdb_grouptrav_init(db)) == NULL)
        return;
    while(vxpdb_grouptrav_walk(db, trav, &group) > 0)
        printf("insert into groups values ('%s',%ld);\n",
         group.gr_name, group.gr_gid);
    vxpdb_grouptrav_free(db, trav);

    vxpdb_user_free(&user, 0);
    vxpdb_group_free(&group, 0);

    // Info
    printf("unlock tables;\n");
    printf("# Number of users: %ld\n", vxpdb_modctl(db, PDB_COUNT_USERS));
    printf("# Number of groups: %ld\n", vxpdb_modctl(db, PDB_COUNT_GROUPS));
    return;
}

static void dump_as_shadow(struct vxpdb_state *db) {
    struct vxpdb_group group = {};
    struct vxpdb_user user   = {};
    void *trav;

    // Users
    printf("#---/etc/passwd---\n");
    if((trav = vxpdb_usertrav_init(db)) == NULL)
        return;
    while(vxpdb_usertrav_walk(db, trav, &user) > 0)
        printf("%s:x:%ld:%ld:%s:%s:%s\n", user.pw_name, user.pw_uid,
         user.pw_gid, user.pw_real, user.pw_home, user.pw_shell);
    vxpdb_usertrav_free(db, trav);

    // Shadow
    printf("#---/etc/shadow---\n");
    if((trav = vxpdb_usertrav_init(db)) == NULL)
        return;
    while(vxpdb_usertrav_walk(db, trav, &user) > 0)
        if(user.sp_passwd != NULL)
            printf("%s:%s:%ld:%ld:%ld:%ld:%ld:%ld\n", user.pw_name,
             user.sp_passwd, user.sp_lastchg, user.sp_min, user.sp_max,
             user.sp_warn, user.sp_expire, user.sp_inact);
    vxpdb_usertrav_free(db, trav);

    // vxShadow
    printf("#---/etc/vxshadow---\n");
    if((trav = vxpdb_usertrav_init(db)) == NULL)
        return;
    while(vxpdb_usertrav_walk(db, trav, &user) > 0)
        if(user.vs_uuid != NULL || user.vs_pvgrp != NULL || user.vs_defer != 0)
            printf("%s:%s:%s:%ld\n", user.pw_name, user.vs_uuid, user.vs_pvgrp,
             user.vs_defer);
    vxpdb_usertrav_free(db, trav);

    // Group
    printf("#---/etc/group---\n");
    if((trav = vxpdb_grouptrav_init(db)) == NULL)
        return;
    while(vxpdb_grouptrav_walk(db, trav, &group) > 0)
        printf("%s:*:%ld:\n", group.gr_name, group.gr_gid);
    vxpdb_grouptrav_free(db, trav);

    vxpdb_user_free(&user, 0);
    vxpdb_group_free(&group, 0);

    // Info
    printf("#---INFO---\n");
    printf("# Number of users: %ld\n", vxpdb_modctl(db, PDB_COUNT_USERS));
    printf("# Number of groups: %ld\n", vxpdb_modctl(db, PDB_COUNT_GROUPS));
    return;
}

//-----------------------------------------------------------------------------
static int get_options(int *argc, const char ***argv) {
    struct HXoption options_table[] = {
        {.ln = "ldap",   .type = HXTYPE_VAL, .ptr = &Output_type,
         .val = OUTPUT_LDAP, .help = "Output in LDIF format"},
        {.ln = "mysql",  .type = HXTYPE_VAL, .ptr = &Output_type,
         .val = OUTPUT_MYSQL, .help = "Output in MYSQLdump format"},
        {.ln = "shadow", .type = HXTYPE_VAL, .ptr = &Output_type,
         .val = OUTPUT_SHADOW, .help = "Output in SHADOW format (default)"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &Module,
         .help = "Use specified PDB back-end", .htyp = "MODULE"},
        {.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
         .help = "Show version information"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };
    return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi) {
    printf("Vitalnix " VITALNIX_VERSION " pdbdump\n");
    exit(EXIT_SUCCESS);
}

//=============================================================================
