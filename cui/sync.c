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
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libHX.h>
#include "include/accdb.h"
#include "cui/data.h"
#include "cui/global.h"
#include "cui/main.h"
#include "cui/sync.h"
#include "cui/xml_in.h"

static int create_home(const char *, long, long);
static inline void free_addl_entry(struct eds_entry *);
static void handle_user(struct adb_user *, struct HXbtree *,
 struct HXdeque *, long *);
static char *make_home(const char *, char *, size_t);
static void update_defer_timer(struct adb_user *, long);

//-----------------------------------------------------------------------------
int sync_prep_dgrp(struct adb_group *grp, const char *name) {
    /* Here, we request a group to which compare the data source against.
    Usually, a group is always the same for a particular input set otherwise
    strange things may happen. You can either enter the group name or a GID.
    (Once there is a non-digit in your input, it is treated as a group
    name.) */
    int eax;

    if(only_digits(name)) {
        grp->gid = strtol(name, NULL, 0);
        grp->gname = NULL;
    } else {
        grp->gid = -1;
        grp->gname = (char *)name;
    }

    if((eax = B_GROUPINFO(grp, grp, 1)) < 0) {
        fprintf(stderr, "Error querying the ACCDB: %s (eax=%d, errno=%d)\n",
         strerror(errno), eax, errno);
        return eax;
    } else if(eax == 0) {
        fprintf(stderr, "Group \"%s\" does not exist\n", name);
        return 0;
    }

    /* After [successful] groupinfo(), dgroup->gname is now a pointer to space
    malloc'ed from within the backend. So, dgroup->gname can safely be used
    after this line. */
    return 1;
}

int sync_read_file(const char *file, struct HXbtree **al, int ft_hint) {
    /* In this part, we stuff all users from the data source into an
    associative array, called "al". Some elements of that array will further be
    changed until it is ready for running through import_ds(). */
    struct eds_entry *bucket;
    char key[MAXSNLEN];
    void *state;
    int eax;

    if((eax = eds_open(file, &state, ft_hint)) <= 0) {
        return eax;
    }

    /* This loop fills the "al" associative array by generating a suited
    (pre-stage) login name. (These login names may already be used by the
    system and must first be fixed up (e.g. by adding a unique postcount
    number), so that there are only unique login name in the user database.) */

    *al = HXbtree_init(HXBT_MAP | HXBT_CKEY | HXBT_CMPFN, stricmp);

    while(1) {
        bucket = malloc(sizeof(struct eds_entry));

        if((eax = eds_read(state, bucket)) < 0) {
            fprintf(stderr, "Warning: Stumbled upon a bogus entry in"
             " Data Source, continuing\n");
            free(bucket);
            continue;
        } else if(eax == 0) {
            // End of list
            free(bucket);
            break;
        }

        complete_pentry(bucket, key, MAXSNLEN);
        HXbtree_add(*al, key, bucket);
    }

    eds_close(state);
    return 1;
}

void sync_compare(struct adb_group *dgroup, struct HXbtree *al,
 struct HXdeque **dl, struct HXbtree **lnlist) 
{
    /* And this is now the comparison algorithm, (half of) the core. While
    traversing the users, we mark used login names by putting them in an array
    (lnlist). Login names of users which are about to be deleted are also
    pushed there, so they do not get reassigned right now, but at the next
    run-through. */
    struct adb_user passwd;

    printf("Comparing EDS against ADB\n");
    printf("%ld user(s) in EDS list\n", al->itemcount);
    *lnlist = HXbtree_init(HXBT_CDATA | HXBT_SCMP);

