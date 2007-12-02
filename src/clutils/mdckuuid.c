/*
 *	mdckuuid - find UUID similarities
 *	Copyright © CC Computer Consultants GmbH, 2006 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <libHX.h>
#include <vitalnix/config.h>
#include <vitalnix/compiler.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/libvxutil.h>
#define sizeof_s(x) (sizeof(x) - 1)

struct ckentry {
	struct vxeds_entry entry;
	char source;
};

struct levd_pair {
	const struct ckentry *x, *y;
	unsigned int uuid_dist, name_dist, cost_factor, combined_dist;
};

static char *ck_db_name, *ck_input_file, *ck_input_fmt;
static unsigned int ck_threshold = 100;

static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " mdckuuid\n");
	exit(EXIT_SUCCESS);
}

static bool get_options(int *argc, const char ***argv)
{
	struct HXoption options_table[] = {
		{.sh = 'M', .type = HXTYPE_STRING, .ptr = &ck_db_name,
		 .help = "Use specified database", .htyp = "name"},
		{.sh = 'T', .type = HXTYPE_UINT, .ptr = &ck_threshold,
		 .help = "Omit all entries with a greater distance "
		 "(default: 100)", .htyp = "dist"},
		{.sh = 'V', .type = HXTYPE_STRING, .cb = show_version,
		 .help = "Show version information"},
		{.sh = 'i', .type = HXTYPE_STRING, .ptr = &ck_input_file,
		 .help = "External Data Source", .htyp = "file"},
		{.sh = 't', .type = HXTYPE_STRING, .ptr = &ck_input_fmt,
		 .help = "EDS type", .htyp = "TYPE"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	if (HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0)
		return false;

	if (ck_input_file == NULL) {
		fprintf(stderr, "%s: You need to specify -i\n", **argv);
		return false;
	}

	return true;
}

static void read_eds(struct HXdeque *workspace)
{
	struct ckentry *obj;
	void *state;
	int ret;

	ret = vxeds_open(ck_input_file, ck_input_fmt, &state);
	if (ret <= 0)
		return;

	while (true) {
		obj = malloc(sizeof(*obj));
		if (obj == NULL) {
			perror("malloc");
			abort();
		}

		if ((ret = vxeds_read(state, &obj->entry)) < 0) {
			if (ret == -EINVAL)
				fprintf(stderr, "%s: Warning: Stumbled upon a "
				        "bogus entry in Data Source, "
				        "stopping.\n", __func__);
			else
				fprintf(stderr, "%s+eds_read(): %s\n",
				        __func__, strerror(-ret));
			vxeds_free_entry(&obj->entry);
			free(obj);
			continue;
		} else if (ret == 0) {
			/* End of list */
			free(obj);
			break;
		}

		obj->source = '-';
		HXdeque_push(workspace, obj);
	}

	vxeds_close(state);
	return;
}

static void read_pdb(struct HXdeque *workspace, struct vxpdb_state *dbh)
{
	struct vxpdb_user user = {};
	struct ckentry *obj;
	void *trav;

	trav = vxpdb_usertrav_init(dbh);
	if (trav == NULL) {
		perror("vxpdb_usertrav_init");
		return;
	}

	while (vxpdb_usertrav_walk(dbh, trav, &user) > 0) {
		if (user.vs_uuid == NULL)
			continue;
		obj = malloc(sizeof(*obj));
		if (obj == NULL) {
			perror("malloc");
			abort();
		}
		obj->source          = '+';
		obj->entry.uuid      = user.vs_uuid;
		obj->entry.full_name = user.pw_real;
		user.vs_uuid         = NULL;
		user.pw_real         = NULL;
		HXdeque_push(workspace, obj);
		vxpdb_user_free(&user, false);
		vxpdb_user_clean(&user);
	}

	vxpdb_usertrav_free(dbh, trav);
	return;
}

static inline unsigned int min3(unsigned int a, unsigned int b, unsigned int c)
{
	if (a < b)
		b = a;
	if (b < c)
		return b;
	else
		return c;
}

