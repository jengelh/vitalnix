/*
 *	lpacct/drop.c - Conversion and counting functions (pure math)
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include "drop.h"
#include "image.h"
#include "lpacct.h"

/* Functions */
static int pxcost_cmyk(int, struct image *, struct cost *);
static int pxcost_cmypk(int, struct image *, struct cost *);
static int pxcost_cmy(int, struct image *, struct cost *);
static int pxcost_gray(int, struct image *, struct cost *);
static inline unsigned int abs1(int);
static inline unsigned int kdist(unsigned int, unsigned int, unsigned int);
static inline unsigned int mean3(unsigned int, unsigned int, unsigned int);
static inline unsigned int min3(unsigned int, unsigned int, unsigned int);
static inline unsigned int rgb_to_gray(unsigned int, unsigned int,
	unsigned int);

/* Variables */
int (*const mpxm_analyzer[])(int, struct image *, struct cost *) = {
	[COLORSPACE_CMYK]  = pxcost_cmyk,
	[COLORSPACE_CMYPK] = pxcost_cmypk,
	[COLORSPACE_CMY]   = pxcost_cmy,
	[COLORSPACE_GRAY]  = pxcost_gray,
};

//-----------------------------------------------------------------------------
void drop2bl(struct costf *out, const struct cost *in, int dpi)
{
	double d = 255 * 29.7 * 21 * dpi * dpi;
	out->c = 6.4516 * in->c / d;
	out->m = 6.4516 * in->m / d;
	out->y = 6.4516 * in->y / d;
	out->k = 6.4516 * in->k / d;
	out->t = 6.4516 * in->t / d;
	out->p = in->p;
	return;
}

void drop2sqcm(struct costf *out, const struct cost *in, int dpi)
{
	/*
	 * 1 in   = 2.54   cm
	 * 1 in^2 = 6.4516 cm^2
	 */
	long d = 255 * dpi * dpi;
	out->c = 6.4516 * in->c / d;
	out->m = 6.4516 * in->m / d;
	out->y = 6.4516 * in->y / d;
	out->k = 6.4516 * in->k / d;
	out->t = 6.4516 * in->t / d;
	out->p = in->p;
	return;
}

void drop2sqm(struct costf *out, const struct cost *in, int dpi)
{
	long long d = 2550000ULL * dpi * dpi;
	out->c = 6.4516 * in->c / d;
	out->m = 6.4516 * in->m / d;
	out->y = 6.4516 * in->y / d;
	out->k = 6.4516 * in->k / d;
	out->t = 6.4516 * in->t / d;
	out->p = in->p;
	return;
}

void drop2sqin(struct costf *out, const struct cost *in, int dpi)
{
	long d = 255 * dpi * dpi; /* max for (dpi*dpi) is 4104 */
	out->c = (double)in->c / d;
	out->m = (double)in->m / d;
	out->y = (double)in->y / d;
	out->k = (double)in->k / d;
	out->t = (double)in->t / d;
	out->p = in->p;
	return;
}

//-----------------------------------------------------------------------------
/*
 * pxcost_cmyk -
 * @image:	Image data
 * @cost:	Storage point for image cost
 *
 * Count the amount of CMYK color used in an array of RGB24 pixels (PPM). The
 * unit is a "droplet", which is a 1/255 full drop of color. For example, if
 * @res[0] is 1020 (255x4), color for exactly four full-intensity cyan pixels
 * has been used, or for eight half-intensity pixels, etc...
 *
 * 	255 droplets = 1 px^2
 *
 * This function and pxcost_cmy() use integer math to gain a little speed
 * over floating point operations.
 */
static int pxcost_cmyk(int fd, struct image *image, struct cost *cost)
{
	unsigned long long tc = 0, tm = 0, ty = 0, tk = 0;
	const unsigned char *current;
	unsigned int k, w;
	long ret, pixels;

	while ((ret = mpxm_chunk_next(fd, image)) > 0) {
		invert_image(image);
		current = image->buffer;
		pixels  = ret / 3;
		while (pixels-- > 0) {
			k = min3(current[0], current[1], current[2]);
			if (k != 255) {
				w = 255 - k;
				tc += 255 * (current[0] - k) / w;
				tm += 255 * (current[1] - k) / w;
				ty += 255 * (current[2] - k) / w;
			}
			tk += k;
			current += 3;
		}
	}

	cost->c = tc;
	cost->m = tm;
	cost->y = ty;
	cost->k = tk;
	cost->t = tc + tm + ty + tk;
	return ret;
}