    /* Sure, I could pack the if(dl != NULL) into the while() loop, but I do
    not do so, for performance reasons. Using the following approach, we only
    need to check for dl != NULL once, rather than for every user. */
    if(dl != NULL) {
        long ugrp = 0, ukeep = 0; // some stats
        void *travp = NULL;

        *dl = HXdeque_init();
        B_USERTRAV_INIT(&travp);

        while(B_USERTRAV(&travp, &passwd)) {
            // Note to self: The passwd list is traversed, not the AL/SDF list.
            HXbtree_add(*lnlist, passwd.lname);

            if((dgroup->gname != NULL && passwd.igrp != NULL &&
             strcmp(dgroup->gname, passwd.igrp) == 0) ||
             (dgroup->gid != -1 && dgroup->gid == passwd.gid)) {
                // Users we look for (only specified group)
                ++ugrp;
                handle_user(&passwd, al, *dl, &ukeep);
            }
        }

        printf("%ld group member(s) found in ADB\n", ugrp);
        printf("    %ld to keep\n", ukeep);
        printf("    %lu to delete\n", (**dl).itemcount);
        printf("%lu to add\n", al->itemcount);
    } else {
        /* If "dl" is NULL, a single user is to be added, in which case nobody
        gets removed (moved from "al" to "dl") at all. We just fill up "lnlist"
        for login name fixup. */
        void *travp;
        B_USERTRAV_INIT(&travp);
        while(B_USERTRAV(&travp, &passwd)) {
            HXbtree_add(*lnlist, passwd.lname);
        }
    }

    return;
}

void sync_fixup_addl(struct HXbtree *al, struct HXbtree *lnlist) {
    // Here comes the fixup loop (other part of the core).
    // DEVELOPER Notes: frees "lnlist"
    void *travp = HXbtrav_init(al, NULL);
    long stat_tries = 0, stat_users = 0;
    struct HXbtree_node *node;
    char sbuf[16];

    if(al->itemcount == 0) { return; }

    printf("Username fixup...\n");
    while((node = HXbtraverse(travp)) != NULL) {
        struct eds_entry *entry = node->data;
        int i = 0;

        sbuf[15] = '\0';
        strncpy(sbuf, entry->lname, 15);

        /* If the fixup'ed login name with the unique number already exists,
        bump the unique number. */
        while(HXbtree_find(lnlist, sbuf) != NULL) {
            fixup_login_name(entry->lname, ++i, sbuf, 16);
            ++stat_tries;
        }

        if(i > 0) { ++stat_users; }
        if(strcmp(entry->lname, sbuf) != 0) {
            HX_strclone(&entry->lname, sbuf);
        }
        HXbtree_add(lnlist, sbuf);
    }

    HXbtrav_free(travp);
    HXbtree_free(lnlist);
    printf("    %ld correction loops\n", stat_tries);
    printf("    %ld name(s) corrected\n", stat_users);
    return;
}

void sync_set_aflush(void) {
    if(Opt.c_aflush == AFLUSH_ON) {
        B_MODCTL(ADB_ADDFLAGS, ADB_SYNC);
    } else if(Opt.c_aflush == AFLUSH_DEFAULT) {
        unsigned long f = B_MODCTL(ADB_ADDFLAGS, 0);
        printf("Notice: AUTOFLUSH is %s\n", (f & ADB_SYNC) ? "on" : "off");
    } else {
        B_MODCTL(ADB_DELFLAGS, ADB_SYNC);
    }
    return;
}

FILE *sync_open_log(const char *file, char mc) {
    char mode[4] = {mc, '\0'};
    FILE *rv;

    if(mc == '\0') { return NULL; }
    if((rv = fopen(file, mode)) == NULL) {
        fprintf(stderr, "Error: Could not open %s: %s\n",
         file, strerror(errno));
        return NULL;
    }

    setvbuf(rv, NULL, _IOLBF, 0);

    // change the number if the format should ever change
    fprintf(rv, "# $logformat 0\n" "# C-Spark %s\n", Opt.progv);
    return rv;
}

