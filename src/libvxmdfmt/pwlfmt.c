/*=============================================================================
Vitalnix User Management Suite
libvxmdfmt/pwlfmt.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2006
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
#include <vitalnix/compiler.h>
#include <vitalnix/config.h>
#include "libvxcore/loader.h"
#include "libvxmdfmt/internal.h"
#include "libvxmdfmt/libvxmdfmt.h"
#include "libvxmdfmt/vtable.h"
#include "libvxmdsync/logversion.h"
#include <vitalnix/libvxutil/defines.h>

// Functions
static int pwl_build_tree(FILE *, struct HXbtree **);
static void pwlfmt_destruct(struct pwlfmt_workspace *);
static int pwlfmt_process2(struct pwlfmt_workspace *, const struct HXbtree *);

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int pwlfmt_new(struct pwlfmt_workspace *w)
{
    int esave, ret;

    if((w->vtable = vxcore_module_lookup("libvxmdfmt", w->style_name)) == NULL)
        return -PWLFMT_ENOSTYLE;
    if(w->vtable->require_template && w->template_file == NULL)
        return -PWLFMT_EREQTPL;

    w->input_fh      = NULL;
    w->output_fh     = NULL;
    w->style_data    = NULL;
    w->template_data = NULL;

    if(w->vtable->init != NULL && (ret = w->vtable->init(w)) <= 0)
        return ret;
    if((w->input_fh = fopen(w->input_file, "r")) == NULL) {
        esave = errno;
        ret   = -PWLFMT_EEINPUT;
        goto out;
    }
    if((w->output_fh = fopen(w->output_file, "w")) == NULL) {
        esave = errno;
        ret   = -PWLFMT_EEOUTPUT;
        goto out;
    }

    return 1;

 out:
    pwlfmt_destruct(w);
    errno = esave;
    return ret;
}

EXPORT_SYMBOL int pwlfmt_process(struct pwlfmt_workspace *w)
{
    struct HXbtree *bt = NULL;
    int esave = 0, ret;

    if((ret = pwl_build_tree(w->input_fh, &bt)) == 0) {
        ret = -PWLFMT_ENOUSERS;
        goto out;
    }
    if(ret < 0) {
        esave = errno;
        goto out;
    }

    w->num_entries = ret;
    ret = pwlfmt_process2(w, bt);

 out:
    pwlfmt_destruct(w);
    if(bt != NULL)
        HXbtree_free(bt);
    errno = esave;
    return ret;
}

EXPORT_SYMBOL const char *pwlfmt_strerror(int err)
{
#define B PWLFMT_ESUCCESS
    static const char *const errlist[] = {
        [PWLFMT_ESUCCESS  - B] = "Success",
        [PWLFMT_ENOSTYLE  - B] = "Style not found",
        [PWLFMT_EREQTPL   - B] = "Style requires template",
        [PWLFMT_EEINPUT   - B] = "Could not open input file",
        [PWLFMT_EEOUTPUT  - B] = "Could not open output file",
        [PWLFMT_EINPUTVER - B] = "Version of input file not supported",
        [PWLFMT_ENOUSERS  - B] = "No users found in logfile. "
                                 "(Wrong file and/or format?)",
    };
#undef B
    if(err < 0)
        err = -err;
    err -= PWLFMT_ESUCCESS;
    if(err >= ARRAY_SIZE(errlist))
        return NULL;
    return errlist[err];
}

EXPORT_SYMBOL const struct pwlstyle_vtable *pwlstyles_trav(void **trav_pptr)
{
    return vxcore_section_trav(trav_pptr, "libvxmdfmt");
}

//-----------------------------------------------------------------------------
/*  pwl_build_tree -
    @fh:         stdio filehandle to read from
    @b_ret:      pointer to storage point for resulting B-tree

    Reads the MDSYNC log from @fh and puts new users into a two-level
    pvgrp-keyed B-tree, sorting them as a wanted side-effect. The pointer to
    the B-tree is put into @*b_ret.

    Returns the number of entries put into the structures.
*/
static int pwl_build_tree(FILE *fh, struct HXbtree **b_ret) {
    unsigned int seen_format_id = 0;
    struct HXbtree *bt;
    hmc_t *ln = NULL;
    int ret = 0;

    bt = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_CMPFN, compare_wbc);
    if(bt == NULL)
        return -errno;

    while(HX_getl(&ln, fh) != NULL) {
        char *bufp = ln, *c_type, *c_pvgrp;
        struct HXbtree *sgp;

        HX_chomp(ln);
        if(*ln == '\0')
            continue;
        if(*ln == '#') {
            if(!seen_format_id && strncmp(ln, "# $logformat", 12) == 0) {
                if(strcmp(ln + 12, " " MDSYNC_LOG_VER) != 0) {
                    fprintf(stderr, "Input file is not of version \""
                            MDSYNC_LOG_VER "\"\n");
                    HXbtree_free(bt);
                    return -PWLFMT_EINPUTVER;
                }
                ++seen_format_id;
            }
            continue;
        }

        c_type  = HX_strsep(&bufp, ":");
        c_pvgrp = HX_strsep(&bufp, ":");
        if(*c_type != '+' || c_pvgrp == NULL || bufp == NULL)
            continue;

        // Only print users which were added ('+')
        if((sgp = HXbtree_get(bt, c_pvgrp)) == NULL) {
            sgp = HXbtree_init(HXBT_CDATA | HXBT_CMPFN, strcoll);
            HXbtree_add(bt, c_pvgrp, sgp);
        }
        HXbtree_add(sgp, bufp);
        ++ret;
    }

    *b_ret = bt;
    return ret;
}

