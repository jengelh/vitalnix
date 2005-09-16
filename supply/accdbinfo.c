/*=============================================================================
Vitalnix User Management Suite
  Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2003 - 2005
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

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
#include "include/accdb.h"
#include "sysprog/shared.h"

static void backend_info(char *);
static void read_ldso_conf(void);
static void read_environment(void);
static int get_options(int *, const char ***);

static struct HXdeque *Dirs = NULL;
static size_t mcount = 0;
static int OP_open = 0;

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
    struct HXdeque_node *cd;

    Dirs = HXdeque_init();
    HXdeque_push(Dirs, "/lib");
    HXdeque_push(Dirs, "/usr/lib");
    read_ldso_conf();
    HXdeque_push(Dirs, ".");
    read_environment();

    get_options(&argc, (const char ***)&argv);
    backend_info("*");

    cd = Dirs->first;
    while(cd != NULL) {
        struct HXdir *cdp;
        char *dentry;

        if((cdp = HXdir_open(cd->ptr)) == NULL) {
          cd = cd->Next;
          continue;
        }

        while((dentry = HXdir_read(cdp)) != NULL) {
            char buf[MAXFNLEN];
            if(strncmp(dentry, "accdb_", 6) != 0) { continue; }
            snprintf(buf, MAXLNLEN, "%s/%s", (char *)cd->ptr, dentry);
            backend_info(buf);
        }

        HXdir_close(cdp);
        cd = cd->Next;
    }

    printf("------------------------------------------------------------\n");
    printf("Read %u modules\n", (unsigned int)mcount);
    HXdeque_free(Dirs);
    return !1;
}

static void backend_info(char *fn) {
    struct adb_module *md = adb_load(fn, NULL);
    char sepl = '<', sepr = '>';

    if(md == NULL) { return; }
    if(strcmp(fn, "*") == 0) { sepl = sepr = '"'; }

    printf(
      "------------------------------------------------------------\n"
      "Module %c%s%c:\n"
      "    Name       : %s\n"
      "    Description: %s\n",
      sepl, fn, sepr, md->name, md->desc
    );

    if(strchr(md->info, '\n') == NULL) {
        printf("    Info       : %s\n", md->info);
    } else {
        printf("    Info:\n%s", md->info);
    }

    if(OP_open) {
        md->open(md, 0);
        md->close(md);
    }

    adb_unload(md);
    ++mcount;
    return;
}

static void read_ldso_conf(void) {
    char *ln = NULL;
    FILE *fp;

    if((fp = fopen("/etc/ld.so.conf", "r")) == NULL) { return; }
    while(HX_getl(&ln, fp) != NULL) {
        char *q;
        HX_chomp(ln);
        if((q = strchr(ln, '=')) != NULL) { *q = '\0'; }
        HXdeque_push(Dirs, HX_strdup(ln));
    }

    fclose(fp);
    hmc_free(ln);
    return;
}

static void read_environment(void) {
    char *entry, *our, *travp;
    if((entry = getenv("LD_LIBRARY_PATH")) == NULL) { return; }
    HX_strclone(&our, entry);
    travp = our;

    while((travp = HX_strsep(&travp, ":")) != NULL) {
        HXdeque_push(Dirs, travp);
    }

    free(our);
    return;
}

static int get_options(int *argc, const char ***argv) {
    struct HXoption options_table[] = {
        {.sh = 'L', .type = HXOPT_STRDQ, .ptr = Dirs,
         .help = "Additional search directory", .htyp = "dir"},
        {.sh = 'O', .type = HXOPT_NONE, .ptr = &OP_open,
         .help = "Open (am_open) the ACCDB back-end"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, 0) <= 0) {
        struct HXoptcb cbi = {.arg0 = **argv, .table = options_table};
        HX_getopt_help(&cbi, stderr);
        return 0;
    }

    return 1;
}

//=============================================================================
