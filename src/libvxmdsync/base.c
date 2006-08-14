/*=============================================================================
Vitalnix User Management Suite
libvxmdsync/base.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2006
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
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "compiler.h"
#include "vitalnix-config.h"
#include "libvxeds/libvxeds.h"
#include "libvxmdsync/libvxmdsync.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xwfunc.h"
#include "libvxutil/defines.h"
#include "libvxutil/libvxutil.h"

// Functions
static void kill_eds(const struct HXbtree_node *);
static void kill_pwd(const struct HXbtree_node *);
static int mdsync_read_config(struct mdsync_config *);
static void pconfig_crymeth(const struct HXoptcb *);
static void pconfig_genpw(const struct HXoptcb *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL struct mdsync_workspace *mdsync_init(void)
{
    struct mdsync_workspace *w;
    struct mdsync_config *c;
    int esave;

    if((w = calloc(1, sizeof(struct mdsync_workspace))) == NULL)
        goto out;

    // Used in mdsync_read_file()
    if((w->add_req = HXbtree_init(HXBT_MAP | HXBT_SCMP)) == NULL)
        goto out;
    if((w->update_req = HXbtree_init(HXBT_MAP | HXBT_SCMP)) == NULL)
        goto out;

    // Used in mdsync_compare()
    w->defer_start = HXdeque_init();
    w->defer_wait  = HXdeque_init();
    w->defer_stop  = HXdeque_init();
    w->delete_now  = HXdeque_init();
    w->lnlist      = HXbtree_init(HXBT_CDATA | HXBT_SCMP);

    if(w->defer_start == NULL || w->defer_wait == NULL ||
     w->defer_stop == NULL || w->delete_now == NULL || w->lnlist == NULL)
        goto out;

    // mdsync_add()
    w->output_data = hmc_minit(NULL, 64);

    // Defaults, defaults, defaults
    c = &w->config;
    c->new_pw_length = 10;
    c->genpw_type    = GENPW_ZH;
    c->crypw_type    = CRYPW_BLOWFISH;
    //c->home_umask    = S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    //c->skeleton_dir  = NULL;
    //c->home_base     = "/home";
    //c->split_level   = 0;
    //c->user_defaults.pw_shell = "/bin/bash";

    mdsync_read_config(c);
    return w;

 out:
    esave = errno;
    if(w != NULL)
        mdsync_free(w);
    errno = esave;
    return NULL;
}

/* Side effects: Allocates w->dest_group (freed in mdsync_free()). */
EXPORT_SYMBOL int mdsync_prepare_group(struct mdsync_workspace *w,
 const char *name)
{
    if(vxutil_only_digits(name))
        return vxpdb_getgrgid(w->database, strtol(name, NULL, 0),
               &w->dest_group);
    else
        return vxpdb_getgrnam(w->database, name, &w->dest_group);
}

EXPORT_SYMBOL int mdsync_open_log(struct mdsync_workspace *w,
 const char *output_file)
{
    if((w->logfile = fopen(output_file, "w")) == NULL)
        return -errno;

    setvbuf(w->logfile, NULL, _IOLBF, 0);

    // change the number if the format should ever change
    fprintf(w->logfile,
        "# $logformat " MDSYNC_LOG_VER "\n"
        "# Vitalnix libvxmdsync %s\n", VITALNIX_VERSION);
    return 1;
}