int sync_add(struct adb_group *dgroup, struct HXbtree *al, FILE *logp) {
    // Adding the users to the system database
    struct HXbtree_node *node;
    long ic = al->itemcount;
    char *plain_pw;
    void *travp;
    int eax;

    if(ic == 0) { return 1; } // Nothing to do

    if(Opt.master_preadd != NULL) {
        struct HX_repmap catalog[] = {{'n', "%ld", &ic}, {0}};
        char cmd[4 * MAXLNLEN];
        HX_strrep5(Opt.master_preadd, catalog, cmd, sizeof(cmd));
        system(cmd);
    }

    plain_pw = malloc(Opt.i_pswdlen + 1);
    travp = HXbtrav_init(al, NULL);
    while((node = HXbtraverse(travp)) != NULL) {
        struct eds_entry *in = node->data;
        struct adb_user out;
        char uhome[MAXFNLEN];

        memset(&out, 0, sizeof(struct adb_user));
        out.lname = in->lname;
        out.uid   = -1;
        out.gid   = dgroup->gid;
        out.gecos = in->rname;
        out.home  = make_home(in->lname, uhome, MAXFNLEN);
        out.shell = Opt.f_shell;
        out.igrp  = dgroup->gname;
        out.sgrp  = NULL;
        out.xuid   = in->xuid;
        out.sgroup = in->sgroup;

        if(Opt.i_pswdlen < 0) {
            out.passwd = "!";
            out.last_change = 0;
        } else if(Opt.i_pswdlen == 0) {
            out.passwd = "";
            out.last_change = now_in_days();
        } else {
            unsigned long fl = GENPW_ONE_DIGIT | GENPW_ONE_CASE;
            if(Opt.b_pwphon) { fl |= GENPW_PHONEMIC; }
            vx_genpw(plain_pw, Opt.i_pswdlen, fl);
            vx_cryptpw(plain_pw, NULL, Opt.c_crymeth, &out.passwd);
            out.last_change = now_in_days();
        }

        out.keep_min = Opt.default_times.keep_min;
        out.keep_max = Opt.default_times.keep_max;
        out.warn_age = Opt.default_times.warn_age;
        out.expire   = Opt.default_times.expire;
        out.inactive = Opt.default_times.inactive;

        if(Opt.user_preadd != NULL) {
            /* Order of positional parameters is important, as we should stay
            compatible in future. See vetc/spark.cfg or doc/spark_ui.html for
            the assigned order. */
            char cmd[4 * MAXLNLEN];
            struct HX_repmap catalog[] = {
                {'l', NULL, out.lname}, {'n', NULL, out.gecos},
                {'u', "%lu", &out.uid}, {'g', "%lu", &out.gid},
                {'G', NULL, out.igrp},  {'S', NULL, out.sgrp},
                {'h', NULL, out.home},  {'s', NULL, out.shell},
                {0},
            };
            HX_strrep5(Opt.user_preadd, catalog, cmd, sizeof(cmd));
            system(cmd);
        }

        if((eax = B_USERADD(&out)) <= 0) {
            memset(plain_pw, '\0', Opt.i_pswdlen + 1);
            free(plain_pw);
            printf("\n");
            fprintf(stderr, "useradd: Something went wrong. (eax=%d,"
             " errno=%d: %s)\n", eax, errno, strerror(errno));
            HXbtrav_free(travp);
            return eax;
        }

        if(logp == NULL) {
            printf("Added %s with password %s\n", out.lname, plain_pw);
        } else {
            fprintf(logp, "+:%s:%s:%s:%s:%s\n", in->sgroup, in->nname,
             in->vname, out.lname, plain_pw);
        }

        if(Opt.i_pswdlen > 0) { free(out.passwd); }

        // Create home directory and optionally copy skeleton dir
        if(!create_home(out.home, out.uid, out.gid)) {
            printf("\n");
            fprintf(stderr, "Warning: Could not create home directory %s: %s\n",
             out.home, strerror(errno));
        }

        if(Opt.user_postadd != NULL) {
            char cmd[4 * MAXLNLEN];
            struct HX_repmap catalog[] = {
                {'l', NULL, out.lname}, {'n', NULL, out.gecos},
                {'u', "%lu", &out.uid}, {'g', "%lu", &out.gid},
                {'G', NULL, out.igrp},  {'S', NULL, out.sgrp},
                {'h', NULL, out.home},  {'s', NULL, out.shell},
                {0},
            };
            HX_strrep5(Opt.user_postadd, catalog, cmd, sizeof(cmd));
            system(cmd);
        }

        if((--ic & 0x1F) == 0) {
            printf("add: %ld user(s) remaining\n", ic);
        }
    }

    printf("Successfully added %lu user(s)\n", al->itemcount);
    memset(plain_pw, '\0', Opt.i_pswdlen + 1);
    free(plain_pw);
    HXbtrav_free(travp);

    if(Opt.c_aflush == AFLUSH_POSTADD) {
        B_MODCTL(ADB_FLUSH);
    }

    if(Opt.master_postadd != NULL) {
        char cmd[4 * MAXLNLEN];
        struct HX_repmap catalog[] = {{'n', "%lu", &al->itemcount}, {0}};
        HX_strrep5(Opt.master_postadd, catalog, cmd, sizeof(cmd));
        system(cmd);
    }

    return 1;
}

