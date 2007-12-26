/*
 *	html2man - Minimal HTML to NROFF converter
 *	Copyright © CC Computer Consultants GmbH, 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <libHX.h>
#include <libxml/HTMLparser.h>
#include <vitalnix/compiler.h>

/* Functions */
static void tag_generic(xmlNode *, bool);

/* Variables */
static char *manpage_name, *manpage_section;
static char *manpage_date, *manpage_author, *manpage_title;
static struct HXdeque *tag_b_i_stack;
static struct HXdeque *tag_h_stack;
static char xlat_last;

//-----------------------------------------------------------------------------
static inline int strcmp_1u(const xmlChar *a, const char *b)
{
	return strcmp(reinterpret_cast(const char *, a), b);
}

static inline char *xmlGetProp_2s(xmlNode *p, const char *v)
{
	return reinterpret_cast(char *, xmlGetProp(p,
	       reinterpret_cast(const xmlChar *, v)));
}

static inline void xlat_reset(void)
{
	xlat_last = ' ';
}

static inline bool is_nbsp(const unsigned char *s)
{
	return s[0] == 0xC2 && s[1] == 0xA0;
}

static void xlat_printf_h(const unsigned char *s)
{
	for (; *s != '\0'; ++s) {
		while (xlat_last == ' ' && isspace(*s))
			++s;
		if (*s == '\0')
			break;
		if (is_nbsp(s)) {
			fputc(' ', stdout);
			++s;
		} else if (*s == '\n') {
			xlat_last = ' ';
			fputc(xlat_last, stdout);
			continue;
		} else if (*s == '\\') {
			printf("\\e");
		} else if (*s == '-' || *s == '`') {
			printf("\\%c", *s);
		} else if (*s == '"') {
			printf("\"\"");
		} else {
			fputc(*s, stdout);
		}
		xlat_last = *s;
	}
}

static void xlat_printf(const unsigned char *s)
{
	if (tag_h_stack->items > 0) {
		xlat_printf_h(s);
		return;
	}
	for (; *s != '\0'; ++s) {
		while (xlat_last == ' ' && isspace(*s))
			++s;
		if (*s == '\0')
			break;
		if (is_nbsp(s)) {
			fputc(' ', stdout);
			++s;
		} else if (*s == '\n') {
			xlat_last = ' ';
			fputc(xlat_last, stdout);
			continue;
		} else if (*s == '\\') {
			printf("\\e");
		} else if (*s == '-' || *s == '`') {
			printf("\\%c", *s);
		} else {
			fputc(*s, stdout);
		}
		xlat_last = *s;
	}
}

static void tag_b(xmlNode *ptr, bool print_text)
{
	printf("\\fB");
	HXdeque_push(tag_b_i_stack, "\\fB");
	tag_generic(ptr, print_text);
	HXdeque_pop(tag_b_i_stack);
	if (tag_b_i_stack->last == NULL)
		printf("\\fR");
	else
		printf("%s", static_cast(const char *, tag_b_i_stack->last->ptr));
}

static void tag_br(xmlNode *ptr, bool print_text)
{
	if (tag_h_stack->items > 0)
		printf("\"\n%s\"",
		       static_cast(const char *, tag_h_stack->last->ptr));
	else
		printf("\n.sp 0\n");
	xlat_reset();
	tag_generic(ptr, print_text);
}

static void tag_i(xmlNode *ptr, bool print_text)
{
	printf("\\fI");
	HXdeque_push(tag_b_i_stack, "\\fI");
	tag_generic(ptr, print_text);
	HXdeque_pop(tag_b_i_stack);
	if (tag_b_i_stack->last == NULL)
		printf("\\fR");
	else
		printf("%s", static_cast(const char *, tag_b_i_stack->last->ptr));
}

static void tag_h1(xmlNode *ptr, bool print_text)
{
	printf(".SH \"");
	HXdeque_push(tag_h_stack, ".SH ");
	tag_generic(ptr, true);
	xlat_reset();
	HXdeque_pop(tag_h_stack);
	printf("\"\n");
}

