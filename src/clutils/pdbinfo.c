/*=============================================================================
Vitalnix User Management Suite
clutils/pdbinfo.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "compiler.h"
#include <vitalnix/config.h>
#include "libvxpdb/libvxpdb.h"
#include "libvxpdb/xafunc.h"
#include "libvxutil/defines.h"

// Functions
static void backend_info(char *);
static void read_ldso_conf(void);
static void read_ldso_conf1(const char *);
static void read_environment(void);

static int get_options(int *, const char ***);
static void show_version(const struct HXoptcb *);

// Variables
static struct HXdeque *Dirs = NULL;
static unsigned int mcount  = 0;
static int OP_open          = 0;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct HXdeque_node *cd;

    Dirs = HXdeque_init();
    HXdeque_push(Dirs, ".");
    read_ldso_conf();
#ifdef CONFIG_LIBDIR
    HXdeque_push(Dirs, CONFIG_LIBDIR);
#endif
    read_environment();

    if(!get_options(&argc, &argv))
        return EXIT_FAILURE;

    backend_info("*");

    for(cd = Dirs->first; cd != NULL; cd = cd->Next) {
        struct HXdir *cdp;
        char *dentry;

        if((cdp = HXdir_open(cd->ptr)) == NULL)
            continue;

        while((dentry = HXdir_read(cdp)) != NULL) {
            char buf[MAXFNLEN];
            if(strncmp(dentry, "drv_", 8) != 0)
                continue;
            snprintf(buf, sizeof(buf), "%s/%s",
                     static_cast(const char *, cd->ptr), dentry);
            backend_info(buf);
        }

        HXdir_close(cdp);
    }

    printf("------------------------------------------------------------\n");
    printf("Read %u modules\n", mcount);
    HXdeque_free(Dirs);
    return EXIT_SUCCESS;
}

static void backend_info(char *fn) {
    struct vxpdb_state *md;
    char sepl = '<', sepr = '>';

    if((md = vxpdb_load(fn)) == NULL && fn[0] == '*' && fn[1] == '\0')
        return;

    printf(
        "------------------------------------------------------------\n"
        "Module %c%s%c:\n",
        sepl, fn, sepr
    );

    if(md == NULL) {
        printf("    ERROR: Could not load module\n");
        return;
    }

    if(fn[0] == '*' && fn[1] == '\0')
        sepl = sepr = '"';

    printf(
        "    Name       : %s\n"
        "    Description: %s\n"
        "    Author     : %s\n",
        md->vtable->name, md->vtable->desc, md->vtable->author
    );

    if(OP_open) {
        vxpdb_open(md, 0);
        vxpdb_close(md);
    }

    vxpdb_unload(md);
    ++mcount;
    return;
}

static void read_ldso_conf(void) {
    char buf[MAXFNLEN], *dentry;
    void *dirp;

    read_ldso_conf1("/etc/ld.so.conf");
    if((dirp = HXdir_open("/etc/ld.so.conf.d")) == NULL)
        return;
    while((dentry = HXdir_read(dirp)) != NULL) {
        snprintf(buf, sizeof(buf), "/etc/ld.so.conf.d/%s", dentry);
        read_ldso_conf1(buf);
    }

    HXdir_close(dirp);
    return;
}

static void read_ldso_conf1(const char *file) {
    hmc_t *ln = NULL;
    FILE *fp;

    if((fp = fopen(file, "r")) == NULL)
        return;

    while(HX_getl(&ln, fp) != NULL) {
        char *q;
        if(*ln != '/')
            continue;
        HX_chomp(ln);
        if((q = strchr(ln, '=')) != NULL)
            *q = '\0';
        HXdeque_push(Dirs, HX_strdup(ln));
    }

    fclose(fp);
    hmc_free(ln);
    return;
}

static void read_environment(void) {
    char *entry, *our, *travp;
    if((entry = getenv("LD_LIBRARY_PATH")) == NULL)
        return;

    HX_strclone(&our, entry);
    travp = our;
    while((travp = HX_strsep(&travp, ":")) != NULL)
        HXdeque_push(Dirs, travp);

    free(our);
    return;
}

static int get_options(int *argc, const char ***argv) {
    struct HXoption options_table[] = {
        {.sh = 'L', .type = HXTYPE_STRDQ, .ptr = Dirs,
         .help = "Additional search directory", .htyp = "dir"},
        {.sh = 'O', .type = HXTYPE_NONE, .ptr = &OP_open,
         .help = "Open the PDB back-end (pdb_open function)"},
        {.sh = 'V', .type = HXTYPE_NONE, .cb = show_version,
         .help = "Show version information"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };
    return HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) > 0;
}

static void show_version(const struct HXoptcb *cbi) {
    printf("Vitalnix " VITALNIX_VERSION " pdbinfo\n");
    exit(EXIT_SUCCESS);
}

//=============================================================================
