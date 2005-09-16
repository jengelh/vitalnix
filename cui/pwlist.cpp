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
#include <errno.h>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "cui/autorun.h"
#include "cui/data.h"
#include "cui/global.h"
#include "cui/main.h"
#include "cui/pwlist.h"

#define PT_TBL_HEADER  const char *sg
#define PT_TBL_ENTRY   const char *sg, const char *nname, const char *vname, \
                       const char *user, const char *pw
#define PT_TBL_FOOTER  const char *sg

enum {
    PH_VERSION  = 'V',
    PH_SGROUP   = 'g',
    PH_IFILE    = 'i',
    PH_OFILE    = 'o',
    PH_NNAME    = 'n',
    PH_PASSWORD = 'p',
    PH_USER     = 'u',
    PH_VNAME    = 'v',
};

#define DEFCAT_FILE_HEADER \
    struct HX_repmap catalog[] = { \
        {PH_IFILE, NULL, (char *)ifile}, \
        {PH_OFILE, NULL, (char *)ofile}, \
        {PH_VERSION, NULL, Opt.progv}, \
        {0}, \
    }
#define DEFCAT_TBL_HEADER \
    struct HX_repmap catalog[] = { \
        {PH_SGROUP, NULL, (char *)sg}, \
        {PH_VERSION, NULL, Opt.progv}, \
        {0}, \
    }
#define DEFCAT_TBL_ENTRY \
    struct HX_repmap catalog[] = { \
        {PH_SGROUP, NULL, (char *)sg}, \
        {PH_NNAME, NULL, (char *)nname}, \
        {PH_PASSWORD, NULL, (char *)pw}, \
        {PH_USER, NULL, (char *)user}, \
        {PH_VNAME, NULL, (char *)vname}, \
        {0}, \
    }

class Style {
  public: // functions
    static void *operator new(size_t);
    virtual ~Style(void);
    int process(struct HXbtree *);
    virtual int read_template(const char *) { return 1; }
    virtual int open_i(const char *);
    virtual int open_o(const char *);

  public: // variables
    FILE *ifp, *ofp;
    char *ifile, *ofile;
    static const int require_template = 0;

  protected: // functions
    virtual void file_header(void) {};
    virtual void tbl_header(PT_TBL_HEADER) {};
    virtual void tbl_entry(PT_TBL_ENTRY) {};
    virtual void tbl_footer(PT_TBL_FOOTER) {};
    virtual void file_footer(void) {};
};

class SG_html : public Style {
  protected: // functions
    void file_header(void);
    void tbl_header(PT_TBL_HEADER);
    void tbl_entry(PT_TBL_ENTRY);
    void tbl_footer(PT_TBL_FOOTER);
    void file_footer(void);
};

class SG_rtf : public Style {
  public: // functions
    ~SG_rtf(void);
    int read_template(const char *);

  public: // variables
    static const int require_template = 1;

  protected: // functions
    void file_header(void);
    void tbl_header(PT_TBL_HEADER);
    void tbl_entry(PT_TBL_ENTRY);
    void tbl_footer(PT_TBL_FOOTER);
    void file_footer(void);

  private: // variables
    char *tpl_data, *tps_file_header, *tps_tbl_header, *tps_tbl_entry,
     *tps_tbl_footer, *tps_file_footer;
};

class SG_txt : public Style {
  protected: // functions
    void tbl_header(PT_TBL_HEADER);
    void tbl_entry(PT_TBL_ENTRY);
    void tbl_footer(PT_TBL_FOOTER);
};

class A1 : public Style {
  protected: // functions
    void file_header(void);
    void tbl_entry(PT_TBL_ENTRY);
};

class SB : public Style {
  public: // functions
    ~SB(void);
    int read_template(const char *);

  public: // variables
    static const int require_template = 1;

  protected: // functions
    void file_header(void);
    void tbl_entry(PT_TBL_ENTRY);
    void tbl_footer(PT_TBL_FOOTER);