int sync_del(struct HXdeque *dl, FILE *logp) {
    // Deleting the old users
    struct HXdeque_node *travp;
    long ic = dl->itemcount;
    char cdate[MAXSNLEN];
    int eax;

    if(ic == 0) { return 1; } // Nothing to do

    /* We generate a time stamp here so that a possible backup operation will
    write into the same directory for all users to be deleted. */
    now_in_ymdhms(cdate, MAXSNLEN);

    if(Opt.master_predel != NULL) {
        char cmd[4 * MAXLNLEN];
        struct HX_repmap catalog[] =
         {{'d', NULL, cdate}, {'n', "%ld", &ic}, {0}};
        HX_strrep5(Opt.master_predel, catalog, cmd, sizeof(cmd));
        system(cmd);
    }

    travp = dl->first;
    while(travp != NULL) {
        struct adb_user rq;
        char *lname;

        memset(&rq, 0, sizeof(struct adb_user));
        rq.lname = travp->ptr;
        rq.uid = rq.gid = -1;

        if((eax = B_USERINFO(&rq, &rq, 1)) < 0) {
            printf("\n");
            fprintf(stderr, "userinfo: Something went wrong. (eax=%d, "
             "errno=%d: %s)\n", eax, errno, strerror(errno));
            return eax;
        } else if(eax == 0) {
            // This should not happen. Someone deleted the user before we did.
            printf("\n");
            fprintf(stderr, "Warning: Someone deleted \"%s\" before we"
             " did.\n", rq.lname);
            continue;
        }

        if(Opt.user_predel != NULL) {
            char cmd[4 * MAXLNLEN];
            struct HX_repmap catalog[] = {
                {'l', NULL, rq.lname}, {'u', "%lu", &rq.uid},
                {'g', "%lu", &rq.gid}, {'G', NULL, rq.igrp},
                {'h', NULL, rq.home}, {'d', NULL, cdate}, {0},
            };
            HX_strrep5(Opt.user_predel, catalog, cmd, sizeof(cmd));
            system(cmd);
        }
        HX_rrmdir(rq.home);

        // need a copy since travp->ptr will be undefined after userdel
        lname = HX_strdup(travp->ptr);

        // (surprise, we can reuse "rq" without modification)
        if((eax = B_USERDEL(&rq)) < 0) {
            printf("\n");
            fprintf(stderr, "userdel: Something went wrong. (eax=%d, "
             "errno=%d: %s)\n", eax, errno, strerror(errno));
            return eax;
        }

        if(logp != NULL) { fprintf(logp, "-:%s\n", lname); }

        if(Opt.user_postdel != NULL) {
            char cmd[4 * MAXLNLEN];
            struct HX_repmap catalog[] =
             {{'l', NULL, lname}, {'d', NULL, cdate}, {0}};
            HX_strrep5(Opt.user_postdel, catalog, cmd, sizeof(cmd));
            system(cmd);
        }

        free(lname);
        if((--ic & 0x0F) == 0) {
            printf("delete: %ld user(s) remaining\n", ic);
        }
        travp = travp->Next;
    }

    printf("Successfully deleted %lu user(s)\n", dl->itemcount);

    if(Opt.master_postdel != NULL) {
        char cmd[4 * MAXLNLEN];
        struct HX_repmap catalog[] =
         {{'d', NULL, cdate}, {'n', "%lu", &dl->itemcount}, {0}};
        HX_strrep5(Opt.master_postdel, catalog, cmd, sizeof(cmd));
        system(cmd);
    }

    return 1;
}