static bool timebolt(void)
{
	static time_t last = 0;
	time_t now = time(NULL);

	if (now <= last)
		return false;

	last = now;
	return true;
}

/*
 * levd - Levenshtein distance
 * @s:		string
 * @width:	length of string @s to consider
 * @t:		string
 * @height:	length of string @t to consider
 */
static unsigned int levd(const char *s, unsigned int width,
    const char *t, unsigned int height)
{
	unsigned int cost, i, j;
	unsigned int *d;

	if (width == 0)
		return height;
	if (height == 0)
		return width;
	d = calloc(sizeof(int), (width + 1) * (height + 1));
#define LOC(x, y) ((y) * (width + 1) + (x))
	for (i = 0; i <= width; ++i)
		d[LOC(i, 0)] = i;
	for (j = 0; j <= height; ++j)
		d[LOC(0, j)] = j;
	for (i = 1; i <= width; ++i) {
		for (j = 1; j <= height; ++j) {
			cost = (s[i-1] == t[j-1]) ? 0 : 1;
			d[LOC(i, j)] = min3(
					d[LOC(i-1, j)] + 1,
					d[LOC(i, j-1)] + 1,
					d[LOC(i-1, j-1)] + cost);
		}
	}
	cost = d[LOC(width, height)];
	free(d);
	return cost;
#undef LOC
}

/*
 * levd_p - compare a pdb with an eds entry
 * @pair:	pair structure
 * @pdb:	PDB entry
 * @eds:	EDS entry
 *
 * Compares a PDB with an EDS entry, calculates their distances,
 * add weighting and make a final distance for sorting.
 */
static void levd_p(struct levd_pair *pair, const struct ckentry *pdb,
    const struct ckentry *eds)
{
	unsigned int pdb_len, eds_len, w;
	char *pdb_uuid, *eds_uuid, *ptr;

	/*
	 * Do not compare the whole UUID if it is VX3A. It contains
	 * an MD5 sum of the full name, which is not helpful at all
	 * for Levenshtein distance. Also skip the {type} specifier.
	 */
	if (strncmp(pdb->entry.uuid, "{VX3A}", sizeof_s("{VX3A}")) == 0) {
		pdb_uuid = pdb->entry.uuid + sizeof_s("{VX3A}");
		pdb_len  = 6;
	} else if (*pdb->entry.uuid == '{' &&
	    (ptr = strchr(pdb->entry.uuid, '}')) != NULL) {
		pdb_uuid = ptr + 1;
		pdb_len  = strlen(pdb_uuid);
	} else {
		pdb_uuid = pdb->entry.uuid;
		pdb_len  = strlen(pdb_uuid);
	}

	if (strncmp(eds->entry.uuid, "{VX3A}", sizeof_s("{VX3A}")) == 0) {
		eds_uuid = eds->entry.uuid + sizeof_s("{VX3A}");
		eds_len  = 6;
	} else if (*eds->entry.uuid == '{' &&
	    (ptr = strchr(eds->entry.uuid, '}')) != NULL) {
		eds_uuid = ptr + 1;
		eds_len  = strlen(eds_uuid);
	} else {
		eds_uuid = eds->entry.uuid;
		eds_len  = strlen(eds_uuid);
	}

	pair->x = pdb;
	pair->y = eds;
	pair->uuid_dist = levd(pdb_uuid, pdb_len, eds_uuid, eds_len);

	/*
	 * Tests have shown that in medium-scale environments (testcase:
	 * 1700 real users), small Leveshtian distances (e.g. < 10) are
	 * very common. But a distance of 2, e.g.  when going from "John
	 * Atams" to "John Adamz" is much less likely than a distance of 1
	 * (to "John Adams"). Weigh in name changes quadratically.
	 *
	 * Note this does not handle UTF-8, so changing a character may
	 * actually incur an extra distance of up to 12 (I think);
	 * usually it is 2 to 4 in the normal case (Latin characters).
	 */
	w = levd(pdb->entry.full_name, strlen(pdb->entry.full_name),
	         eds->entry.full_name, strlen(eds->entry.full_name));
	pair->name_dist = w * w;

	/*
	 * If both UUID (special VX3A case in mind) and name were changed,
	 * quadruple the combined distance.
	 */
	if (pair->uuid_dist != 0 && pair->name_dist != 0)
		pair->cost_factor = 4;
	else
		pair->cost_factor = 1;

	pair->combined_dist = (pair->uuid_dist + pair->name_dist) *
	                      pair->cost_factor;
	return;
}

