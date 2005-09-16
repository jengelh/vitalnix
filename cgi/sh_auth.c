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
#include <security/pam_appl.h>
#include <libHX.h>
#include "cgi/sh_auth.h"

static int my_conv(int, const struct pam_message **,
  struct pam_response **, void *);

//-----------------------------------------------------------------------------
int authenticate(const char *user, const char *pw) {
    const struct pam_conv conv = {
        .conv        = my_conv,
        .appdata_ptr = (void *)pw,
    };
    pam_handle_t *ph;
    int eax;

    if((eax = pam_start("passwd", user, &conv, &ph)) != PAM_SUCCESS) {
        return eax;
    }

    eax = pam_authenticate(ph, 0);
    pam_end(ph, PAM_SUCCESS);
    return eax == PAM_SUCCESS;
}

static int my_conv(int num_msg, const struct pam_message **msgp,
 struct pam_response **resp, void *ptr)
{
    int j;
    for(j = 0; j < num_msg; ++j) {
        const struct pam_message *this = msgp[j];
        if(strcmp(this->msg, "Password: ") == 0) {
            resp[j] = malloc(sizeof(struct pam_response));
            resp[j]->resp = HX_strdup(ptr);
            resp[j]->resp_retcode = 0;
        }
    }
    return PAM_SUCCESS;
}

//=============================================================================