void sync_free(struct HXbtree *al, struct HXdeque *dl) {
    if(al != NULL) {
        void *travp = HXbtrav_init(al, NULL);
        struct HXbtree_node *node;

        while((node = HXbtraverse(travp)) != NULL) {
            free_addl_entry(node->data);
        }

        HXbtrav_free(travp);
        HXbtree_free(al);
    }

    if(dl != NULL) { HXdeque_free(dl); }
    return;
}

//-----------------------------------------------------------------------------
static int create_home(const char *d, long uid, long gid) {
    if(HX_mkdir(d) <= 0) { return 0; }
    lchown(d, uid, gid);
    chmod(d, (S_IRWXU | S_IRWXG | S_IRWXO) & ~Opt.umask);
    if(Opt.f_skeld != NULL) {
        HX_copy_dir(Opt.f_skeld, d, HXF_UID | HXF_GID | HXF_KEEP, uid, gid);
    }
    return 1;
}

static inline void free_addl_entry(struct eds_entry *e) {
    if(e == NULL) { return; }
    free(e->lname);
    free(e->vname);
    free(e->nname);
    free(e->rname);
    free(e->sgroup);
    free(e->xuid);
    free(e);
    return;
}

static void handle_user(struct adb_user *p, struct HXbtree *al,
 struct HXdeque *dl, long *ukeep)
{
    char buf[MAXLNLEN], *ptr, *rname = HX_strdup(p->gecos);

    if(rname != NULL && (ptr = strchr(rname, ',')) != NULL) {
        *ptr = '\0';
    }

    snprintf(buf, sizeof(buf), "%s\x01%s", rname, p->xuid);

    if(HXbtree_find(al, buf) != NULL) {
        ++*ukeep;
        free_addl_entry(HXbtree_get(al, buf));
        HXbtree_del(al, buf);
        if(Opt.default_times.defer_del > 0 && p->defer_del > 0) {
            // back on EDS list
            update_defer_timer(p, 0);
        }
        // what remains in "al" is the users to add
    } else {
        // removed from EDS list
        if(Opt.default_times.defer_del > 0) {
            if(p->defer_del == 0) {
                update_defer_timer(p, now_in_days() +
                 Opt.default_times.defer_del);
            } else if(now_in_days() >= p->defer_del) {
                // did not reappear on EDS list within timelimit
                HXdeque_push(dl, p->lname);
            }
        } else {
            HXdeque_push(dl, p->lname);
        }
    }

    free(rname);
    return;
}

static char *make_home(const char *ln, char *buf, size_t s) {
    if(Opt.c_split == 1 || strlen(ln) == 1) {
        snprintf(buf, s, "%s/%c/%s", Opt.f_home, *ln, ln);
    } else if(Opt.c_split >= 2) {
        snprintf(buf, s, "%s/%c/%c%c/%s",
         Opt.f_home, *ln, ln[0], ln[1], ln);
    } else {
        snprintf(buf, s, "%s/%s", Opt.f_home, ln);
    }
    return buf;
}

static void update_defer_timer(struct adb_user *user, long day) {
    struct adb_user modify = {
        .uid         = -1,
        .gid         = -1,
        .last_change = -1,
        .keep_min    = -1,
        .keep_max    = -1,
        .warn_age    = -1,
        .expire      = -1,
        .inactive    = -1,
        .defer_del   = day,
    };
    B_USERMOD(user, &modify);
    return;
}

//=============================================================================
