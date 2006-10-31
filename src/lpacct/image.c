/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.
*/
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libHX.h>
#include "acct.h"
#include "drop.h"
#include "global.h"
#include "image.h"
#include "util.h"

// Definitions
struct mapping {
    void *addr;
    off_t size;
};

// Functions
static int image_map(const char *, struct mapping *, struct image *);
static int image_get_header(FILE *, struct image *);
static inline double px_to_cm(unsigned int, unsigned int);
static inline double px_to_in(unsigned int, unsigned int);

//-----------------------------------------------------------------------------
/*  proc_image
    @file:      File to analyze

    Account for the file.
*/
int proc_image(const struct options *op)
{
    struct mapping mapping;
    struct image image;
    struct cost cost;
    struct costf sqcm, sqm, sqin, bl;

    if(!image_map(op->filename, &mapping, &image))
        return 0;

    pixel_cost[op->colorspace](&image, &cost);
    munmap(mapping.addr, mapping.size);

    drop2sqcm(&sqcm, &cost, op->dpi);
    drop2sqm(&sqm, &cost, op->dpi);
    drop2sqin(&sqin, &cost, op->dpi);
    drop2bl(&bl, &cost, op->dpi);

    if(op->verbose) {
        if(op->unit_metric) {
            double w = px_to_cm(image.width, op->dpi),
                   h = px_to_cm(image.height, op->dpi);
            printf("%lu x %lu px @ %u dpi = %.2f x %.2f cm (%.2f cm²)\n",
                image.width, image.height, op->dpi,
                w, h, w * h);
        }
        if(op->unit_i_sqin) {
            double w = px_to_in(image.width, op->dpi),
                   h = px_to_in(image.height, op->dpi);
            printf("%lu x %lu px @ %u dpi = %.2f x %.2f in (%.2f in²)\n",
                image.width, image.height, op->dpi,
                w, h, w * h);
        }
        if(op->unit_metric)
            printf("\t(1 ISO A4-Blatt Full Intensity = 1*0.06237 i*m²)\n");

#define COLOR(x) \
        if(op->unit_droplet) printf("  %9lld d", cost.x); \
        if(op->unit_i_sqcm)  printf("  %6.2f i*cm²", sqcm.x); \
        if(op->unit_i_sqm)   printf("  %6.5f i*m²", sqm.x); \
        if(op->unit_i_sqin)  printf("  %6.2f i*in²", sqin.x); \
        if(op->unit_a4)      printf("  %6.4f A4", bl.x); \
        printf("\n");

        printf("Cyan   "); COLOR(c);
        printf("Magenta"); COLOR(m);
        printf("Yellow "); COLOR(y);
        printf("Black  "); COLOR(k);
        printf("----------------------------------------"
               "---------------------------------------\n");
        printf("Total  "); COLOR(t);
#undef COLOR
    }

    // ->do_account is only set when called as a CUPS filter.
    if(op->do_account) {
        acct_syslog(op, &sqm);
        acct_mysql(op, &sqm);
    }

    return 1;
}

//-----------------------------------------------------------------------------
/*  image_map
    @file:      File to map
    @mapping:   Store point for mapping parameters
    @image:     Store point for image parameters

    Maps @file, fills in the mapping parameters into @mapping (needed for
    unmap) and fills in @image with image parameters such as width, height and
    the actual pixels (being a pointer to the mapped space).
*/
static int image_map(const char *file, struct mapping *mapping,
  struct image *image)
{
    struct stat sb;
    FILE *fp;

    if((fp = fopen(file, "rb")) == NULL) {
        fprintf(stderr, PREFIX "image_map: fopen() failed with %d\n", errno);
        return 0;
    }
    if(fstat(fileno(fp), &sb) != 0) {
        fprintf(stderr, PREFIX "proc_image: fstat() failed with %d\n", errno);
        fclose(fp);
        return 0;
    }

    mapping->size = sb.st_size;
    if(!image_get_header(fp, image)) {
        fclose(fp);
        return 0;
    }

    if(image->type != FILETYPE_PGM && image->type != FILETYPE_PPM) {
        fprintf(stderr, PREFIX "pxm_get_handler: Could not identify image\n");
        fclose(fp);
        return 0;
    }

    if((mapping->addr = mmap(NULL, mapping->size, PROT_READ, MAP_SHARED,
      fileno(fp), 0)) == MMAP_FAILED)
    {
        fprintf(stderr, PREFIX "proc_image: mmap() failed with %d\n", errno);
        fclose(fp);
        return 0;
    }

    image->data   = mapping->addr + ftell(fp);
    image->pixels = image->width * image->height;

    fclose(fp);
    return 1;
}

//-----------------------------------------------------------------------------
/*  image_get_header
    @fp:        file handle to read from
    @image:     store point for image info

    Reads the header of a PGM/PPM file, fills in @image.
*/
static int image_get_header(FILE *fp, struct image *image)
{
    const char *p;
    char buf[80];

    image->type = FILETYPE_NONE;

    /*
     *  Read filetype
     */
    while((p = fgets(buf, sizeof(buf), fp)) != NULL) {
        HX_chomp(buf);
        if(strcmp(buf, "P5") == 0) {
            image->type = FILETYPE_PGM;
            break;
        }
        if(strcmp(buf, "P6") == 0) {
            image->type = FILETYPE_PPM;
            break;
        }
    }
    if(p == NULL) // EOF before P5/P6 found
        return 0;

    /*
     *  Skip comments.
     */
    while((p = fgets(buf, sizeof(buf), fp)) != NULL) {
        HX_chomp(buf);
        if(*buf != '#')
            break;
    }
    if(p == NULL) // EOF before size header
        return 0;

    /*
     *  Image size
     */
    sscanf(buf, "%ld %ld", &image->width, &image->height);
    p = fgets(buf, sizeof(buf), fp);
    if(p == NULL) // EOF before color header
        return 0;

    return image->type != FILETYPE_NONE;
}

//-----------------------------------------------------------------------------
static inline double px_to_cm(unsigned int p, unsigned int dpi)
{
    return 2.54 * p / dpi;
}

static inline double px_to_in(unsigned int p, unsigned int dpi)
{
    return (double)p / dpi;
}

//=============================================================================
