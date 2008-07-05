#ifndef LPACCT_DROP_H
#define LPACCT_DROP_H 1

struct image;

struct cost {
	unsigned long long c, m, y, k, t;
	unsigned int p;
};

struct costf {
	double c, m, y, k, t;
	unsigned int p;
};

/*
 *	FUNCTIONS
 */
extern void drop2bl(struct costf *, const struct cost *, int);
extern void drop2sqcm(struct costf *, const struct cost *, int);
extern void drop2sqm(struct costf *, const struct cost *, int);
extern void drop2sqin(struct costf *, const struct cost *, int);

/*
 *	VARIABLES
 */
extern int (*const mpxm_analyzer[])(int, struct image *, struct cost *);

#endif /* LPACCT_DROP_H */
