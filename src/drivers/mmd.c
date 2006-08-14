/*=============================================================================
Vitalnix User Management Suite
drivers/mmd.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006
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
#include <libHX.h>
#include "vitalnix-config.h"
#include "drivers/proto.h"
#include "drivers/static-build.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"

#define WR_OPEN(state)  ((state)->wr_mod.mh_state == STATE_OPEN)
#define WR_MOD(state)   ((state)->wr_mod.mh_instance)

// Definitions
enum open_state {
    STATE_OUT,
    STATE_LOADED,
    STATE_OPEN,
};

struct module_handle {
    char *mh_name;
    enum open_state mh_state;
    struct vxpdb_state *mh_instance;
};

struct multi_state {
    int destructing;
    struct HXdeque *rd_mod;
    struct module_handle wr_mod;
};

// Functions
DRIVER_PROTO_ALL(vmmd);
static int modules_construct(struct multi_state *, char **);
static int modules_open(struct multi_state *, long);
static void modules_close(struct multi_state *);
static void modules_destruct(struct multi_state *);
static void read_config(struct multi_state *);
static void read_rdmod(const struct HXoptcb *);
static void read_wrmod(const struct HXoptcb *);

// Variables
static struct vxpdb_mvtable THIS_MODULE = {
    .name           = "Multiple Module driver",
    .author         = "Jan Engelhardt <jengelh [at] gmx de>, 2006",
    DRIVER_CB_ALL(vmmd),
};

//-----------------------------------------------------------------------------
REGISTER_MODULE(multi, &THIS_MODULE);

static int vmmd_init(struct vxpdb_state *vp, const char *config_file) {
    struct multi_state *state;

    if((state = vp->state = calloc(1, sizeof(struct multi_state))) == NULL)
        return -errno;

    read_config(state);
    return 1;
}

static int vmmd_open(struct vxpdb_state *vp, long flags) {
    struct multi_state *state = vp->state;
    int ret;

    if((ret = modules_open(state, flags)) <= 0)
        return ret;

    return 1;
}

static void vmmd_close(struct vxpdb_state *vp) {
    struct multi_state *state = vp->state;
    modules_close(state);
    return;
}

static void vmmd_deinit(struct vxpdb_state *vp) {
    struct multi_state *state = vp->state;
    modules_destruct(state);
    free(state);
    return;
}

static long vmmd_modctl(struct vxpdb_state *vp, long command, ...) {
    return -ENOSYS;
}

static int vmmd_lock(struct vxpdb_state *vp) {
    struct multi_state *state = vp->state;
    if(WR_OPEN(state))
        return vxpdb_lock(WR_MOD(state));
    return 1;
}

static int vmmd_unlock(struct vxpdb_state *vp) {
    struct multi_state *state = vp->state;
    if(WR_OPEN(state))
        return vxpdb_unlock(WR_MOD(state));
    return 1;
}

//-----------------------------------------------------------------------------
static int vmmd_useradd(struct vxpdb_state *vp, const struct vxpdb_user *rq) {
    struct multi_state *state = vp->state;
    if(WR_OPEN(state))
        return vxpdb_useradd(WR_MOD(state), rq);
    return -EROFS;
}

static int vmmd_usermod(struct vxpdb_state *vp,
 const struct vxpdb_user *sr_mask, const struct vxpdb_user *mod_mask)
{
    struct multi_state *state = vp->state;
    if(WR_OPEN(state))
        return vxpdb_usermod(WR_MOD(state), sr_mask, mod_mask);
    return -EROFS;
}

static int vmmd_userdel(struct vxpdb_state *vp,
 const struct vxpdb_user *sr_mask)
{
    struct multi_state *state = vp->state;
    if(WR_OPEN(state))
        return vxpdb_userdel(WR_MOD(state), sr_mask);
    return -EROFS;
}

static int vmmd_userinfo(struct vxpdb_state *vp,
 const struct vxpdb_user *sr_mask, struct vxpdb_user *dest, size_t size)
{
/*    struct shadow_state *state = vp->state;
    const struct HXdeque_node *travp = state->dq_user->first;
    struct vxpdb_user temp_mask;
    int found = 0;

    if(mask == result) {
        memcpy(&temp_mask, mask, sizeof(struct vxpdb_user));
        mask = &temp_mask;
    }

    for(travp = state->dq_user->first;
     travp != NULL && (dest == NULL || size > 0); travp = travp->Next)
    {
        const struct vxpdb_user *src = travp->ptr;
        if(!vxpdb_user_match(src, mask))
            continue;
        if(result != NULL) {
            vxpdb_user_copy(result, src);
            ++result;
            ++found;
            --size;
        } else {
            if(size == 0)
                return 1;
            ++found;
        }
    }

    return found;*/
    return 0;
}

static void *vmmd_usertrav_init(struct vxpdb_state *vp) {
    return NULL;
}