  private: // variables
    char *tpl_data;
};

static const struct stme {
    const char *name, *desc;
    Style *style;
} Styles_table[] = {
    {"sg_html", "sgroup-sorted HTML", new SG_html()},
    {"sg_rtf",  "sgroup-sorted RTF",  new SG_rtf()},
    {"sg_txt",  "sgroup-sorted TXT",  new SG_txt()},
    {"a1",      "user-sorted TXT",    new A1()},
    {"sb",      "TEX-Serienbrief",    new SB()},
    {NULL},
};

extern "C" {
    static struct HXbtree *build_tree(FILE *);
    static const struct stme *lookup_style(const char *);
    static char *utf8_to_rtfuni(const char *);
};

//-----------------------------------------------------------------------------
int pwl_proc(struct pwl_param *param) {
    const struct stme *spi;
    struct HXbtree *bt = NULL;
    Style *sp;
    int eax;

    if((spi = lookup_style(param->style)) == NULL) {
        fprintf(stderr, "Style \"%s\" not found!\n", param->style);
        return 0;
    }

    sp = spi->style;
    if((eax = sp->read_template(param->tpl)) < 0) {
        perror("Style::read_template()");
        goto out;
    } else if(eax == 0) {
        goto out;
    }

    if((eax = sp->open_i(param->in)) <= 0) {
        printf("Could not open %s: %s\n", param->in, strerror(errno));
        goto out;
    }

    printf("Processing \"%s\" to \"%s\" using \"%s\"\n",
     param->in, param->out, param->style);

    if((bt = build_tree(sp->ifp)) == NULL) {
        eax = -errno;
        fprintf(stderr, "build_tree(): %s\n", strerror(errno));
        goto out;
    }

    if((eax = sp->open_o(param->out)) <= 0) {
        printf("Could not open %s: %s\n", param->out, strerror(errno));
        goto out;
    }

    sp->ifile = param->in;
    sp->ofile = param->out;
    eax = sp->process(bt);
 out:
    if(bt != NULL) { HXbtree_free(bt); }
    delete sp;
    printf("\n");
    return eax;
}

void pwl_styles(void) {
    const struct stme *p = Styles_table;
    printf("Available styles: (asterisk denotes \"requires template"
     " file\" using -t option)\n");
    while(p->name != NULL) {
        printf("\t" "%c%-10s   %s\n", (p->style->require_template ? ' ' : '*'),
         p->name, p->desc);
        ++p;
    }
    return;
}

int style_need_tpl(const char *style_name) {
    const struct stme *s;
    if((s = lookup_style(style_name)) == NULL) {
        return -1;
    }
    return s->style->require_template;
}

//-----------------------------------------------------------------------------
static struct HXbtree *build_tree(FILE *ifp) {
    /* This function reads a logfile as generated by the synchronization stage,
    puts it in a B-tree (again) for automatic sorting. */
    struct HXbtree *bt;
    char buf[MAXLNLEN];
    int std = 0;

    bt = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_CMPFN, compare_wbc);
    if(bt == NULL) {
        return NULL;
    }

    while(fgets(buf, MAXLNLEN, ifp) != NULL) {
        char *bufp = buf, *c_type, *c_sgroup;
        struct HXbtree *sgp;

        HX_chomp(buf);
        if(*buf == '\0') { continue; }
        if(*buf == '#') {
            if(!std && strncmp(buf, "# $logformat", 12) == 0 &&
             strcmp(buf + 12, " " LOGFMT_VERSION) != 0) {
                fprintf(stderr, "Input file is not of version \"" LOGFMT_VERSION "\"\n");
                HXbtree_free(bt);
                return 0;
            }
            continue;
        }

        // Won't take any $logformat into account now since data has started.
        ++std;
        c_type   = HX_strsep(&bufp, ":");
        c_sgroup = HX_strsep(&bufp, ":");
        if(*c_type != '+' || c_sgroup == NULL || bufp == NULL) {
            continue;
        }

        // Only print users which were added ('+')
        if((sgp = HXbtree_get<HXbtree *>(bt, c_sgroup)) == NULL) {
            sgp = HXbtree_init(HXBT_CDATA | HXBT_CMPFN, strcoll);
            HXbtree_add(bt, c_sgroup, sgp);
        }
        HXbtree_add(sgp, bufp);
    }

    return bt;
}

