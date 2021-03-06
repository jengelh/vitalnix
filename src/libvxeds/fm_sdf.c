/*
 *	libvxeds/fm_sdf.c - SDF parser
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2002 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/defs.h>
#include <libHX/string.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include "libvxeds/static-build.h"
#include <vitalnix/libvxeds/vtable.h>
#include <vitalnix/libvxutil/libvxutil.h>
#define ICONV_NULL reinterpret_cast(iconv_t, -1)

/* Definitions */
struct sdf_state {
	char *input_file;
	FILE *input_fh;
	int current_line;
	hxmc_t *mc;
	iconv_t conv_fd;
};

/* Functions */
static char *convert(iconv_t, char *);

//-----------------------------------------------------------------------------
static int sdf_open(const char *filename, void **state_pptr)
{
	struct sdf_state *state;

	if ((state = *state_pptr = calloc(1, sizeof(struct sdf_state))) == NULL)
		return -errno;

	if ((state->conv_fd = iconv_open("utf-8", "cp437")) == ICONV_NULL ||
	    (state->input_fh = fopen(filename, "r")) == NULL) {
		free(state);
		return -errno;
	}

	state->input_file = HX_strdup(filename);
	state->mc         = NULL;
	return 1;
}

static int sdf_read(void *state_ptr, struct vxeds_entry *e)
{
	struct sdf_state *state = state_ptr;
	char *data[6], *line;
	int num_fields;

	if (HX_getl(&state->mc, state->input_fh) == NULL)
		return 0;
	++state->current_line;
	HX_chomp(state->mc);

	if (*state->mc == '#' || strncmp(state->mc, "(*", 2) == 0 || /* *) */
	    *state->mc == '\0')
		/* Skip comment lines */
		return sdf_read(state_ptr, e);

	line = convert(state->conv_fd, state->mc);

	if ((num_fields = HX_split5(line, ";", ARRAY_SIZE(data), data)) < 3) {
		fprintf(stderr, "%s: SDF-4 format: Line %d has less than three "
		        " fields, skipping!\n", __func__, state->current_line);
		free(line);
		return sdf_read(state_ptr, e);
	}

	if (vxutil_only_digits(data[0])) {
		/*
		 * SDF style from Max-Plank-Gymnasium has another
		 * (though, unimportant) field at the front.
		 */
		if (--num_fields < 3) {
			fprintf(stderr, "%s: SDF-5 format: Line %d has less "
			        "than 4 fields, skipping!\n",
			        __func__, state->current_line);
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
	e->uuid       = vxuuid_vx3(e->full_name, vxutil_string_xday(data[2]));

	/*
	 * In Vitalnix2, the birthdate was used as UUID, and the <Full Name,
	 * UUID> _tuple_ was unique among all users. In Vitalnix3, the UUID is
	 * generated from the full name and the birthdate, so that the UUID
	 * _itself_ is unique and can therefore be used as a key.
	 */

	free(line);
	return 1;
}

static void sdf_close(void *state_ptr)
{
	struct sdf_state *state = state_ptr;
	HXmc_free(state->mc);
	if (state->conv_fd != ICONV_NULL)
		iconv_close(state->conv_fd);
	if (state->input_fh != NULL)
		fclose(state->input_fh);
	HX_strclone(&state->input_file, NULL);
	free(state_ptr);
}

//-----------------------------------------------------------------------------
static char *convert(iconv_t cd, char *in)
{
	char *inp = in, *out, *outp;
	size_t inl, outl;

	errno = 0;
	if (in == NULL || *in == '\0')
		return NULL;
	inl  = strlen(inp) + 1;
	outl = 3 * inl;
	if ((outp = out = malloc(outl)) == NULL)
		return NULL;
	iconv(cd, &inp, &inl, &outp, &outl);
	return out;
}

//-----------------------------------------------------------------------------
static const struct edsformat_vtable THIS_FORMAT = {
	.desc   = "Kolleg SDF",
	.ext    = "sdf",
	.open   = sdf_open,
	.read   = sdf_read,
	.close  = sdf_close,
};

REGISTER_MODULE(sdf, &THIS_FORMAT);