/*  pwlfmt_destruct - Destroy the workspace
    @w:  Workspace to free

    Calls the destructor function of the output style and frees any resources
    used by the workspace.
*/
static void pwlfmt_destruct(struct pwlfmt_workspace *w) {
    if(w->vtable->exit != NULL)  w->vtable->exit(w);
    if(w->input_fh != NULL)      fclose(w->input_fh);
    if(w->output_fh != NULL)     fclose(w->output_fh);
    if(w->template_data != NULL) free(w->template_data);
    return;
}

/*  pwlfmt_process2 - Print tree
    @w:          Workspace to operate on
    @tl_tree:    Tree to print out

    Applies the B-tree (previously generated by pwl_build_tree()) to the
    style's functions.
*/
static int pwlfmt_process2(struct pwlfmt_workspace *w,
  const struct HXbtree *tl_tree)
{
    const struct HXbtree_node *tl_node;
    struct pwl_data data;
    void *tl_trav;
    int proc = 0;

    if((tl_trav = HXbtrav_init(tl_tree, NULL)) == NULL)
        return -errno;

    if(w->vtable->file_header != NULL) {
        memset(&data, 0, sizeof(data));
        w->vtable->file_header(w, &data);
    }

    while((tl_node = HXbtraverse(tl_trav)) != NULL) {
        struct HXbtree *pg_tree = tl_node->data;
        const char *pg_name = tl_node->key;
        void *pg_trav = HXbtrav_init(pg_tree, NULL);
        struct HXbtree_node *pg_node;

        if(w->vtable->tbl_header != NULL) {
            memset(&data, 0, sizeof(data));
            data.pvgrp = pg_name;
            w->vtable->tbl_header(w, &data);
        }

        while((pg_node = HXbtraverse(pg_trav)) != NULL) {
            char *field[5], *sd = pg_node->key;

            if(HX_split5(sd, ":", ARRAY_SIZE(field), field) != 4) {
                fprintf(stderr, "%s: Error in logfile and/or tree, during "
                  "group \"%s\"\n", __PRETTY_FUNCTION__, pg_name);
                continue;
            }

            memset(&data, 0, sizeof(data));
            data.pvgrp      = pg_name;
            data.first_name = field[1];
            data.surname    = field[0];
            data.username   = field[2];
            data.password   = field[3];
            w->vtable->tbl_entry(w, &data);

            if(w->report != NULL)
                w->report(w, ++proc, w->num_entries);
        }

        if(w->vtable->tbl_footer != NULL) {
            memset(&data, 0, sizeof(data));
            data.pvgrp = pg_name;
            w->vtable->tbl_footer(w, &data);
        }

        HXbtrav_free(pg_trav);
        HXbtree_free(pg_tree);
    }

    if(w->vtable->file_footer != NULL) {
        memset(&data, 0, sizeof(data));
        w->vtable->file_footer(w, &data);
    }
    HXbtrav_free(tl_trav);
    return 1;
}

//=============================================================================
