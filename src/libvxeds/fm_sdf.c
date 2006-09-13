/*=============================================================================
Vitalnix User Management Suite
libvxeds/d_sdf.c - SDF parsing module
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
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "compiler.h"
#include "libvxeds/libvxeds.h"
#include "libvxeds/static-build.h"
#include "libvxeds/vtable.h"
#include "libvxutil/defines.h"
#include "libvxutil/libvxutil.h"
#define ICONV_NULL reinterpret_cast(iconv_t, -1)

// Structures
struct sdf_state {
    char *input_file;
    FILE *input_fh;
    int current_line;
    hmc_t *hmc;
    iconv_t conv_fd;
};

// Functions
static int sdf_open(const char *, void **);
static int sdf_read(void *, struct vxeds_entry *);
static void sdf_close(void *);
static char *convert(iconv_t, char *);

// Module
static const struct edsformat_vtable THIS_FORMAT = {
    .desc  = "Kolleg SDF",
    .ext   = "sdf",
    .open  = sdf_open,
    .read  = sdf_read,
    .close = sdf_close,
};

//-----------------------------------------------------------------------------
REGISTER_MODULE(sdf, &THIS_FORMAT);

static int sdf_open(const char *filename, void **state_pptr) {
    struct sdf_state *state;

    if((state = *state_pptr = calloc(1, sizeof(struct sdf_state))) == NULL)
        return -errno;

    if((state->conv_fd = iconv_open("utf-8", "cp437")) == ICONV_NULL ||
     (state->input_fh = fopen(filename, "r")) == NULL) {
        free(state);
        return -errno;
    }

    state->input_file = HX_strdup(filename);
    state->hmc        = NULL;
    return 1;
}

static int sdf_read(void *state_ptr, struct vxeds_entry *e) {
    struct sdf_state *state = state_ptr;
    char *data[5], *line;
    int num_fields;

    if(HX_getl(&state->hmc, state->input_fh) == NULL)
        return 0;
    ++state->current_line;
    HX_chomp(state->hmc);

    if(*state->hmc == '#' || strncmp(state->hmc, "(*", 2) == 0 ||  // *)
     *state->hmc == '\0')
        return sdf_read(state_ptr, e); // Skip comment line

    line = convert(state->conv_fd, state->hmc);

    if((num_fields = HX_split5(line, ";", ARRAY_SIZE(data), data)) < 3) {
        fprintf(stderr, "%s: SDF-4 format: Line %d has less than 3 fields,"
                " skipping!\n", __FUNCTION__, state->current_line);
        free(line);
        return sdf_read(state_ptr, e);
    }

    if(vxutil_only_digits(data[0])) {
        /* SDF style from Max-Plank-Gymnasium has another (though, unimportant)
        field at the front. */
        if(--num_fields < 3) {
            fprintf(stderr, "%s: SDF-5 format: Line %d has less than 4"
                    " fields, skipping!\n", __FUNCTION__, state->current_line);
            free(line);
            return sdf_read(state_ptr, e);
        }
        memmove(&data[0], &data[1], 4 * sizeof(char *));
    }

    memset(e, 0, sizeof(*e));
    e->first_name = HX_strdup(data[1]);
    e->surname    = HX_strdup(data[0]);
    e->full_name  = vxeds_bfullname(e->first_name, e->surname);
    e->pvgrp      = HX_strdup(data[3]);
    e->uuid       = vxuuid_vx3(e->full_name, vxutil_string_iday(data[2]));

    /* In Vitalnix2, the birthdate was used as UUID, and the <Full Name, UUID>
    _tuple_ was unique among all users. In Vitalnix3, the UUID is generated
    from the full name and the birthdate, so that the UUID _itself_ is unique
    and can therefore be used as a key. */

    free(line);
    return 1;
}

static void sdf_close(void *state_ptr) {
    struct sdf_state *state = state_ptr;
    hmc_free(state->hmc);
    if(state->conv_fd != ICONV_NULL)
        iconv_close(state->conv_fd);
    if(state->input_fh != NULL)
        fclose(state->input_fh);
    HX_strclone(&state->input_file, NULL);
    free(state_ptr);
    return;
}

//-----------------------------------------------------------------------------
static char *convert(iconv_t cd, char *in) {
    char *inp = in, *out, *outp;
    size_t inl, outl;

    errno = 0;
    if(in == NULL || *in == '\0')
        return NULL;
    inl  = strlen(inp) + 1;
    outl = 3 * inl;
    if((outp = out = malloc(outl)) == NULL)
        return NULL;
    iconv(cd, &inp, &inl, &outp, &outl);
    return out;
}

//=============================================================================