static const struct stme *lookup_style(const char *name) {
    const struct stme *p = Styles_table;
    while(p->name != NULL) {
        if(stricmp(p->name, name) == 0) {
            return p;
        }
        ++p;
    }
    return NULL;
}

static char *utf8_to_rtfuni(const char *ip) {
    iconv_t cd = iconv_open("wchar_t", "UTF-8");
    char *dest = hmc_minit(NULL, 0);
    const char *cfh = ip;
    char buf[16];
    size_t is, os;
    wchar_t oc, *op = &oc;

    while(*ip != '\0') {
        if(*(const unsigned char *)ip < 0x80) {
            ++ip;
            continue;
        }

        is = strlen(ip);
        os = sizeof(wchar_t);
        hmc_memcat(&dest, cfh, ip - cfh);
        iconv(cd, (char **)&ip, &is, (char **)&op, &os);
        snprintf(buf, sizeof(buf), "\\uc0\\u%d", oc);
        hmc_strcat(&dest, buf);
        cfh = ip;
    }

    if(*cfh != '\0') {
        hmc_strcat(&dest, cfh);
    }
    return dest;
}

//-----------------------------------------------------------------------------
void *Style::operator new(size_t s) {
    return calloc(1, s);
}

int Style::process(struct HXbtree *tl_tree) {
    void *tl_trav = HXbtrav_init(tl_tree, NULL);
    struct HXbtree_node *tl_node;

    if(tl_trav == NULL) {
        fprintf(stderr, "%s: Initializing traverser failed: %s\n",
         __PRETTY_FUNCTION__, strerror(errno));
        return -errno;
    }

    file_header();
    while((tl_node = HXbtraverse(tl_trav)) != NULL) {
        struct HXbtree *sg_tree = static_cast<struct HXbtree *>(tl_node->data);
        const char *sg_name     = static_cast<const char *>(tl_node->key);
        void *sg_trav           = HXbtrav_init(sg_tree, NULL);
        struct HXbtree_node *sg_node;

        tbl_header(sg_name);

        while((sg_node = HXbtraverse(sg_trav)) != NULL) {
            char *data[4], *sd = static_cast<char *>(sg_node->key);

            memset(data, 0, sizeof(data));
            if(HX_split5(sd, ":", 4, data) != 4) {
                fprintf(stderr, "%s: Error in logfile and/or tree, during group \"%s\"\n",
                 __PRETTY_FUNCTION__, sg_name);
                continue;
            }
            tbl_entry(sg_name, data[0], data[1], data[2], data[3]);
        }

        tbl_footer(sg_name);
        HXbtrav_free(sg_trav);
        HXbtree_free(sg_tree);
    }

    file_footer();
    HXbtrav_free(tl_trav);
    return 1;
}

Style::~Style(void) {
    if(ifp != NULL) { fclose(ifp); }
    if(ofp != NULL) { fclose(ofp); }
    return;
}

int Style::open_i(const char *input) {
    if((ifp = fopen(input, "r")) == NULL) {
        return -errno;
    }
    return 1;
}

int Style::open_o(const char *output) {
    if((ofp = fopen(output, "w")) == NULL) {
        return -errno;
    }
    return 1;
}

//-----------------------------------------------------------------------------
/* Please do not remove the "Formatted by" messages as they are the only
external advertisements for this software. */