static int vmmd_usertrav_walk(struct vxpdb_state *vp, void *ptr,
 struct vxpdb_user *dest)
{
    return 1;
}

static void vmmd_usertrav_free(struct vxpdb_state *vp, void *ptr) {
    return;
}

//-----------------------------------------------------------------------------
static int vmmd_groupadd(struct vxpdb_state *vp, const struct vxpdb_group *rq) {
    struct multi_state *state = vp->state;
    if(WR_OPEN(state))
        return vxpdb_groupadd(WR_MOD(state), rq);
    return -EROFS;
}

static int vmmd_groupmod(struct vxpdb_state *vp,
 const struct vxpdb_group *sr_mask, const struct vxpdb_group *mod_mask)
{
    struct multi_state *state = vp->state;
    if(WR_OPEN(state))
        return vxpdb_groupmod(WR_MOD(state), sr_mask, mod_mask);
    return -EROFS;
}

static int vmmd_groupdel(struct vxpdb_state *vp,
 const struct vxpdb_group *sr_mask)
{
    struct multi_state *state = vp->state;
    if(WR_OPEN(state))
        return vxpdb_groupdel(WR_MOD(state), sr_mask);
    return -EROFS;
}

static void *vmmd_grouptrav_init(struct vxpdb_state *vp) {
    return NULL;
}

static int vmmd_grouptrav_walk(struct vxpdb_state *vp, void *ptr,
 struct vxpdb_group *dest)
{
    return 1;
}

static void vmmd_grouptrav_free(struct vxpdb_state *vp, void *ptr) {
    return;
}

static int vmmd_groupinfo(struct vxpdb_state *vp,
 const struct vxpdb_group *sr_mask, struct vxpdb_group *dest, size_t size)
{
    return 0;
}

//-----------------------------------------------------------------------------
static int modules_construct(struct multi_state *state, char **name) {
    state->rd_mod = HXdeque_init();

    while(name != NULL && *name != NULL) {
        struct module_handle mh = {
            .mh_name     = *name,
            .mh_state    = STATE_OUT,
        };
        HXdeque_push(state->rd_mod, HX_memdup(&mh, sizeof(mh)));
        ++name;
    }

    return 1;
}

static int modules_open(struct multi_state *state, long flags) {
    struct HXdeque_node *node = state->rd_mod->first;
    int ret = 1;

    for(node = state->rd_mod->first; node != NULL; node = node->Next) {
        struct module_handle *mh = node->ptr;
        if((ret = vxpdb_open(mh->mh_instance, flags & ~PDB_WRLOCK)) <= 0) {
            modules_close(state);
            return ret;
        }
        mh->mh_state = STATE_OPEN;
    }

    return ret;
}

static void modules_close(struct multi_state *state) {
    struct HXdeque_node *node;

    for(node = state->rd_mod->first; node != NULL; node = node->Next) {
        struct module_handle *mod = node->ptr;
        if(mod->mh_state == STATE_OPEN)
            vxpdb_close(mod->mh_instance);
        mod->mh_state = STATE_LOADED;
    }

    if(WR_OPEN(state)) {
        vxpdb_close(WR_MOD(state));
        state->wr_mod.mh_state = STATE_LOADED;
    }

    return;
}

static void modules_destruct(struct multi_state *state) {
    struct HXdeque_node *node;

    for(node = state->rd_mod->first; node != NULL; node = node->Next) {
        struct module_handle *mod = node->ptr;
        if(mod->mh_state == STATE_LOADED) {
            vxpdb_unload(mod->mh_instance);
            mod->mh_state = STATE_OUT;
        }
    }

    if(state->wr_mod.mh_state == STATE_LOADED) {
        vxpdb_unload(WR_MOD(state));
        state->wr_mod.mh_state = STATE_OUT;
    }

    return;
}

static void read_config(struct multi_state *state) {
    struct HXoption options_table[] = {
        {.ln = "RD_MODULES", .type = HXTYPE_STRING, .cb = read_rdmod, .uptr = state},
        {.ln = "WR_MODULE",  .type = HXTYPE_STRING, .cb = read_wrmod, .uptr = state},
        HXOPT_TABLEEND,
    };

    HX_shconfig(CONFIG_ETC_VITALNIX "/db_multi.conf", options_table);
    return;
}

static void read_rdmod(const struct HXoptcb *cbi) {
    struct multi_state *state = cbi->current->uptr;
    char **mod = HX_split(cbi->s, ":", NULL, 0);
    if((state->destructing = modules_construct(state, mod)) <= 0)
        modules_destruct(state);
    HX_zvecfree(mod);
    return;
}

static void read_wrmod(const struct HXoptcb *cbi) {
    struct multi_state *state = cbi->current->uptr;
    if(state->destructing == 0) {
        state->wr_mod.mh_name  = HX_strdup(cbi->s);
        state->wr_mod.mh_state = STATE_OUT;
    }
    return;
}

//=============================================================================