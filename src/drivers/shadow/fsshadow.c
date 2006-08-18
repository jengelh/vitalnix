/*=============================================================================
Vitalnix User Management Suite
drivers/shadow/fsshadow.c
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
#include "drivers/shadow/shadow.h"
#include "libvxpdb/libvxpdb.h"
#include "libvxutil/defines.h"

//-----------------------------------------------------------------------------
/*  db_read_shadow
    @fp:        stdio filehandle to read from
    @dq:        state->dq_user

    Updates the user list with bits from the shadow file.
*/
void db_read_shadow(FILE *fp, struct HXdeque *dq) {
    struct vxpdb_user *u = NULL;
    char *ln = NULL;

    if(dq->itemcount == 0) // No uses in @dq, so no need to read shadow.
        return;

    while(HX_getl(&ln, fp) != NULL) {
        char *data[9];

        if(*ln == '#')
            continue;

        HX_chomp(ln);
        memset(data, 0, sizeof(data));
        HX_split5(ln, ":", ARRAY_SIZE(data), data);
        if(*data[0] == '\0' || (u = lookup_user(dq, data[0], PDB_NOUID)) == NULL)
            continue; // orphaned entry

        u->sp_passwd  = HX_strdup(data[1]);
        u->sp_lastchg = strtol(data[2], NULL, 0);
        u->sp_min     = strtol(data[3], NULL, 0);
        u->sp_max     = strtol(data[4], NULL, 0);
        u->sp_warn    = strtol(data[5], NULL, 0);

        if(data[6] == NULL || *data[6] == '\0')
            u->sp_expire = strtol(data[6], NULL, 0);
        if(data[7] == NULL || *data[7] == '\0')
            u->sp_inact = strtol(data[7], NULL, 0);
        static_cast(char **, u->be_priv)[1] = HX_strdup(data[8]);
    }

    hmc_free(ln);
    return;
}

void db_write_shadow(FILE *fp, const struct vxpdb_user *u) {
    const char *password = (u->sp_passwd != NULL) ? u->sp_passwd : "!";
    const char **priv = u->be_priv;

    fprintf(fp, "%s:%s:%ld:%ld:%ld:%ld:", u->pw_name, password,
            u->sp_lastchg, u->sp_min, u->sp_max, u->sp_warn);

    if(u->sp_expire != PDB_NO_EXPIRE)   fprintf(fp, "%ld", u->sp_expire);
    fprintf(fp, ":");
    if(u->sp_inact != PDB_NO_INACTIVE)  fprintf(fp, "%ld", u->sp_inact);
    fprintf(fp, ":");
    if(priv != NULL && priv[1] != NULL) fprintf(fp, "%s", priv[1]);
    fprintf(fp, "\n");
    return;
}

//=============================================================================
