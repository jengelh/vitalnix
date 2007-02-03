/*
    lpacct/image.h
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
*/
#ifndef LPACCT_IMAGE_H
#define LPACCT_IMAGE_H 1

#include <sys/types.h>

enum imagetype {
    FILETYPE_NONE = 0,
    FILETYPE_PGM,
    FILETYPE_PPM,
};

struct options;

struct image { // page
    enum imagetype type;
    unsigned long width, height;
    unsigned long long nr_pixels, nr_bytes;

    unsigned long long rem_bytes;
    void *buffer;
    unsigned int buffer_size; // bytes
};

/*
 *      FUNCTIONS
 */
extern void invert_image(struct image *);
extern long mpxm_chunk_next(int, struct image *);
extern int mpxm_process(int, const struct options *);

#endif // LPACCT_IMAGE_H
