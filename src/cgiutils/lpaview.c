/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006 - 2007
    This code is released under version 2.1 of the GNU LGPL.

    lpaview - Web interface for accounting stats
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include <mysql.h>
#include <vitalnix/config.h>
#include <vitalnix/libvxcgi/libvxcgi.h>
#include <vitalnix/libvxutil/libvxutil.h>
#include "lpacct/drop.h"
#include "lpacct/lpacct.h"

// Functions
static void do_lpaview(const char *, struct HXbtree *);
static void print_table(const char *, struct HXbtree *, MYSQL_RES *);
static void costf_add(struct costf *, const struct costf *);
static void header(void);
static void form(const char *);
static void footer(void);

// Variables
static const char *const Wrong_auth =
    "<p class=\"red\"><b>Wrong username and/or password!</b></p>";

//-----------------------------------------------------------------------------
int main(int argc, const char **argv)
{
    struct HXbtree *data = vxcgi_split(vxcgi_read_data(argc, argv));
    const char *user = vxutil_azstr(HXbtree_get(data, "user")),
               *pw   = vxutil_azstr(HXbtree_get(data, "pw"));

    header();

    if(*user == '\0' || *pw == '\0')
        form(user);
    else if(!vxutil_valid_username(user))
        printf("%s", Wrong_auth);
    else if(vxcgi_authenticate(user, pw))
        do_lpaview(user, data);
    else
        printf("%s", Wrong_auth);

    footer();
    printf("\n");
    HXbtree_free(data);
    return EXIT_SUCCESS;
}

static void do_lpaview(const char *user, struct HXbtree *data)
{
    struct options p;
    hmc_t *query = NULL;
    MYSQL_RES *res;
    MYSQL *conn;

    lpacct_readconfig(&p);
    if((conn = lpacct_sql_start(&p)) == NULL) {
        printf("Error reading config: %s\n", strerror(errno));
        goto out;
    }

    setuid(65534);
    setgid(65534);

    query = hmc_sinit("select ");
    hmc_strcat(&query, lpacct_sql_fields);
    hmc_strcat(&query, " from printlog;");
    if(mysql_query(conn, query) != 0) {
        printf("mysql_query: %s\n", mysql_error(conn));
        goto out;
    }
    if((res = mysql_use_result(conn)) == NULL) {
        printf("mysql_use_result: %s\n", mysql_error(conn));
        goto out;
    }

    print_table(user, data, res);
    mysql_free_result(res);

 out:
    hmc_free(query);
    mysql_close(conn);
    return;
}

static void print_table(const char *user, struct HXbtree *data, MYSQL_RES *res)
{
    struct costf all_cost = {};
    int details = *vxutil_azstr(HXbtree_get(data, "d")) != '\0';
    MYSQL_ROW row;

    printf(
      "<p><b>Statistics for user \"%s\"</b></p>"
      "<table cellpadding=\"2\" cellspacing=\"1\" border=\"1\""
      " class=\"bordered\"><tr>"
      "<td align=\"center\" class=\"gray\">Queue-JobID</td>"
      "<td align=\"center\" class=\"gray\">Title</td>",
      user);

    if(details)
        printf(
          "<td align=\"center\" class=\"gray\" bgcolor=\"#80FFFF\">C</td>"
          "<td align=\"center\" class=\"gray\" bgcolor=\"#FF80FF\">M</td>"
          "<td align=\"center\" class=\"gray\" bgcolor=\"#FFFF80\">Y</td>"
          "<td align=\"center\" class=\"gray\" bgcolor=\"#C0C0C0\">K</td>");

    printf(
      "<td align=\"center\" class=\"gray\">Ink coverage</td>"
      "<td align=\"center\" class=\"gray\">Pages</td></tr>");

    while((row = mysql_fetch_row(res)) != NULL) {
        struct costf job_cost = {
            .c = strtod(row[4], NULL),
            .m = strtod(row[5], NULL),
            .y = strtod(row[6], NULL),
            .k = strtod(row[7], NULL),
            .t = strtod(row[8], NULL),
            .p = strtol(row[9], NULL, 0),
        };
        printf(
          "<tr><td>%s-%s</td>"
          "<td>%s</td>",
          row[0], // queue
          row[1], // jid
          row[3] // title
        );

        if(details)
            printf(
              "<td align=\"right\">%.2f</td>"
              "<td align=\"right\">%.2f</td>"
              "<td align=\"right\">%.2f</td>"
              "<td align=\"right\">%.2f</td>",
              job_cost.c, job_cost.m, job_cost.y, job_cost.k);

        printf(
          "<td align=\"right\">%.2f</td>"
          "<td align=\"right\">%u</td>"
          "</tr>",
          job_cost.t, job_cost.p);

        costf_add(&all_cost, &job_cost);
    }

    printf("<tr><td colspan=\"2\">Total</td>");
    if(details)
        printf(
          "<td align=\"right\">%.2f</td>"
          "<td align=\"right\">%.2f</td>"
          "<td align=\"right\">%.2f</td>"
          "<td align=\"right\">%.2f</td>",
          all_cost.c, all_cost.m, all_cost.y, all_cost.k);

    printf(
      "<td align=\"right\">%.2f</td>"
      "<td align=\"right\">%u</td>"
      "</tr></table>",
      all_cost.t, all_cost.p);

    return;
}

//-----------------------------------------------------------------------------
static void costf_add(struct costf *out, const struct costf *in)
{
    out->c += in->c;
    out->m += in->m;
    out->y += in->y;
    out->k += in->k;
    out->t += in->c + in->m + in->y + in->k;
    out->p += in->p;
    return;
}

static void header(void)
{
    printf(
      "Content-Type: text/html\n\n"
      "<html>"
      "<head>"
      "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">"
      "<link rel=\"stylesheet\" href=\"/intranet.css\" type=\"text/css\">"
      "</head>"

      "<body>"

      "<div align=\"center\">"
      "<p style=\"font-size: 16pt;\"><b>Query print accounting status</b></p>"
    );
    return;
}

static void form(const char *user)
{
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
          "<td align=\"right\">Show CMYK coverage</td>"
          "<td>&nbsp;</td>"
          "<td><input type=\"checkbox\" name=\"d\" value=\"1\" /></td>"
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

static void footer(void)
{
    printf(
      "</div>"
      "</body>"
      "</html>"
    );
    return;
}

//=============================================================================