static int sort_by_distance(const void *va, const void *vb)
{
	const struct levd_pair **a = static_cast(const struct levd_pair **, va);
	const struct levd_pair **b = static_cast(const struct levd_pair **, vb);
	return (*a)->combined_dist - (*b)->combined_dist;
}

static void output(const struct HXdeque *pdb, const struct HXdeque *eds)
{
	const struct HXdeque_node *y, *x;
	struct HXdeque *pair_list = HXdeque_init();
	bool tty = isatty(STDERR_FILENO);
	unsigned int i = 0;
	void **pair_flat;

	for (y = pdb->first; y != NULL; y = y->next) {
		struct levd_pair p;

		for (x = eds->first; x != NULL; x = x->next) {
			levd_p(&p, y->ptr, x->ptr);
			if (p.combined_dist == 0 ||
			    p.combined_dist > ck_threshold)
				continue;
			HXdeque_push(pair_list, HX_memdup(&p, sizeof(p)));
		}
		++i;
		if (tty && timebolt())
			fprintf(stderr, "\r\e[2K" "%u/%u", i, pdb->items);
	}

	if (tty)
		fprintf(stderr, "\r\e[2K");

	pair_flat = HXdeque_to_vec(pair_list, NULL);
	if (pair_flat == NULL) {
		perror("malloc");
		return;
	}
	qsort(pair_flat, pair_list->items, sizeof(struct levpair *),
	      sort_by_distance);

	for (i = 0; i < pair_list->items; ++i) {
		struct levd_pair *pair = pair_flat[i];
		unsigned int m, n;
		char abuf[32], bbuf[32];

		m = snprintf(abuf, sizeof(abuf), "[%4u]", pair->combined_dist);
		n = snprintf(bbuf, sizeof(bbuf), "%u/%u/%u",
		             pair->uuid_dist, pair->name_dist, pair->cost_factor);
		if (m > n)
			n = m;
		printf("%*s: - %-44s  %s\n"
		       "%*s: + %-44s  %s\n\n",
		       n, abuf,
		       pair->x->entry.uuid, pair->x->entry.full_name,
		       n, bbuf,
		       pair->y->entry.uuid, pair->y->entry.full_name);
	}

	free(pair_flat);
	HXdeque_genocide(pair_list);
	return;
}

static int run1(void)
{
	struct HXdeque *workspace_pdb, *workspace_eds;
	struct vxpdb_state *dbh;
	int ret;

	if ((dbh = vxpdb_load(ck_db_name)) == NULL) {
		perror("Could not load database");
		return EXIT_FAILURE;
	}

	if ((ret = vxpdb_open(dbh, 0)) <= 0) {
		fprintf(stderr, "Could not open database: %s\n",
		        strerror(-ret));
		goto out;
	}

	if ((workspace_pdb = HXdeque_init()) == NULL) {
		perror("malloc");
		goto out2;
	}
	if ((workspace_eds = HXdeque_init()) == NULL) {
		perror("malloc");
		goto out3;
	}

	read_eds(workspace_eds);
	read_pdb(workspace_pdb, dbh);
	output(workspace_pdb, workspace_eds);
	HXdeque_genocide(workspace_eds);
 out3:
	HXdeque_genocide(workspace_pdb);
 out2:
	vxpdb_close(dbh);
 out:
	vxpdb_unload(dbh);
	return ret;
}

int main(int argc, const char **argv)
{
	int ret;

	ck_db_name = HX_strdup("*");
	if (!get_options(&argc, &argv))
		return EXIT_FAILURE;

	ret = run1();
	free(ck_db_name);
	free(ck_input_file);
	free(ck_input_fmt);
	return ret;
}