void SG_html::file_header(void) {
    fprintf(ofp, "<html><head><meta http-equiv=\"Content-Type\""
     " content=\"text/html; charset=UTF-8\">"
     "<head><title>%s</title><style type=\"text/css\">"
     "h1 { font: bold 16pt \"Arial\", sans-serif; }\n"
     "th { font: bold 12pt \"Arial\", sans-serif; text-align: left;"
     " border-bottom: 1.5pt solid #000000; }\n"
     ".serif { font: 12pt \"Times New Roman\", \"Times\", serif; }\n"
     ".sserif { font: 12pt \"Verdana\", \"Tahoma\", \"Arial\", sans-serif; }\n"
     "</style><body>", ifile);
    return;
}

void SG_html::tbl_header(PT_TBL_HEADER) {
    fprintf(ofp, "<p>&nbsp;</p><table width=\"100%%\"><tr><td>"
     "<h1>Group %s</h1></td>"
     "<td align=\"right\"><i>Formatted by Vitalnix C-Spark %s</i></td></tr>"
     "</table><table cellpadding=\"5\" width=\"100%%\"><tr>"
     "<th width=\"50%%\">Name</th><th>Login</th><th>Password</th></tr>\n",
     sg, Opt.progv);
    return;
}

void SG_html::tbl_entry(PT_TBL_ENTRY) {
    char buf[MAXSNLEN];
    struct HX_repmap catalog[] = {
        {'n', NULL, buf},
        {'p', NULL, (void *)pw},
        {'u', NULL, (void *)user},
        {0},
    };
    snprintf(buf, MAXSNLEN, "%s, %s", nname, vname);
    HX_fstrrep(ofp, "<tr><td>%n</td><td class=\"sserif\">%u</td>"
     "<td class=\"sserif\">%p</td></tr>\n", catalog);
    return;
}

void SG_html::tbl_footer(PT_TBL_FOOTER) {
    fprintf(ofp, "</table>");
    return;
}

void SG_html::file_footer(void) {
    fprintf(ofp, "</body></html>");
    return;
}

//-----------------------------------------------------------------------------
SG_rtf::~SG_rtf(void) {
    delete tpl_data;
    return;
}

int SG_rtf::read_template(const char *tpf) {
    char *ptr;
    if(tpf == NULL) {
        fprintf(stderr, "sg_rtf style requires a template! (-t option)\n");
        return 0;
    }

    if((tpl_data = read_file(tpf)) == NULL) {
        fprintf(stderr, "%s: Could not open template file %s: %s\n",
         __PRETTY_FUNCTION__, tpf, strerror(errno));
        return 0;
    }

    /* The following fixup is so that the RTF file can be edited more easily.
    And of course, we keep it in one file, which is essential. Microsoft Office
    successfully ignores our private tags, so that we can check out what our
    template looks like without having to wade through. */
    ptr             = tpl_data;
    tps_file_header = HX_strsep2(&ptr, "{\\THDR}");
    tps_tbl_header  = HX_strsep2(&ptr, "{\\ENTRY}");
    tps_tbl_entry   = HX_strsep2(&ptr, "{\\TFOOT}");
    tps_tbl_footer  = HX_strsep2(&ptr, "{\\FOOTER}");
    tps_file_footer = ptr;
    if(tps_file_header == NULL) { tps_file_header = ""; }
    if(tps_tbl_header  == NULL) { tps_tbl_header  = ""; }
    if(tps_tbl_entry   == NULL) { tps_tbl_entry   = ""; }
    if(tps_tbl_footer  == NULL) { tps_tbl_footer  = ""; }
    if(tps_file_footer == NULL) { tps_file_footer = ""; }
    return 1;
}

void SG_rtf::file_header(void) {
    DEFCAT_FILE_HEADER;
    HX_fstrrep(ofp, tps_file_header, catalog);
    return;
}

void SG_rtf::tbl_header(PT_TBL_HEADER) {
    DEFCAT_TBL_HEADER;
    HX_fstrrep(ofp, tps_tbl_header, catalog);
    return;
}