static void tag_h2(xmlNode *ptr, bool print_text)
{
	printf(".SS \"");
	HXdeque_push(tag_h_stack, ".SS ");
	tag_generic(ptr, true);
	xlat_reset();
	HXdeque_pop(tag_h_stack);
	printf("\"\n");
}

static void tag_p(xmlNode *ptr, bool print_text)
{
	printf(".PP\n");
	xlat_reset();
	tag_generic(ptr, true);
	xlat_reset();
	printf("\n");
}

static void tag_tr(xmlNode *ptr, bool print_text)
{
	printf(".TP\n");
	for (ptr = ptr->children; ptr != NULL; ptr = ptr->next) {
		if (ptr->type != XML_ELEMENT_NODE)
			continue;
		tag_generic(ptr, true);
		xlat_reset();
		printf("\n");
	}
}

static const struct tag_map {
	const char *name;
	void (*func)(xmlNode *, bool);
} tag_table[] = {
	{"b",  tag_b},
	{"br", tag_br},
	{"h1", tag_h1},
	{"h2", tag_h2},
	{"i",  tag_i},
	{"p",  tag_p},
	{"tr", tag_tr},
	{NULL, tag_generic},
};

static void tag_generic(xmlNode *ptr, bool print_text)
{
	const struct tag_map *tg_lookup;

	for (ptr = ptr->children; ptr != NULL; ptr = ptr->next) {
		if (print_text && ptr->type == XML_TEXT_NODE) {
			xlat_printf((const unsigned char *)ptr->content);
			continue;
		}
		for (tg_lookup = tag_table; ; ++tg_lookup)
			if (tg_lookup->name == NULL) {
				tag_generic(ptr, print_text);
				break;
			} else if (strcmp(signed_cast(const char *, ptr->name),
			    tg_lookup->name) == 0) {
				(*tg_lookup->func)(ptr, print_text);
				break;
			}
	}
}

static bool readhtml(const char *file)
{
	xmlNode *ptr;
	xmlDoc *doc;
	
	if ((doc = htmlParseFile(file, "utf-8")) == NULL)
		return false;
	ptr = xmlDocGetRootElement(doc);
	if (ptr != NULL)
		tag_generic(ptr, false);
	xmlFreeDoc(doc);
	return ptr != NULL;
}

static bool get_options(int *argc, const char ***argv)
{
	struct HXoption options_table[] = {
		{.sh = 'a', .type = HXTYPE_STRING, .ptr = &manpage_author,
		 .help = "Author for manpage"},
		{.sh = 'd', .type = HXTYPE_STRING, .ptr = &manpage_date,
		 .help = "Date for manpage"},
		{.sh = 'n', .type = HXTYPE_STRING, .ptr = &manpage_name,
		 .help = "Short name for manpage"},
		{.sh = 's', .type = HXTYPE_STRING, .ptr = &manpage_section,
		 .help = "Section for manpage"},
		{.sh = 't', .type = HXTYPE_STRING, .ptr = &manpage_title,
		 .help = "Title for manpage", .htyp = "string"},
		HXOPT_AUTOHELP,
		HXOPT_TABLEEND,
	};
	return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static inline const char *asciiz(const char *s)
{
	return (s == NULL) ? "" : s;
}

int main(int argc, const char **argv)
{
	int ret;

	if (!get_options(&argc, &argv))
		return EXIT_FAILURE;

	printf(".TH \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"\n",
	       asciiz(manpage_name), asciiz(manpage_section),
	       asciiz(manpage_date), asciiz(manpage_author),
	       asciiz(manpage_title));
	tag_b_i_stack = HXdeque_init();
	tag_h_stack   = HXdeque_init();
	HXdeque_push(tag_b_i_stack, "\\fR");
	xlat_reset();
	ret = readhtml(argv[1]) ? EXIT_SUCCESS : EXIT_FAILURE;
	HXdeque_free(tag_b_i_stack);
	HXdeque_free(tag_h_stack);
	return ret;
}