/*
 * pxcost_cmypk -
 * @image:	Image data
 * @cost:	Storage point for image cost
 *
 * Cost analyzer for CMY+K. Experimental and probably not conforming quite
 * to actual printer implementations.
 */
static int pxcost_cmypk(int fd, struct image *image, struct cost *cost)
{
	unsigned long long tc = 0, tm = 0, ty = 0, tk = 0;
	const unsigned char *current;
	long ret, pixels;

	while ((ret = mpxm_chunk_next(fd, image)) > 0) {
		invert_image(image);
		current = image->buffer;
		pixels  = ret / 3;
		while (pixels-- > 0) {
			if (kdist(current[0], current[1], current[2]) <= 8) {
				tk += mean3(current[0], current[1], current[2]);
			} else {
				tc += current[0];
				tm += current[1];
				ty += current[2];
			}
			current += 3;
		}
	}

	cost->c = tc;
	cost->m = tm;
	cost->y = ty;
	cost->k = tk;
	cost->t = tc + tm + ty + tk;
	return ret;
}

/*
 * pxcost_cmy -
 * @image:	Image data
 * @cost:	Storage point for image cost
 *
 * Like pxcost_cmyk(), but for CMY. Use with printers that do not have a black
 * component, like HP DeskJet 320.
 */
static int pxcost_cmy(int fd, struct image *image, struct cost *cost)
{
	unsigned long long c = 0, m = 0, y = 0;
	const unsigned char *current;
	long ret, pixels;

	while ((ret = mpxm_chunk_next(fd, image)) > 0) {
		current = image->buffer;
		pixels  = ret / 3;
		c += 255 * pixels;
		m += 255 * pixels;
		y += 255 * pixels;
		while (pixels-- > 0) {
			c -= current[0];
			m -= current[1];
			y -= current[2];
			current += 3;
		}
	}

	cost->c = c;
	cost->m = m;
	cost->y = y;
	cost->k = 0;
	cost->t = c + m + y;
	return ret;
}

/*
 * pxcost_gray -
 * @image:	Image data
 * @cost:	Storage point for image cost
 *
 * Count the amount of black color used in an array of RGB24 or K8 pixels.
 */
static int pxcost_gray(int fd, struct image *image, struct cost *cost)
{
	const unsigned char *current;
	unsigned long long k = 0;
	long ret = 0, pixels;

	if (image->type == FILETYPE_PPM) {
		while ((ret = mpxm_chunk_next(fd, image)) > 0) {
			/*
			 * If @r, @g and @b are 32 bit, then at most 16843009
			 * pixels can be processed per chunk without causing
			 * overflow.
			 */
			unsigned int r = 0, g = 0, b = 0;
			current = image->buffer;
			pixels  = ret / 3;
			k      += 255 * pixels;

			while (pixels-- > 0) {
				r += current[0];
				g += current[1];
				b += current[2];
				current += 3;
			}
			k -= rgb_to_gray(r, g, b);
		}
	} else if (image->type == FILETYPE_PGM) {
		while ((pixels = ret = mpxm_chunk_next(fd, image)) > 0) {
			current = image->buffer;
			k      += 255 * pixels;
			while (pixels-- > 0)
				k -= *current++;
		}
	}

	cost->c = cost->m = cost->y = 0;
	cost->k = cost->t = k;
	return ret;
}

//-----------------------------------------------------------------------------
static inline unsigned int abs1(int a)
{
	return (a < 0) ? -a : a;
}

/*
 * kdist - distance to black
 * @r:	red component
 * @g:	green component
 * @b:	blue component
 *
 * Calculates the distance of (@r,@g,@b) to the gray baseline
 * (0,0,0)->(255,255,255). Input range is 0..255, output range is 0..43350.
 * http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
 */
static inline unsigned int kdist(unsigned int r, unsigned int g,
	unsigned int b)
{
	unsigned int x = abs1(r - g);
	unsigned int y = abs1(r - b);
	unsigned int z = abs1(g - b);
	/* Floating point variant (slower): ceil((x*x + y*y + z*z) / 3.0) */
	return (x * x + y * y + z * z + 2) / 3;
}

static inline unsigned int mean3(unsigned int a, unsigned int b,
    unsigned int c)
{
	return (a + b + c) / 3;
}

static inline unsigned int min3(unsigned int a, unsigned int b,
    unsigned int c)
{
	unsigned int r = (a < b) ? a : b;
	return (r < c) ? r : c;
}

static inline unsigned int rgb_to_gray(unsigned int r, unsigned int g,
    unsigned int b)
{
	return (78 * r + 151 * g + 27 * b) / 256;
}

//=============================================================================
