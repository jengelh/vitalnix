/*=============================================================================
Vitalnix User Management Suite
clutils/mdpwlfmt.c
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2006
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "vitalnix-config.h"
#include "libvxmdfmt/libvxmdfmt.h"
#include "libvxmdfmt/vtable.h"

// Functions
static int get_options(int *, const char ***, struct pwlfmt_workspace *);
static void list_styles(void);
static int pwlfmt(struct pwlfmt_workspace *);
static void show_version(const struct HXoptcb *);

// Variables
static int List_styles = 0;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct pwlfmt_workspace i = {};
    int ret = EXIT_SUCCESS;

    if(!get_options(&argc, &argv, &i))
        return EXIT_FAILURE;

    if(List_styles)
        list_styles();
    else
        if(pwlfmt(&i) <= 0)
            ret = EXIT_FAILURE;

    free(i.style_name);
    free(i.input_file);
    free(i.output_file);
    free(i.template_file);
    return ret;
}

static void list_styles(void) {
    const struct pwlstyle_vtable *p;
    void *trav = NULL;

    printf("Available styles: (asterisk denotes \"requires template"
     " file\" using -t option)\n");

    while((p = pwlstyles_trav(&trav)) != NULL)
        printf("\t" "%c%-10s   %s\n",
          (p->require_template ? '*' : ' '), p->name, p->desc);

    return;
}

static int pwlfmt(struct pwlfmt_workspace *i) {
    int ret;

    if(i->style_name == NULL || i->input_file == NULL ||
     i->output_file == NULL) {
        fprintf(stderr, "-i, -o and -s options are required.\n");
        return 0;
    }

    if((ret = pwlfmt_new(i)) <= 0) {
        if(ret == -PWLFMT_EEINPUT)
            fprintf(stderr, "Error: %s %s: %s\n", pwlfmt_strerror(ret),
                    i->input_file, strerror(errno));
        else if(ret == -PWLFMT_EEOUTPUT)
            fprintf(stderr, "Error: %s %s: %s\n", pwlfmt_strerror(ret),
                    i->output_file, strerror(errno));
        else if(pwlfmt_strerror(ret) != NULL)
            fprintf(stderr, "Error: %s\n", pwlfmt_strerror(ret));
        else
            fprintf(stderr, "Error: %s\n", strerror(errno));
        return ret;
    }

    printf("Processing \"%s\" to \"%s\" using \"%s\"\n",
           i->input_file, i->output_file, i->style_name);

    if((ret = pwlfmt_process(i)) <= 0) {
        if(pwlfmt_strerror(ret) != NULL)
            fprintf(stderr, "Error: %s\n", pwlfmt_strerror(ret));
        else
            fprintf(stderr, "Error during processing: %s\n", strerror(-ret));
        return ret;
    }

    return 1;
}

//-----------------------------------------------------------------------------
static int get_options(int *argc, const char ***argv,
  struct pwlfmt_workspace *i)
{
    struct HXoption options_table[] = {
        {.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
         .help = "Show version information"},
        {.sh = 'i', .type = HXTYPE_STRING, .ptr = &i->input_file,
         .help = "Input file", .htyp = "FILE"},
        {.sh = 'o', .type = HXTYPE_STRING, .ptr = &i->output_file,
         .help = "Output file", .htyp = "FILE"},
        {.sh = 's', .type = HXTYPE_STRING, .ptr = &i->style_name,
         .help = "Style to use"},
        {.sh = 't', .type = HXTYPE_STRING, .ptr = &i->template_file,
         .help = "Template for style (if applies)", .htyp = "FILE"},
        {.sh = 'w', .type = HXTYPE_NONE, .ptr = &List_styles,
         .help = "Show available styles"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi) {
    printf("Vitalnix " VITALNIX_VERSION " mdpwlfmt\n");
    exit(EXIT_SUCCESS);
}

//=============================================================================
