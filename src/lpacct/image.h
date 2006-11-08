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
    unsigned long nr_pixels;
    unsigned long long nr_bytes;

    unsigned long long rem_bytes;
    unsigned char *buffer;
    unsigned int buffer_size; // bytes
};

/*
 *      FUNCTIONS
 */
extern long mpxm_chunk_next(int, struct image *);
extern int mpxm_process(int, const struct options *);

#endif // LPACCT_IMAGE_H
