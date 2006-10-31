#ifndef LPACCT_COST_H
#define LPACCT_COST_H 1

struct image;

struct cost {
    unsigned long long c, m, y, k, t;
};

struct costf {
    double c, m, y, k, t;
};

/*
 *      FUNCTIONS
 */
extern void drop2bl(struct costf *, const struct cost *, int);
extern void drop2sqcm(struct costf *, const struct cost *, int);
extern void drop2sqm(struct costf *, const struct cost *, int);
extern void drop2sqin(struct costf *, const struct cost *, int);
extern void pixel_cost_cmyk(const struct image *, struct cost *);
extern void pixel_cost_cmy(const struct image *, struct cost *);
extern void pixel_cost_gray(const struct image *, struct cost *);

/*
 *      VARIABLES
 */
extern void (*const pixel_cost[])(const struct image *, struct cost *);

#endif // LPACCT_PIXELCOST_H
