/*
 *	pwlfmt - Format password lists
 *	Copyright © Jan Engelhardt <jengelh [at] medozas de>, 2003 - 2011
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX/init.h>
#include <libHX/option.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include <vitalnix/libvxmdfmt/vtable.h>

/* Functions */
static bool get_options(int *, const char ***, struct pwlfmt_workspace *);
static void list_styles(void);
static int pwlfmt(struct pwlfmt_workspace *);
static void show_version(const struct HXoptcb *);

/* Variables */
static unsigned int List_styles = 0;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
	struct pwlfmt_workspace i = {};
	int ret;

	if ((ret = HX_init()) <= 0)
		abort();
	if (!get_options(&argc, &argv, &i))
		return EXIT_FAILURE;

	ret = EXIT_SUCCESS;
	if (List_styles)
		list_styles();
	else if (pwlfmt(&i) <= 0)
		ret = EXIT_FAILURE;

	free(i.style_name);
	free(i.input_file);
	free(i.output_file);
	free(i.template_file);
	HX_exit();
	return ret;
}

static void list_styles(void)
{
	const struct pwlstyle_vtable *p;
	void *trav = NULL;

	printf("Available styles: (asterisk denotes \"requires template"
	       " file\" using -t option)\n");

	while ((p = pwlstyles_trav(&trav)) != NULL)
		printf("\t" "%c%-10s   %s\n",
		       (p->require_template ? '*' : ' '), p->name, p->desc);
}

static int pwlfmt(struct pwlfmt_workspace *i)
{
	int ret;

	if (i->style_name == NULL || i->input_file == NULL ||
		i->output_file == NULL) {
		fprintf(stderr, "-i, -o and -s options are required.\n");
		return 0;
	}

	if ((ret = pwlfmt_new(i)) <= 0) {
		if (ret == -PWLFMT_EEINPUT)
			fprintf(stderr, "Error: %s %s: %s\n",
			        pwlfmt_strerror(ret), i->input_file,
			        strerror(errno));
		else if (ret == -PWLFMT_EEOUTPUT)
			fprintf(stderr, "Error: %s %s: %s\n",
			        pwlfmt_strerror(ret), i->output_file,
			        strerror(errno));
		else if (pwlfmt_strerror(ret) != NULL)
			fprintf(stderr, "Error: %s\n", pwlfmt_strerror(ret));
		else
			fprintf(stderr, "Error: %s\n", strerror(errno));
		return ret;
	}

	printf("Processing \"%s\" to \"%s\" using \"%s\"\n",
	       i->input_file, i->output_file, i->style_name);

	if ((ret = pwlfmt_process(i)) <= 0) {
		if (pwlfmt_strerror(ret) != NULL)
			fprintf(stderr, "Error: %s\n", pwlfmt_strerror(ret));
		else
			fprintf(stderr, "Error during processing: %s\n",
			        strerror(-ret));
		return ret;
	}

	return 1;
}

//-----------------------------------------------------------------------------
static bool get_options(int *argc, const char ***argv,
    struct pwlfmt_workspace *i)
{
	struct HXoption options_table[] = {
		{.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
		 .help = "Show version information"},
		{.sh = 'i', .type = HXTYPE_STRING, .ptr = &i->input_file,
		 .help = "Input file", .htyp = "file"},
		{.sh = 'o', .type = HXTYPE_STRING, .ptr = &i->output_file,
		 .help = "Output file", .htyp = "file"},
		{.sh = 's', .type = HXTYPE_STRING, .ptr = &i->style_name,
		 .help = "Style to use"},
		{.sh = 't', .type = HXTYPE_STRING, .ptr = &i->template_file,
		 .help = "Template for style (if applies)", .htyp = "file"},
		{.sh = 'w', .type = HXTYPE_NONE, .ptr = &List_styles,
		 .help = "Show available styles"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};

	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi)
{
	printf("Vitalnix " PACKAGE_VERSION " pwlfmt\n");
	exit(EXIT_SUCCESS);
}