EXPORT_SYMBOL void mdsync_free(struct mdsync_workspace *w)
{
    // mdsync_add()
    hmc_free(w->output_data);

    // mdsync_open_log()
    if(w->logfile != NULL)
        fclose(w->logfile);

    // mdsync_fixup()

    // mdsync_compare()
    /* Auto-genocide of the linked list keys, because @w->lnlist (the owner
    of the keys) has HXBT_CDATA. See compare.c for comment. */
    if(w->defer_start != NULL) HXdeque_free(w->defer_start);
    if(w->defer_wait  != NULL) HXdeque_free(w->defer_wait);
    if(w->defer_stop  != NULL) HXdeque_free(w->defer_stop);
    if(w->delete_now  != NULL) HXdeque_free(w->delete_now);
    if(w->lnlist      != NULL) HXbtree_free(w->lnlist);

    // mdsync_read_file()
    if(w->add_req != NULL) {
        if(w->add_req->root != NULL)
            kill_eds(w->add_req->root);
        HXbtree_free(w->add_req);
    }
    if(w->update_req != NULL) {
        if(w->update_req->root != NULL)
            kill_pwd(w->update_req->root);
        HXbtree_free(w->update_req);
    }

    // mdsync_prepare_group()
    vxpdb_group_free(&w->dest_group, 0);
    return;
}

//-----------------------------------------------------------------------------
/*  kill_eds
    @node:      node to free data at

    Recursively descent into @node and its subtrees to free the EDS entries.
*/
static void kill_eds(const struct HXbtree_node *node) {
    vxeds_free_entry(node->data);
    if(node->sd[0] != NULL)
        kill_eds(node->sd[0]);
    if(node->sd[1] != NULL)
        kill_eds(node->sd[1]);
    return;
}

/*  kill_pwd
    @node:      node to free data at

    Recursively descend into @node and its subtrees to free the PWD data
    structures.
*/
static void kill_pwd(const struct HXbtree_node *node) {
    vxpdb_user_free(node->data, 1);
    if(node->sd[0] != NULL)
        kill_pwd(node->sd[0]);
    if(node->sd[1] != NULL)
        kill_pwd(node->sd[1]);
    return;
}

static int mdsync_read_config(struct mdsync_config *c) {
    struct HXoption mdsync_options_table[] = {
        {.ln = "POSTADD_FLUSH",     .type = HXTYPE_NONE,   .ptr = &c->postadd_flush},
//        {.ln = "DEFAULT_BACKEND",   .type = HXTYPE_STRING, .ptr = &Opt.module_path},
        {.ln = "DEFERRED_DELETION", .type = HXTYPE_LONG,   .ptr = &c->add_opts.defaults.vs_defer},
        {.ln = "PSWD_LEN",          .type = HXTYPE_INT,    .ptr = &c->new_pw_length},
        {.ln = "PSWD_METH",         .type = HXTYPE_NONE,   .cb  = pconfig_crymeth, .uptr = &c->crypw_type},
        {.ln = "GENPW_TYPE",        .type = HXTYPE_INT,    .cb  = pconfig_genpw, .uptr = &c->genpw_type},
        HXOPT_TABLEEND,
    };
    vxconfig_read_useradd(CONFIG_ETC_VITALNIX "/useradd.conf", &c->add_opts);
    vxconfig_read_useradd(CONFIG_ETC_VITALNIX "/libvxmdsync.conf", &c->add_opts);
    HX_shconfig(CONFIG_ETC_VITALNIX "/libvxmdsync.conf", mdsync_options_table);
    return 1;
}

static void pconfig_crymeth(const struct HXoptcb *cbi) {
    int *ptr = cbi->current->uptr;

    if(stricmp(cbi->s, "md5") == 0) {
#ifdef _WIN32
        fprintf(stderr, "Warning: No MD5 support under Win32.\n");
#else
        *ptr = CRYPW_MD5;
#endif
    } else if(stricmp(cbi->s, "des") == 0) {
#ifdef _WIN32
        fprintf(stderr, "Warning: No DES support under Win32.\n");
#else
        *ptr = CRYPW_DES;
#endif
    } else {
        *ptr = CRYPW_BLOWFISH;
    }
    return;
}

static void pconfig_genpw(const struct HXoptcb *cbi) {
    int *ptr = cbi->current->uptr;

    if(stricmp(cbi->s, "jp") == 0)
        *ptr = GENPW_JP;
    else if(stricmp(cbi->s, "zh") == 0)
        *ptr = GENPW_ZH;
    else
        *ptr = 0;
    return;
}

//=============================================================================