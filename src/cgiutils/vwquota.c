/*=============================================================================
Vitalnix User Management Suite
cgiutils/vwquota.c - Web-frontend for quota information
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2006
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <vitalnix/compiler.h>
#include "libvxcgi/libvxcgi.h"
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>

// Functions
static void do_quota(const char *);
static void header(void);
static void form(const char *);
static void footer(void);

// Variables
static const char *const Wrong_auth =
    "<p class=\"red\"><b>Wrong username and/or password!</b></p>";

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct HXbtree *data = vxcgi_split(vxcgi_read_data(argc, argv));
    const char *user = vxutil_azstr(HXbtree_get(data, "user")),
               *pw   = vxutil_azstr(HXbtree_get(data, "pw"));

    header();

    if(*user == '\0' && *pw == '\0')
        form(user);
    else if(vxutil_valid_username(user))
        printf("%s", Wrong_auth);
    else if(vxcgi_authenticate(user, pw))
        do_quota(user);
    else
        printf("%s", Wrong_auth);

    footer();
    printf("\n");
    HXbtree_free(data);
    return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
static void do_quota(const char *user) {
    long saved = getuid();
    hmc_t *ln = NULL;
    char buf[256];
    FILE *fp;
    int i;

    snprintf(buf, sizeof(buf), "quota_pv \"%s\"", user);
    setuid(0);
    fp = popen(buf, "r");
    seteuid(saved);
    setuid(saved);
    if(fp == NULL)
        return;

    printf(
      "<table cellpadding=\"2\" cellspacing=\"0\" border=\"1\""
      " class=\"bordered\"><tr>"
      "<td align=\"center\" class=\"gray\" rowspan=\"2\">Device</td>"
      "<td align=\"center\" class=\"gray\" colspan=\"5\">Blocks</td>"
      "<td align=\"center\" class=\"gray\" colspan=\"5\">Files</td>"
      "</tr><tr>"
    );

    for(i = 0; i < 2; ++i)
        printf(
          "<td align=\"right\" class=\"gray\">Current Use</td>"
          "<td align=\"right\" class=\"gray\">%%</td>"
          "<td align=\"right\" class=\"gray\">Soft Limit</td>"
          "<td align=\"right\" class=\"gray\">Hard Limit</td>"
          "<td align=\"right\" class=\"gray\">Grace Time</td>"
        );
    printf("</tr>");

    while(HX_getl(&ln, fp) != NULL) {
        unsigned long long dqb_used, dqb_soft, dqb_hard, dqi_used,
                           dqi_soft, dqi_hard;
        char *bover = "", *de_bover = "", *iover = "", *de_iover = "";
        char *field[11];

        memset(field, 0, sizeof(field));
        if(HX_split5(ln, " ", ARRAY_SIZE(field), field) < 10)
            continue;

        dqb_used = strtoull(field[1], NULL, 0);
        dqb_soft = strtoull(field[3], NULL, 0);
        dqb_hard = strtoull(field[4], NULL, 0);
        dqi_used = strtoull(field[6], NULL, 0);
        dqi_soft = strtoull(field[8], NULL, 0);
        dqi_hard = strtoull(field[9], NULL, 0);

        if(*field[2] == '*') { bover = "<b>"; de_bover = "</b>"; }
        if(*field[6] == '*') { iover = "<b>"; de_iover = "</b>"; }

        printf("<tr><td><tt>%s</tt></td>", field[0]);

        // Blocks
        printf("<td align=\"right\">%s%llu%s</td>", bover, dqb_used, de_bover);
        if(dqb_soft == 0)
            printf("<td align=\"right\">-</td>");
        else
            printf("<td align=\"right\">%s%ld%%%s</td>",
             bover, static_cast(long, dqb_used * 100 / dqb_soft), de_bover);
        printf("<td align=\"right\">%llu</td>", dqb_soft);
        printf("<td align=\"right\">%llu</td>", dqb_hard);
        printf("<td align=\"right\">%s%s%s</td>", bover, field[5], de_bover);

        // Files
        printf("<td align=\"right\">%s%llu%s</td>", iover, dqi_used, de_iover);
        if(dqi_soft == 0)
            printf("<td align=\"right\">-</td>");
        else
            printf("<td align=\"right\">%s%ld%%%s</td>",
             iover, static_cast(long, dqi_used * 100 / dqi_soft), de_iover);
        printf("<td align=\"right\">%llu</td>", dqi_soft);
        printf("<td align=\"right\">%llu</td>", dqi_hard);
        printf("<td align=\"right\">%s%s%s</td>", iover, field[9], de_iover);
        printf("</tr>");
    }

    printf("</table>");
    pclose(fp);
    hmc_free(ln);
    return;
}

//-----------------------------------------------------------------------------
static void header(void) {
    printf(
      "Content-Type: text/html\n\n"
      "<html>"
      "<head>"
      "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">"
      "<link rel=\"stylesheet\" href=\"/intranet.css\" type=\"text/css\">"
      "</head>"

      "<body>"

      "<div align=\"center\">"
      "<p>&nbsp;</p>"
      "<p>&nbsp;</p>"
      "<p style=\"font-size: 16pt;\"><b>Query DiskQuota Status</b></p>"
    );
    return;
}

static void form(const char *user) {
    printf(
      "<p>&nbsp;</p>"
      "<form method=\"POST\">"
      "<table style=\"border: 1px dotted #CCCCCC; padding-left: 40px;"
      " padding-right: 40px;\">"
        "<tr>"
          "<td align=\"right\">Username</td>"
          "<td>&nbsp;</td>"
          "<td><input type=\"text\" name=\"user\" size=\"10\""
           " value=\"%s\" /></td>"
        "</tr>"
        "<tr>"
          "<td align=\"right\">Password</td>"
          "<td>&nbsp;</td>"
          "<td><input type=\"password\" name=\"pw\" size=\"10\" /></td>"
        "</tr>"
        "<tr>"
          "<td align=\"center\" colspan=\"3\"><input type=\"submit\""
           " style=\"padding: 0px;\" /></td>"
        "</tr>"
      "</table>"
      "</form>"
      ,
      user
    );
    return;
}

static void footer(void) {
    printf(
      "</div>"
      "</body>"
      "</html>"
    );
    return;
}

//=============================================================================
