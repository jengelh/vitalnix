/*
 *	lpacct/image.c - Image processing routines
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2006 - 2008
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libHX/defs.h>
#include <libHX/string.h>
#include "acct.h"
#include "drop.h"
#include "global.h"
#include "image.h"
#include "lpacct.h"

/* Functions */
static void cost_add(struct cost *, const struct cost *);
static inline unsigned long min2(unsigned long, unsigned long);
static int mpxm_chunk_alloc(struct image *);
static int mpxm_fdgetl(int, hxmc_t **);
static int mpxm_get_header(int, struct image *);
static void print_stats(const struct options *, const struct cost *,
	const struct image *);
static inline double px_to_cm(unsigned int, unsigned int);
static inline double px_to_in(unsigned int, unsigned int);

//-----------------------------------------------------------------------------
/**
 * invert_image -
 * @image:	image to operate on
 *
 * Efficiently inverts all pixels in the image.
 */
void invert_image(struct image *image)
{
	unsigned long *Lptr = image->buffer;
	unsigned char *Bptr, *Bend = image->buffer + image->buffer_size;
	unsigned long *Lend = static_cast(void *,
	                      reinterpret_cast(unsigned long, Bend) &
	                      ~(sizeof(long) - 1));
	while (Lptr < Lend) {
		*Lptr = ~*Lptr;
		++Lptr;
	}
	Bptr = static_cast(void *, Lptr);
	while (Bptr < Bend) {
		*Bptr = ~*Bptr;
		++Bptr;
	}
}

/**
 * mpxm_chunk_next -
 * @image:	image state to operate on
 *
 * Reads the next chunk from @image->fd. Returns the number of bytes read,
 * or zero on end-of-image, and %-errno on failure. Returns %-EPIPE when
 * EOF is prematurely encountered.
 */
long mpxm_chunk_next(int fd, struct image *image)
{
	unsigned long block_size, read_size;
	unsigned char *ptr = image->buffer;
	long ret;

	if (image->rem_bytes == 0)
		return 0;

	/*
	 * The caller may depend on the fact that this function always returns
	 * a multiple of a certain number of bytes (e.g. 3 for RGB images). 
	 * @image->buffer_size is assured to be a multiple, however, a single
	 * read() on a pipe might not yield a multiple, which is why we need a
	 * loop, to be sure to read @image->buffer_size.
	 */
	block_size = read_size = min2(image->rem_bytes, image->buffer_size);

	while (read_size > 0) {
		if ((ret = read(fd, ptr, read_size)) < 0)
			return -errno;
		else if (ret == 0)
			/*
			 * If we were at EOF, the loop condition would be
			 * false, hence we never should get here during the
			 * loop unless the pipe is prematurely closed.
			 */
			return -EPIPE;

		read_size -= ret;
		ptr += ret;
	}

	image->rem_bytes -= block_size;
	return block_size;
}

/**
 * mpxm_process -
 * @fd:		file descriptor to read from
 * @cost:	accounting structure
 *
 * Process all pages from @fd and write accounting results into @cost.
 * Propagates the error from any subfunction to the caller.
 */
int mpxm_process(int fd, const struct options *op)
{
	struct image image = {};
	struct cost all_cost = {}, page_cost = {};
	int ret;

	while ((ret = mpxm_get_header(fd, &image)) > 0) {
		if ((ret = mpxm_chunk_alloc(&image)) <= 0)
			pr_exit(NULL, "mpxm_chunk_alloc: %s\n", strerror(-ret));

		image.rem_bytes = image.nr_bytes;
		ret = mpxm_analyzer[op->colorspace](fd, &image, &page_cost);
		if (ret < 0)
			pr_exit(NULL, "mpxm_analyzer: %s\n", strerror(-ret));

		cost_add(&all_cost, &page_cost);
		++all_cost.p;

		if (op->per_page_stats) {
			printf("Page %u:\n", all_cost.p);
			print_stats(op, &page_cost, &image);
		}
	}

	free(image.buffer);
	if (op->per_doc_stats) {
		printf("Total cost of all %u pages\n", all_cost.p);
		print_stats(op, &all_cost, NULL);
	}

	/* ->do_account is only set when called as a CUPS filter. */
	if (ret >= 0) {
		struct costf a4;
		drop2bl(&a4, &all_cost, op->dpi);
		if (op->acct_syslog)
			acct_syslog(op, &a4);
		if (op->acct_mysql)
			acct_mysql(op, &a4);
	}

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
}