void SG_rtf::tbl_entry(PT_TBL_ENTRY) {
    char *uni_nname = utf8_to_rtfuni(nname);
    char *uni_vname = utf8_to_rtfuni(vname);

    struct HX_repmap catalog[] = {
        {PH_SGROUP,   NULL, (char *)sg},
        {PH_NNAME,    NULL, (char *)uni_nname},
        {PH_PASSWORD, NULL, (char *)pw},
        {PH_USER,     NULL, (char *)user},
        {PH_VNAME,    NULL, (char *)uni_vname},
        {0},
    };

    HX_fstrrep(ofp, tps_tbl_entry, catalog);
    hmc_free(uni_nname);
    hmc_free(uni_vname);
    return;
}

void SG_rtf::tbl_footer(PT_TBL_FOOTER) {
    fprintf(ofp, tps_tbl_footer);
    return;
}

void SG_rtf::file_footer(void) {
    fprintf(ofp, tps_file_footer);
    return;
}

//-----------------------------------------------------------------------------
void SG_txt::tbl_header(PT_TBL_HEADER) {
    fprintf(ofp, "\n\n"
     "                                  Formatted by Vitalnix C-Spark %s\n\n\n"
     "  >>> Group %s\n\n"
     "  %-35s  %-10s  %s\n"
     "  ====================================="
     "==================================\n",
     Opt.progv, sg, "Name", "Login", "Password");
    return;
}

void SG_txt::tbl_entry(PT_TBL_ENTRY) {
    char buf[MAXSNLEN];
    snprintf(buf, MAXSNLEN, "%s, %s", nname, vname);
    fprintf(ofp, "  %-35s  %-10s  %s\n", buf, user, pw);
    return;
}

void SG_txt::tbl_footer(PT_TBL_FOOTER) {
    fputc(0x0C, ofp);
    return;
}

//-----------------------------------------------------------------------------
void A1::file_header(void) {
    fprintf(ofp, "\n\n"
     "                                  Formatted by Vitalnix C-Spark %s\n\n\n"
     "  %-8s  %-35s  %-10s  %s\n"
     "  ====================================="
     "==================================\n",
     Opt.progv, "Group", "Name", "Login", "Password");
    return;
}

void A1::tbl_entry(PT_TBL_ENTRY) {
    char buf[MAXSNLEN];
    snprintf(buf, MAXSNLEN, "%s, %s", nname, vname);
    fprintf(ofp, "  %-8s  %-35s  %-10s  %s\n", sg, buf, user, pw);
    return;
}

//-----------------------------------------------------------------------------
SB::~SB(void) {
    delete tpl_data;
    return;
}

int SB::read_template(const char *tpf) {
    /* Unlike SG_rtf, this template should only contain the
    THDR/ENTRY parts! (TFOOT is handled in sb_entry().) */
    if(tpf == NULL) {
        fprintf(stderr, "sb style requires a template! (-t option)\n");
        return 0;
    }

    if((tpl_data = read_file(tpf)) == NULL) {
        fprintf(stderr, "%s: Could not open template file %s: %s\n",
         __PRETTY_FUNCTION__, tpf, strerror(errno));
        return 0;
    }
    return 1;
}

void SB::file_header(void) {
    fprintf(ofp,
      "\\documentclass[12pt]{article}\n\n"
      "\\usepackage{isolatin1}\n"
      "\\usepackage{graphics}\n"
      "\\usepackage{german}\n\n"
      "\\begin{document}\n\n"
    );
    return;
}

void SB::tbl_entry(PT_TBL_ENTRY) {
    DEFCAT_TBL_ENTRY;
    HX_fstrrep(ofp, tpl_data, catalog);
    fprintf(ofp, "\\newpage\n");
    return;
}

void SB::tbl_footer(PT_TBL_FOOTER) {
    fprintf(ofp, "\\end{document}\n");
    return;
}

//=============================================================================
