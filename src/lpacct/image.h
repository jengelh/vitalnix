#ifndef LPACCT_IMAGE_H
#define LPACCT_IMAGE_H 1

#include <sys/types.h>

enum imagetype {
    FILETYPE_NONE = 0,
    FILETYPE_PGM,
    FILETYPE_PPM,
};

struct options;

struct image {
    enum imagetype type;
    unsigned char *data;
    unsigned long width, height, pixels;
};

/*
 *      FUNCTIONS
 */
extern int mpxm_process(int, const struct options *);

#endif // LPACCT_IMAGE_H
