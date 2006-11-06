/*=============================================================================
Vitalnix User Management Suite
drivers/shadow/fspasswd.c - Shadow driver
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2002 - 2006
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include "drivers/shadow/shadow.h"
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxutil/defines.h>

//-----------------------------------------------------------------------------
/*  db_read_passwd
    @fp:        stdio filehandle to read from

    In this function, we read all users into memory. This speeds processing up,
    as seeking and scanning in userinfo() / groupinfo() would take long
    otherwise. We also have it handy in a struct. Doing so also allows us to
    commit all changes to the user database at once (the usual case with no
    PDB_SYNC flag set).
*/
struct HXdeque *db_read_passwd(FILE *fp) {
    struct HXdeque *dq;
    struct vxpdb_user *u;
    char *ln = NULL;

    if((dq = HXdeque_init()) == NULL)
        return NULL;

    while(HX_getl(&ln, fp) != NULL) {
        char *data[8];

        if(*ln == '#')
            continue;

        HX_chomp(ln);
        memset(data, 0, sizeof(data));
        if(HX_split5(ln, ":", ARRAY_SIZE(data), data) < 4) {
            fprintf(stderr, "vxdb_shadow: bogus line in passwd file\n");
            continue;
        }

        if((u = malloc(sizeof(struct vxpdb_user))) == NULL)
            break;

        vxpdb_user_clean(u);
        u->pw_name  = HX_strdup(data[0]);
        u->pw_uid   = strtol(data[2], NULL, 0);
        u->pw_gid   = strtol(data[3], NULL, 0);
        u->pw_real  = HX_strdup(data[4]);
        u->pw_home  = HX_strdup(data[5]);
        u->pw_shell = HX_strdup(data[6]);

        /* Anything after the seven main fields is "reserved for future use",
        (read: or private use), so we need to keep that intact. */
        u->be_priv = calloc(1, sizeof(void *) * 2);
        static_cast(char **, u->be_priv)[0] = HX_strdup(data[7]);

        /* In case there is no shadow entry, we need some recovery values.
        This is already done by vxpdb_clean_user(). */

        /* Stuff is pushed in a linked list. The list is unordered, so it has
        the same order as /etc/passwd. This is intentional (and suffices). */
        HXdeque_push(dq, u);
    }

    hmc_free(ln);
    return dq;
}

void db_write_passwd(FILE *fp, const struct vxpdb_user *u) {
    const char **priv = u->be_priv;

    fprintf(fp, "%s:x:%ld:%ld:%s:%s:%s",
      u->pw_name, u->pw_uid, u->pw_gid, u->pw_real, u->pw_home, u->pw_shell);
    if(priv != NULL && priv[0] != NULL)
        fprintf(fp, "%s", priv[0]);
    fprintf(fp, "\n");
    return;
}

//=============================================================================
