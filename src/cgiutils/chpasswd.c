/*=============================================================================
Vitalnix User Management Suite
cgiutils/chpasswd.c - Web-frontend for password change
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
#include <unistd.h>
#include <libHX.h>
#include "libvxcgi/libvxcgi.h"
#include "libvxutil/libvxutil.h"

// Functions
static void header(void);
static void form(const char *);
static void footer(void);
static int change_password(const char *, const char *);

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    struct HXbtree *data = vxcgi_split(vxcgi_read_data(argc, argv));
    const char *user     = vxutil_azstr(HXbtree_get(data, "user")),
               *oldpw    = vxutil_azstr(HXbtree_get(data, "oldpw")),
               *newpw    = vxutil_azstr(HXbtree_get(data, "newpw")),
               *newpw2   = vxutil_azstr(HXbtree_get(data, "newpw2"));

    header();

    if(strcmp(user, "") != 0) {
        if(strcmp(oldpw, "") == 0)
            printf("<p class=\"red\"><b>Old password is empty!</b></p>");
        else if(strlen(newpw) < 7)
            printf("<p class=\"red\"><b>New password is too short (at "
                   "least 7 chars)!</b></p>");
        else if(strcmp(newpw, newpw2) != 0)
            printf("<p class=\"red\"><b>Passwords do not match!</b></p>");
        else if(!vxcgi_authenticate(user, oldpw))
            printf("<p class=\"red\"><b>Username and/or password "
                   "invalid!</b></p>");
        else if(!change_password(user, newpw))
            printf("<p class=\"red\"><b>Internal error: /usr/sbin/chpasswd: "
                   "%s</b></p>", strerror(errno));
        else {
            printf("<p class=\"green\"><b>Password change succeeded.</b></p>");
            user = "";
        }
    } else {
        printf("<p>&nbsp;</p>");
    }
    form(user);
    footer();
    printf("\n");
    HXbtree_free(data);
    return EXIT_SUCCESS;
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
      "<p style=\"font-size: 16pt;\"><b>Change Password</b></p>"
    );
    return;
}

static void form(const char *user) {
    printf(
      "<p>&nbsp;</p>"

      "<form method=\"POST\">"

      "<table style=\"border: 1px dotted #CCCCCC; padding-left: 40px;"
      " padding-right: 40px;\">"
      "  <tr>"
      "    <td align=\"right\">Username</td>"
      "    <td>&nbsp;</td>"
      "    <td><input type=\"text\" name=\"user\" size=\"10\""
      "      value=\"%s\" /></td>"
      "  </tr>"
      "  <tr>"
      "    <td align=\"right\">Old Password</td>"
      "    <td>&nbsp;</td>"
      "    <td><input type=\"password\" name=\"oldpw\" size=\"10\" /></td>"
      "  </tr>"
      "  <tr>"
      "    <td align=\"right\">New Password</td>"
      "    <td>&nbsp;</td>"
      "    <td><input type=\"password\" name=\"newpw\" size=\"10\" /></td>"
      "  </tr>"
      "  <tr>"
      "    <td align=\"right\">Reenter New Password</td>"
      "    <td>&nbsp;</td>"
      "    <td><input type=\"password\" name=\"newpw2\" size=\"10\" /></td>"
      "  </tr>"
      "  <tr>"
      "    <td align=\"center\" colspan=\"3\"><input type=\"submit\""
      "      style=\"padding: 0px;\" /></td>"
      "  </tr>"
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

//-----------------------------------------------------------------------------
static int change_password(const char *user, const char *pass) {
    long uid = getuid();
    FILE *cf;

    setuid(0);

    /* chpasswd seems to check UID rather than EUID, that's why there is
    the call to setuid(0); */
    // FIXME: Better do this with libvxpdb
    cf = popen("/usr/sbin/chpasswd -cblowfish", "w");
    setuid(uid);
    seteuid(uid);
    if(cf == NULL)
        return 0;
    fprintf(cf, "%s:%s\n", user, pass);
    pclose(cf);
    return 1;
}

//=============================================================================
