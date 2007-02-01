/*=============================================================================
Vitalnix User Management Suite
clutils/userdel_lib.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2007
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
#include <unistd.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include "clutils/userdel_lib.h"
#include <vitalnix/libvxpdb/config.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/defines.h>

// Functions
static void userdel_getopt_predel(const struct HXoptcb *);
static void userdel_getopt_postdel(const struct HXoptcb *);
static int userdel_read_config(struct userdel_state *);
static int userdel_slash_count(const char *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int userdel_fill_defaults(struct userdel_state *sp)
{
    int ret;

    memset(sp, 0, sizeof(struct userdel_state));
    sp->database = "*";

    if((ret = userdel_read_config(sp)) <= 0)
        return ret;

    return 1;
}

EXPORT_SYMBOL int userdel_get_options(int *argc, const char ***argv,
  struct userdel_state *state)
{
    struct vxconfig_userdel *conf = &state->config;
    struct HXoption options_table[] = {
        // New (vxuserdel) options
        {.sh = 'A', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
         .cb = userdel_getopt_postdel, .uptr = conf,
         .help = "Program to run after user modification", .htyp = "cmd"},
        {.sh = 'B', .type = HXTYPE_STRING | HXOPT_OPTIONAL,
         .cb = userdel_getopt_predel, .uptr = conf,
         .help = "Program to run before user modification", .htyp = "cmd"},
        {.sh = 'F', .type = HXTYPE_NONE, .ptr = &state->force,
         .help = "Force deletion even if UID is 0 or name is 'root'"},
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &state->database,
         .help = "Use specified database", .htyp = "name"},

        // Default options
        {.sh = 'r', .type = HXTYPE_NONE, .ptr = &state->rm_home,
         .help = "Remove home directory, cron tables and mail spool"},

        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
        return 0;

    return 1;
}

EXPORT_SYMBOL int userdel_run(struct userdel_state *state)
{
    struct vxpdb_user search, result;
    int ret, ierr = UD_SUCCESS;
    struct vxpdb_state *db;
    char *username, *home;

    if((db = vxpdb_load(state->database)) == NULL)
        return errno | (UD_ELOAD << UD_SHIFT);

    if((ret = vxpdb_open(db, PDB_WRLOCK)) <= 0) {
        ierr = UD_EOPEN;
        goto close_adb;
    }

    vxpdb_user_clean(&search);
    search.pw_name = state->username;
    if((ret = vxpdb_userinfo(db, &search, &result, 1)) < 0) {
        ierr = UD_EQUERY;
        goto close_backend;
    } else if(ret == 0) {
        ierr = UD_ENOEXIST;
        goto close_backend;
    }

    if(!state->force && (strcmp(result.pw_name, "root") == 0 ||
     result.pw_uid == 0)) {
        ierr = UD_EDENY;
        goto close_backend;
    }
/*
    if(state->ac_before != NULL)
        replace_and_runcmd(state->ac_before, sr_map);
*/
    username = HX_strdup(result.pw_name);
    home     = HX_strdup(result.pw_home);

    if((ret = vxpdb_userdel(db, &search)) <= 0) {
        ierr = UD_EUPDATE;
        goto close_backend2;
    }

    if(state->rm_home) {
        if(home == NULL || *home == '\0')
            fprintf(stderr, "Warning: User had no home directory. "
             "Not removing anything.\n");
        else if(strcmp(home, "/") == 0)
            fprintf(stderr, "Warning: Will refuse to delete "
             "home directory \"/\"\n");
        else if(userdel_slash_count(home) <= 1 && !state->force)
            fprintf(stderr, "Warning: Will not remove home directory \"%s\""
             " which has less than two slashes and thus looks like a system"
             " or a malformed directory. (Remove it manually.)\n", home);
        else
            HX_rrmdir(home);
    }
    if(state->rm_cron) {
        char buf[MAXFNLEN];
        snprintf(buf, sizeof(buf), "crontab -r \"%s\"", username);
        system(buf);
    }
    if(state->rm_mail) {
        char buf[MAXFNLEN];
        snprintf(buf, sizeof(buf), "/var/spool/mail/%s", username);
        unlink(buf);
    }
/*
    if(state->ac_after != NULL)
        replace_and_runcmd(state->ac_after, sr_map);
*/
 close_backend2:
    free(home);

 close_backend:
    vxpdb_close(db);

 close_adb:
    vxpdb_unload(db);
    return ret | (ierr << UD_SHIFT);
}

EXPORT_SYMBOL const char *userdel_strerror(int e)
{
    switch(e >> UD_SHIFT) {
        case UD_ELOAD:
            return "Could not load PDB back-end";
        case UD_EOPEN:
            return "Could not open PDB back-end";
        case UD_EQUERY:
            return "Error querying PDB";
        case UD_ENOEXIST:
            return "User does not exist";
        case UD_EDENY:
            return "Refusing to remove \"root\" or UID 0 accounts when not using the force option";
        case UD_EUPDATE:
            return "Error deleting user";
        case UD_EPOST:
            return "Error during post setup";
        case UD_ECLOSE:
            return "Warning during PDB close";
    }
    return "(unknown userdel error)";
}

//-----------------------------------------------------------------------------
static void userdel_getopt_predel(const struct HXoptcb *cbi)
{
    struct vxconfig_userdel *conf = cbi->current->uptr;
    conf->master_predel = NULL;
    conf->user_predel   = HX_strdup(cbi->s);
    return;
}

static void userdel_getopt_postdel(const struct HXoptcb *cbi)
{
    struct vxconfig_userdel *conf = cbi->current->uptr;
    conf->master_postdel = NULL;
    conf->user_postdel   = HX_strdup(cbi->s);
    return;
}

static int userdel_read_config(struct userdel_state *state) {
    int err, ret = 0;
    struct HXoption config_table[] = {
        {.ln = "REMOVE_CRON", .type = HXTYPE_BOOL,   .ptr = &state->rm_cron},
        {.ln = "REMOVE_HOME", .type = HXTYPE_BOOL,   .ptr = &state->rm_home},
        {.ln = "REMOVE_MAIL", .type = HXTYPE_BOOL,   .ptr = &state->rm_mail},
        HXOPT_TABLEEND,
    };
    err = vxconfig_read_userdel(CONFIG_SYSCONFDIR "/userdel.conf",
          &state->config);
    if(err < 0) 
        ret = err;
    err = HX_shconfig(CONFIG_SYSCONFDIR "/userdel.conf", config_table);
    if(err < 0 && ret == 0)
        ret = err;
    return ret;
}

static int userdel_slash_count(const char *fn) {
    const char *ptr = fn;
    int n = 0;

    if(fn == NULL)
        return 0;

    while((ptr = strchr(ptr, '/')) != NULL) {
        ++n;
        ++ptr;
    }

    return n;
}

//=============================================================================
