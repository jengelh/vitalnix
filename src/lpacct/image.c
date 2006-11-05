/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.
*/
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libHX.h>
#include "acct.h"
#include "drop.h"
#include "global.h"
#include "image.h"
#include "util.h"

// Functions
static void cost_add(struct cost *, const struct cost *);
static int mpxm_fdgetl(int, hmc_t **);
static int mpxm_get_header(int, struct image *);
static int mpxm_get_image(int, struct image *);
static void print_stats(const struct options *, const struct cost *,
    const struct image *);
static inline double px_to_cm(unsigned int, unsigned int);
static inline double px_to_in(unsigned int, unsigned int);

//-----------------------------------------------------------------------------
/*  mpxm_process
    @fd:        file descriptor to read from
    @cost:      accounting structure

    Process all pages from @fd and write accounting results into @cost.
    Propagates the error from any subfunction to the caller.
*/
int mpxm_process(int fd, const struct options *op)
{
    struct image image;
    struct cost all_cost = {}, page_cost = {};
    int ret;

    while((ret = mpxm_get_header(fd, &image)) > 0 &&
      (ret = mpxm_get_image(fd, &image)) > 0)
    {
        pixel_cost[op->colorspace](&image, &page_cost);
        free(image.data);
        cost_add(&all_cost, &page_cost);
        ++all_cost.p;

        if(op->per_page_stats) {
            printf("Page %u:\n", all_cost.p);
            print_stats(op, &page_cost, &image);
        }
    }

    if(op->per_doc_stats) {
        printf("Total cost of all %u pages\n", all_cost.p);
        print_stats(op, &all_cost, NULL);
    }

    // ->do_account is only set when called as a CUPS filter.
/*
    if(ret > 0 && op->do_account) {
        acct_syslog(op, &sqm);
        acct_mysql(op, &sqm);
    }
*/

    return ret;
}

//-----------------------------------------------------------------------------
static void cost_add(struct cost *out, const struct cost *in)
{
    out->c += in->c;
    out->m += in->m;
    out->y += in->y;
    out->k += in->k;
    out->t += in->c + in->m + in->y + in->k;
    out->p += in->p;
    return;
}

/*  mpxm_fdgetl
    @fd:        file descriptor to read from
    @res:       buffer to put data into

    Reads a line from @fd and puts it into @res.
*/
static int mpxm_fdgetl(int fd, hmc_t **res)
{
    char temp[256], *temp_ptr = temp;
    int ret;

    if(*res == NULL) *res = hmc_sinit("");
    else             hmc_trunc(res, 0);

    while(1) {
        /* On read error or newline (make sure newlines makes
        it into @res), flush and return. */
        if((ret = read(fd, temp_ptr, 1)) <= 0 || *temp_ptr++ == '\n') {
            *temp_ptr = '\0';
            hmc_strcat(res, temp);
            break;
        }
        if(temp_ptr == temp + sizeof(temp) - 1) {
            *temp_ptr = '\0';
            hmc_strcat(res, temp);
            temp_ptr = temp;
        }
    }

    return (ret < 0) ? -errno : 0;
}

/*  mpxm_get_header
    @fd:        file descriptor to read from
    @image:     pointer to image parameters

    Reads the next PPM header from the stream @fd into @image. Returns true on
    success, otherwise false.
*/
static int mpxm_get_header(int fd, struct image *image)
{
    hmc_t *ln = NULL;
    int ret;

    if((ret = mpxm_fdgetl(fd, &ln)) < 0)
        return 0;
    HX_chomp(ln);

    if(strcmp(ln, "P6") == 0)
        image->type = FILETYPE_PPM;
    else if(strcmp(ln, "P3") == 0)
        image->type = FILETYPE_PGM;
    else
        return 0;

    // Some programs put a comment line in it
    do {
        if((ret = mpxm_fdgetl(fd, &ln)) < 0)
            return 0;
    } while(*ln == '#');

    if(sscanf(ln, "%lu %lu", &image->width, &image->height) != 2)
        return 0;

    // colorspace line (unused)
    mpxm_fdgetl(fd, &ln);
    hmc_free(ln);

    image->pixels = image->width * image->height;
    return 1;
}

/*  mpxm_get_image
    @fd:        file descriptor to read from
    @image:     image to fill

    Reads the pixel stream from @fd into @image->data. Returns true on success,
    otherwise false.
*/
static int mpxm_get_image(int fd, struct image *image)
{
    ssize_t bytes, ret;
    unsigned char *p;

    if(image->type == FILETYPE_PPM)
        p = image->data = malloc(bytes = 3 * image->pixels);
    else if(image->type == FILETYPE_PGM)
        p = image->data = malloc(bytes = image->pixels);
    else
        return -EINVAL;

    if(image->data == NULL)
        return -errno;
    while((ret = read(fd, p, bytes)) > 0) {
        bytes -= ret;
        p     += ret;
    }
    if(bytes > 0) {
        fprintf(stderr, "%s: Did not read enough, %zd left\n", __func__, bytes);
        return 0;
    }
    return 1;
}

static void print_stats(const struct options *op, const struct cost *cost,
  const struct image *image)
{
    struct costf sqcm, sqm, sqin, bl;

    drop2sqcm(&sqcm, cost, op->dpi);
    drop2sqm (&sqm,  cost, op->dpi);
    drop2sqin(&sqin, cost, op->dpi);
    drop2bl  (&bl,   cost, op->dpi);

    if(image != NULL) {
        double w, h;

        w = px_to_cm(image->width, op->dpi);
        h = px_to_cm(image->height, op->dpi);
        printf("%lu x %lu px @ %u dpi = %.2f x %.2f cm (%.2f cm²)\n",
               image->width, image->height, op->dpi, w, h, w * h);

        w = px_to_in(image->width, op->dpi);
        h = px_to_in(image->height, op->dpi);
        printf("%lu x %lu px @ %u dpi = %.2f x %.2f in (%.2f in²)\n",
               image->width, image->height, op->dpi, w, h, w * h);
    }

    printf("\t(1 ISO A4-Blatt = 0.06237 m²; 1 Droplet = 1/255 px)\n");

    printf("%-8s  %7s  %7s  %7s  %7s  %7s\n",
           "UNIT", "CYAN", "MAGENTA", "YELLOW", "BLACK", "TOTAL");
    printf("Droplets  %7lld  %7lld  %7lld  %7lld  %7lld\n",
           cost->c, cost->m, cost->y, cost->k, cost->t);
    printf("i*cm²     %7.2f  %7.2f  %7.2f  %7.2f  %7.2f\n",
           sqcm.c, sqcm.m, sqcm.y, sqcm.k, sqcm.t);
    printf("i*m²      %7.2f  %7.2f  %7.2f  %7.2f  %7.2f\n",
           sqm.c, sqm.m, sqm.y, sqm.k, sqm.t);
    printf("i*in²     %7.2f  %7.2f  %7.2f  %7.2f  %7.2f\n",
           sqin.c, sqin.m, sqin.y, sqin.k, sqin.t);
    printf("i*A4      %7.2f  %7.2f  %7.2f  %7.2f  %7.2f\n",
           bl.c, bl.m, bl.y, bl.k, bl.t);
    printf("\n");

    return;
#undef COLOR
}

static inline double px_to_cm(unsigned int p, unsigned int dpi)
{
    return 2.54 * p / dpi;
}

static inline double px_to_in(unsigned int p, unsigned int dpi)
{
    return (double)p / dpi;
}

//=============================================================================
