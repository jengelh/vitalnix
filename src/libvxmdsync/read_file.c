/*=============================================================================
Vitalnix User Management Suite
libvxmdsync/read_file.c
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
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include "libvxmdsync/internal.h" // MAX_LNAME
#include <vitalnix/libvxmdsync/libvxmdsync.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

//-----------------------------------------------------------------------------
EXPORT_SYMBOL int mdsync_read_file(struct mdsync_workspace *w,
  const char *input_dsc, const char *input_fmt)
{
    struct vxeds_entry *entry;
    char username[MAX_LNAME+1];
    void *state;
    int ret;

    if((ret = vxeds_open(input_dsc, input_fmt, &state)) <= 0)
        return ret;

    while(1) {
        entry = malloc(sizeof(struct vxeds_entry));

        if((ret = vxeds_read(state, entry)) < 0) {
            if(ret == -EINVAL)
                fprintf(stderr, "%s: Warning: Stumbled upon a bogus entry in"
                        " Data Source, stopping.\n", __FUNCTION__);
            else
                fprintf(stderr, "%s: eds_read() returned %d (%s)\n",
                        __FUNCTION__, ret, strerror(-ret));
            vxeds_free_entry(entry);
            free(entry);
        } else if(ret == 0) {
            // End of list
            free(entry);
            break;
        }

        entry->username = HX_strdup(vxutil_propose_lname(username,
                          sizeof(username), entry->surname, entry->first_name));
        HXbtree_add(w->add_req, entry->uuid, entry);
    }

    vxeds_close(state);
    return 1;
}

//=============================================================================
