/*=============================================================================
Vitalnix User Management Suite
clutils/mdsync.c
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
#include <time.h>
#include <libHX.h>
#include "vitalnix-config.h"
#include "libvxeds/libvxeds.h"
#include "libvxmdsync/libvxmdsync.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"

#define NOTICE_REDO     "Error during operation. You might need to redo " \
                        "the Synchronization process\n" \
                        "to complete all jobs.\n"
// Structures
struct private_info {
    char *backend_module, *group_name, *input_file, *input_fmt, *output_file;
    int no_add, no_update, no_delete, interactive;

    int open_status;
    struct vxpdb_state *module_handle;
    struct mdsync_workspace *mdsw;
    time_t last_print;
};

// Functions
static int sync_init(struct private_info *);
static int sync_run(struct private_info *);
static int sync_add(struct private_info *);
static int sync_mod(struct private_info *);
static int sync_del(struct private_info *);
static void sync_cleanup(struct private_info *);

static void cb_report(unsigned int, const struct mdsync_workspace *,
    unsigned long, unsigned long);
static void print_compare_input(const struct mdsync_workspace *);
static void print_compare_output(const struct mdsync_workspace *);

static int get_options(int *, const char ***, struct private_info *);
static void show_version(const struct HXoptcb *);
static int time_limit(time_t *, time_t);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct private_info priv;
    int ret = EXIT_SUCCESS;

    memset(&priv, 0, sizeof(priv));
    priv.backend_module = HX_strdup("*");

    if(!get_options(&argc, &argv, &priv))
        return EXIT_FAILURE;
    if(!sync_init(&priv) || !sync_run(&priv))
        ret = EXIT_FAILURE;

    sync_cleanup(&priv);
    return ret;
}

static int sync_init(struct private_info *priv) {
    struct mdsync_workspace *mdsw;
    int ret;

    if(priv->group_name == NULL || priv->input_file == NULL ||
     priv->output_file == NULL) {
        fprintf(stderr, "-g, -i and -o options are required.\n");
        return 0;
    }

    if(strcmp(priv->output_file, "-") == 0)
        HX_strclone(&priv->output_file, "/dev/stdout");

    if((priv->module_handle = vxpdb_load(priv->backend_module)) == NULL) {
        fprintf(stderr, "Could not load PDB back-end: %s\n", strerror(errno));
        return 0;
    }

    if((ret = vxpdb_open(priv->module_handle, PDB_WRLOCK)) <= 0) {
        fprintf(stderr, "Could not open PDB back-end: %s\n", strerror(-ret));
        return 0;
    }

    priv->open_status = 1;

    if((priv->mdsw = mdsw = mdsync_init()) == NULL) {
        perror("mdsync_init()");
        return 0;
    }

    mdsw->database     = priv->module_handle;
    mdsw->report       = cb_report;
    mdsw->user_private = priv;
    return 1;
}

static int sync_run(struct private_info *priv) {
    struct mdsync_workspace *mdsw = priv->mdsw;
    const char *fmt;
    int ret;

    if((ret = mdsync_prepare_group(mdsw, priv->group_name)) < 0) {
        fprintf(stderr, "Error querying the PDB: %s\n", strerror(-ret));
        return 0;
    } else if(ret == 0) {
        fprintf(stderr, "Group \"%s\" does not exist\n", priv->group_name);
        return 0;
    }

    if((fmt = priv->input_fmt) == NULL &&
     (fmt = vxeds_derivefromname(priv->input_file)) == NULL) {
        fprintf(stderr, "Could not determine file type of input data source\n");
        return 0;
    }

    if((ret = mdsync_read_file(mdsw, priv->input_file, fmt)) <= 0) {
        fprintf(stderr, "Error while reading Data Source: %s\n",
                strerror(-ret));
        return 0;
    }

    if((ret = mdsync_open_log(mdsw, priv->output_file)) <= 0) {
        fprintf(stderr, "Error trying to open logfile: %s\n", strerror(-ret));
        return 0;
    }

    if(mdsw->config.add_opts.defaults.vs_defer > 0)
        printf(
          "Deferred Deletion feature enabled (%ld day(s)).\n"
          "Note that the deletion count therefore may be lower"
          "than expected, which is normal.\n",
          mdsw->config.add_opts.defaults.vs_defer);

    print_compare_input(mdsw);
    mdsync_compare(mdsw);
    print_compare_output(mdsw);

    mdsync_fixup(mdsw);

    return sync_add(priv) && sync_mod(priv) && sync_del(priv);
}

static int sync_add(struct private_info *priv) {
    int ret;
    if(priv->mdsw->add_req->itemcount == 0) {
        printf("No new users to add.\n");
        return 1;
    }
    if(priv->no_add) {
        printf("Not adding any users due to request (command-line).\n");
        return 1;
    }
    if((ret = mdsync_add(priv->mdsw)) <= 0) {
        printf("mdsync_add(): %s\n" NOTICE_REDO, strerror(-ret));
        return 0;
    }
    printf("Successfully added %ld users\n", priv->mdsw->add_req->itemcount);
    return 1;
}

static int sync_mod(struct private_info *priv) {
    struct mdsync_workspace *mdsw = priv->mdsw;
    long total = mdsw->defer_start->itemcount + mdsw->defer_stop->itemcount;
    int ret;

    if(total == 0) {
        printf("No deferred deletion timers to adjust.\n");
        return 1;
    }
    if(priv->no_update) {
        printf("Not modifying deferred deletion timers due to request"
               " (command-line).\n");
        return 1;
    }
    if((ret = mdsync_mod(priv->mdsw)) <= 0) {
        printf("mdsync_mod(): %s\n" NOTICE_REDO, strerror(-ret));
        return 0;
    }
    printf("Successfully modified %ld timers\n", total);
    return 1;
}

static int sync_del(struct private_info *priv) {
    int ret;

    if(priv->mdsw->delete_now->itemcount == 0) {
        printf("No old users to delete.\n");
        return 1;
    }
    if(priv->no_delete) {
        printf("Not deleting any users due to request (command-line).\n");
        return 1;
    }
    if((ret = mdsync_del(priv->mdsw)) <= 0) {
        printf("mdsync_del(): %s\n" NOTICE_REDO, strerror(-ret));
        return 0;
    }
    printf("Successfully deleted %ld users\n",
           priv->mdsw->delete_now->itemcount);
    return 1;
}

static void sync_cleanup(struct private_info *priv) {
    if(priv->mdsw != NULL)
        mdsync_free(priv->mdsw);
    if(priv->module_handle != NULL) {
        if(priv->open_status)
            vxpdb_close(priv->module_handle);
        vxpdb_unload(priv->module_handle);
    }
    free(priv->backend_module);
    free(priv->group_name);
    free(priv->input_file);
    free(priv->input_fmt);
    free(priv->output_file);
    return;
}

//-----------------------------------------------------------------------------
static void cb_report(unsigned int type, const struct mdsync_workspace *mdsw,
 unsigned long current, unsigned long max)
{
    static const char *const fmt[] = {
        [MDREP_ADD]     = "Add process",
        [MDREP_UPDATE]  = "Update process",
        [MDREP_DSTART]  = "Starting DDTs",
        [MDREP_DSTOP]   = "Stopping DDTs",
        [MDREP_DELETE]  = "Deletion process",
        [MDREP_COMPARE] = "Compare process",
        [MDREP_FIXUP]   = "Fixup process",
    };
    struct private_info *priv = mdsw->user_private;
    double pct;

    if(current == 1)
        priv->last_print = 0;
    if(!time_limit(&priv->last_print, 1) && current != max)
        return;

    printf("%s", fmt[type]);
    pct = current * 100.0 / max;
    if(type == MDREP_COMPARE || type == MDREP_FIXUP)
        printf(": %.2f%%\n", pct);
    else
        printf(": %ld/%ld users (%.2f%%)\n", current, max, pct);
    return;
}

static void print_compare_input(const struct mdsync_workspace *mdsw) {
    printf("Comparing EDS to PDB\n");
    printf("%ld user(s) in EDS list\n", mdsw->add_req->itemcount);
    return;
}

static void print_compare_output(const struct mdsync_workspace *mdsw) {
    printf("%lu group member(s) found in PDB\n", mdsw->num_grp);
    printf("    %lu to keep and update group descriptors\n", mdsw->update_req->itemcount);
    printf("    %lu to start deferred deletion timer\n", mdsw->defer_start->itemcount);
    printf("    %lu to wait for deletion\n", mdsw->defer_wait->itemcount);
    printf("    %lu to stop deferred deletion timer\n", mdsw->defer_stop->itemcount);
    printf("    %lu to delete\n", mdsw->delete_now->itemcount);
    printf("%lu new users to add\n", mdsw->add_req->itemcount);
    return;
}

//-----------------------------------------------------------------------------
static int get_options(int *argc, const char ***argv, struct private_info *p) {
    struct HXoption options_table[] = {
        {.sh = 'M', .type = HXTYPE_STRING, .ptr = &p->backend_module,
         .help = "Backend module", .htyp = "NAME"},
        {.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
         .help = "Show version information"},
        {.sh = 'Y', .type = HXTYPE_NONE, .ptr = &p->interactive,
         .help = "Ask before performing each operation"},
        {.sh = 'g', .type = HXTYPE_STRING, .ptr = &p->group_name,
         .help = "System group to synchronize against", .htyp = "NAME"},
        {.sh = 'i', .type = HXTYPE_STRING, .ptr = &p->input_file,
         .help = "External Data Source", .htyp = "FILE"},
        {.sh = 't', .type = HXTYPE_STRING, .ptr = &p->input_fmt,
         .help = "EDS type", .htyp = "TYPE"},
        {.sh = 'o', .type = HXTYPE_STRING, .ptr = &p->output_file,
         .help = "Output log file (for -S)", .htyp = "FILE"},
        {.ln = "no-add", .type = HXTYPE_NONE, .ptr = &p->no_add,
         .help = "Do not add any users"},
        {.ln = "no-del", .type = HXTYPE_NONE, .ptr = &p->no_delete,
         .help = "Do not delete any users"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi) {
    printf("Vitalnix " VITALNIX_VERSION " mdsync\n");
    exit(EXIT_SUCCESS);
}

static int time_limit(time_t *last, time_t interval) {
    time_t now = time(NULL);
    if(now - *last < interval)
        return 0;
    *last = now;
    return 1;
}

//=============================================================================