static inline unsigned long min2(unsigned long a, unsigned long b)
{
	return (a < b) ? a : b;
}

/**
 * mpxm_chunk_alloc -
 * @image:	image state to operate on
 *
 * Allocates the buffer for chunk reading. Returns positive non-zero on
 * success or %-errno otherwise.
 */
static int mpxm_chunk_alloc(struct image *image)
{
	if (image->buffer == NULL) {
		image->buffer_size = 64 * 1024 * 3;
		if ((image->buffer = malloc(image->buffer_size)) == NULL)
			return -errno;
	}

	return 1;
}

/**
 * mpxm_fdgetl -
 * @fd:		file descriptor to read from
 * @res:	buffer to put data into
 *
 * Reads a line from @fd and puts it into @res.
 */
static int mpxm_fdgetl(int fd, hxmc_t **res)
{
	char temp[256], *temp_ptr = temp;
	int ret;

	if (*res == NULL)
		*res = HXmc_strinit("");
	else
		HXmc_trunc(res, 0);

	while (true) {
		/*
		 * On read error or newline (make sure newlines makes
		 * it into @res), flush and return.
		 */
		if ((ret = read(fd, temp_ptr, 1)) <= 0 || *temp_ptr++ == '\n') {
			*temp_ptr = '\0';
			HXmc_strcat(res, temp);
			break;
		}
		if (temp_ptr == temp + sizeof(temp) - 1) {
			*temp_ptr = '\0';
			HXmc_strcat(res, temp);
			temp_ptr = temp;
		}
	}

	return (ret < 0) ? -errno : 0;
}

/**
 * mpxm_get_header -
 * @fd:		file descriptor to read from
 * @image:	pointer to image parameters
 *
 * Reads the next PPM header from the stream @fd into @image. Returns true on
 * success, otherwise false.
 */
static int mpxm_get_header(int fd, struct image *image)
{
	hxmc_t *ln = NULL;
	int ret;

	if ((ret = mpxm_fdgetl(fd, &ln)) < 0)
		return 0;
	HX_chomp(ln);

	if (strcmp(ln, "P6") == 0)
		image->type = FILETYPE_PPM;
	else if (strcmp(ln, "P5") == 0)
		image->type = FILETYPE_PGM;
	else
		return 0;

	/* Some programs put a comment line in it */
	do {
		if ((ret = mpxm_fdgetl(fd, &ln)) < 0)
			return 0;
	} while (*ln == '#');

	if (sscanf(ln, "%lu %lu", &image->width, &image->height) != 2)
		return 0;

	/* bits per pixel line (unused) */
	mpxm_fdgetl(fd, &ln);
	HXmc_free(ln);

	image->nr_pixels = static_cast(unsigned long long,
	                   image->width * image->height);
	image->nr_bytes  = image->nr_pixels;
	if (image->type == FILETYPE_PPM)
		image->nr_bytes *= 3;

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

	if (image != NULL) {
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

	printf(
		"\t(1 ISO A4-Blatt = 0.06237 m²; 1 Droplet = 1/255 px)\n"
		"%-8s  %7s  %7s  %7s  %7s  %7s\n"
		"Droplets  %7lld  %7lld  %7lld  %7lld  %7lld\n"
		"i*cm²     %7.2f  %7.2f  %7.2f  %7.2f  %7.2f\n"
		"i*m²      %7.2f  %7.2f  %7.2f  %7.2f  %7.2f\n"
		"i*in²     %7.2f  %7.2f  %7.2f  %7.2f  %7.2f\n"
		"i*A4      %7.2f  %7.2f  %7.2f  %7.2f  %7.2f\n"
		"\n",
		"UNIT", "CYAN", "MAGENTA", "YELLOW", "BLACK", "TOTAL",
		cost->c, cost->m, cost->y, cost->k, cost->t,
		sqcm.c, sqcm.m, sqcm.y, sqcm.k, sqcm.t,
		sqm.c, sqm.m, sqm.y, sqm.k, sqm.t,
		sqin.c, sqin.m, sqin.y, sqin.k, sqin.t,
		bl.c, bl.m, bl.y, bl.k, bl.t);
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
