/*=============================================================================
Vitalnix User Management Suite
  Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2003 - 2005
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libHX.h>
#include "include/accdb.h"
#include "cui/autorun.h"
#include "cui/data.h"
#include "cui/global.h"
#include "cui/main.h"
#include "cui/pwlist.h"
#include "cui/sync.h"
#include "cui/xml_in.h"

//-----------------------------------------------------------------------------
// Password list printing (-P)

void autorun_pwl(struct pwl_param *i) {
    if(i->style == NULL || *i->style == '\0' || i->in == NULL ||
     *i->in == '\0' || i->out == NULL || *i->out == '\0') {
        fprintf(stderr, "-i, -o and -s options are required for -P.\n");
        return;
    }

    pwl_proc(i);
    return;
}

//-----------------------------------------------------------------------------
// Synchronization (-S)

void autorun_sync(struct sync_param *i) {
    struct HXbtree *al = NULL, *lnlist = NULL;
    struct adb_group dgroup;
    struct HXdeque *dl = NULL;
    FILE *logp = NULL;
    char *outf;

    if(i->gname == NULL || *i->gname == '\0' || i->in == NULL ||
     *i->in == '\0') {
        fprintf(stderr, "-g and -i options are required for -S.\n");
        return;
    }

    if((outf = i->out) == NULL) { outf = ""; }
    if(B_OPEN(1) <= 0) { exit(EXIT_FAILURE); }
    if(sync_prep_dgrp(&dgroup, i->gname) <= 0 ||
     sync_read_file(i->in, &al, eds_derivefromname(i->in)) <= 0) {
        goto out;
    }

    if(Opt.default_times.defer_del > 0) {
        printf("Deferred Deletion feature enabled (%ld day(s)).\n",
         Opt.default_times.defer_del);
    }
    sync_compare(&dgroup, al, &dl, &lnlist);
    if(al->itemcount == 0 && dl->itemcount == 0) {
        printf("Already synchronized\n");
        goto out;
    }

    sync_fixup_addl(al, lnlist);
    sync_set_aflush();
    if(*outf != '\0' && (logp = sync_open_log(outf, 'w')) == NULL) {
        goto out;
    }

    if(al->itemcount > 0) {
        if(i->no_add) {
            printf("Not adding any users due to request (command-line).\n");
        } else if(sync_add(&dgroup, al, logp) <= 0) {
            printf("Error during operation. You might need to redo the"
             " Synchronization process\n" "to complete all jobs.\n");
            goto out2;
        }
    } else {
        printf("No new users to add.\n");
    }

    if(dl->itemcount > 0) {
        if(i->no_del) {
            printf("Not deleting any users due to request (command-line).\n");
        } else if(sync_del(dl, logp) <= 0) {
            printf("Error during operation. You might need to redo the"
             " Synchronization process\n" "to complete all jobs.\n");
            //goto out2;
        }
    } else {
        printf("No old users to delete.\n");
    }

 out2:
    B_MODCTL(ADB_FLUSH);

 out:
    if(logp != NULL) { fclose(logp); }
    sync_free(al, dl);
    B_CLOSE();
    return;
}

//-----------------------------------------------------------------------------
// Adding a single user (-U)

void autorun_user(struct user_param *i) {
    struct HXbtree *al, *lnlist;
    struct adb_group dgroup;
    struct eds_entry bucket;
    char buf[MAXSNLEN];

    if(i->gname == NULL || *i->gname == '\0' || i->vname == NULL ||
     *i->vname == '\0') {
        fprintf(stderr, "-g and -v options are required for -U.\n");
        return;
    }

    // i->nname can be empty

    if(B_OPEN(1) <= 0) { exit(EXIT_FAILURE); }
    if(!sync_prep_dgrp(&dgroup, i->gname)) { goto out; }

    bucket.vname  = i->vname;
    bucket.nname  = i->nname;
    bucket.sgroup = i->sgroup;
    bucket.xuid   = i->xuid;
    complete_pentry(&bucket, buf, MAXSNLEN);

    al = HXbtree_init(HXBT_MAP | HXBT_CKEY);
    HXbtree_add(al, buf, &bucket);

    sync_compare(&dgroup, al, NULL, &lnlist);
    sync_fixup_addl(al, lnlist);

    if(!sync_add(&dgroup, al, NULL)) {
        printf("Error during operation. You might need to redo the"
         " synchronization process\n" "to complete all jobs.\n");
    }

    B_MODCTL(ADB_FLUSH);
    /* We cannot call sync_free() as most of our data is present in the stack,
    which obviously cannot be freed. */
    free(bucket.lname);
    free(bucket.rname);
    HXbtree_free(al);

 out:
    B_CLOSE();
    return;
}

//=============================================================================
