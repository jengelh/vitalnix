/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.

    Conversion and counting functions (pure math).
*/
#include "drop.h"
#include "image.h"
#include "util.h"

// Functions
static void pxcost_cmyk_mmap(const struct image *, struct cost *);
static void pxcost_cmy_mmap(const struct image *, struct cost *);
static void pxcost_gray_mmap(const struct image *, struct cost *);
static inline unsigned int min3(unsigned int, unsigned int, unsigned int);
static inline unsigned int rgb_to_gray(unsigned int, unsigned int,
  unsigned int);

// Variables
void (*const pixel_cost[])(const struct image *, struct cost *) = {
    [COLORSPACE_GRAY] = pxcost_gray_mmap,
    [COLORSPACE_CMYK] = pxcost_cmyk_mmap,
    [COLORSPACE_CMY]  = pxcost_cmy_mmap,
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
    return;
}

void drop2sqcm(struct costf *out, const struct cost *in, int dpi)
{
    /*
        1 in   = 2.54   cm
        1 in^2 = 6.4516 cm^2
    */
    long d = 255 * dpi * dpi;
    out->c = 6.4516 * in->c / d;
    out->m = 6.4516 * in->m / d;
    out->y = 6.4516 * in->y / d;
    out->k = 6.4516 * in->k / d;
    out->t = 6.4516 * in->t / d;
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
    return;
}

void drop2sqin(struct costf *out, const struct cost *in, int dpi)
{
    long d = 255 * dpi * dpi; // max for (dpi*dpi) is 4104
    out->c = (double)in->c / d;
    out->m = (double)in->m / d;
    out->y = (double)in->y / d;
    out->k = (double)in->k / d;
    out->t = (double)in->t / d;
    return;
}

//-----------------------------------------------------------------------------
/*  pixel_cost_cmyk
    @image:     Image data
    @cost:      Storage point for image cost

    Count the amount of CMYK color used in an array of RGB24 pixels (PPM). The
    unit is a "droplet", which is a 1/255 full drop of color. For example, if
    @res[0] is 1020 (255x4), color for exactly four full-intensity cyan pixels
    has been used, or for eight half-intensity pixels, etc...

        255 droplets = 1 px^2

    This function and pixel_cost_cmy() use integer math to gain a little speed
    over floating point operations.
*/
static void pxcost_cmyk_mmap(const struct image *image, struct cost *cost)
{
    unsigned long long tc = 0, tm = 0, ty = 0, tk = 0;
    const unsigned char *current = image->data;
    unsigned long pixels = image->pixels;
    unsigned char c, m, y, k;

    while(pixels-- > 0) {
        c = 255 - current[0];
        m = 255 - current[1];
        y = 255 - current[2];
        k = min3(c, m, y);
        if(k == 255) {
            c = m = y = 0;
        } else {
            c = 255 * (c - k) / (255 - k);
            m = 255 * (m - k) / (255 - k);
            y = 255 * (y - k) / (255 - k);
        }
        tc += c;
        tm += m;
        ty += y;
        tk += k;
        current += 3;
    }

    cost->c = tc;
    cost->m = tm;
    cost->y = ty;
    cost->k = tk;
    cost->t = tc + tm + ty + tk;
    return;
}

/*  pixel_cost_cmy
    @image:     Image data
    @cost:      Storage point for image cost

    Like cmyk_cost_pixel(), but for CMY. Use with printers that do not have a
    black component, like HP DeskJet 320.
*/
static void pxcost_cmy_mmap(const struct image *image, struct cost *cost)
{
    const unsigned char *current = image->data;
    unsigned long long c = 0, m = 0, y = 0;
    unsigned long pixels = image->pixels;

    while(pixels-- > 0) {
        c += 255 - current[0];
        m += 255 - current[1];
        y += 255 - current[2];
        current += 3;
    }

    cost->c = c;
    cost->m = m;
    cost->y = y;
    cost->k = 0;
    cost->t = c + m + y;
    return;
}

/*  pixel_cost_gray
    @image:     Image data
    @cost:      Storage point for image cost

    Count the amount of black color used in an array of RGB24 or K8 pixels.
*/
static void pxcost_gray_mmap(const struct image *image, struct cost *cost)
{
    const unsigned char *current  = image->data;
    unsigned long pixels = image->pixels;
    unsigned long long k = 0;

    if(image->type == FILETYPE_PGM) {
        while(pixels-- > 0)
            k += 255 - *current++;
    } else {
        while(pixels-- > 0) {
            k += 255 - rgb_to_gray(current[0], current[1], current[2]);
            current += 3;
        }
    }

    cost->c = cost->m = cost->y = 0;
    cost->k = k;
    return;
}

//-----------------------------------------------------------------------------
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
