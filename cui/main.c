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
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libHX.h>
#include "include/accdb.h"
#include "cui/autorun.h"
#include "cui/data.h"
#include "cui/global.h"
#include "cui/main.h"
#include "cui/pwlist.h"
#include "cui/sync.h"
#include "cui/xml_in.h"

enum {
    /*
    MSG
    (notices CQ_ABORT/CQ_EMPTY/CQ_DEFL)
    PROMPT [DEFL] >
    */
    CQ_MSG    = 1 << 0, // show a long help message (like a tooltip)
    CQ_ABORT  = 1 << 1, // allow abort
    CQ_EMPTY  = 1 << 2, // allow empty
    CQ_DEFL   = 1 << 3, // provide default answer
    CQ_PROMPT = 1 << 4, // prompt text
};

static void print_banner(void);
static void init_var(int, const char **);
static void init_applock(void);

static void ui_mainmenu(void);
static int cli_query(char *, size_t, long, ...);

static void ui_sync(void);
static int ui_sync_get_log(char *, size_t);
static int ui_sync_prompt_cont_add(void);
static int ui_sync_prompt_cont_del(void);

static void ui_pwlist(void);
static int ui_pwlist_sty(char *, size_t);

static void ui_single(void);

static flag_t parse_options(int *, const char ***);
static void parse_option_c(const struct HXoptcb *);
static struct pwl_param *parse_option_p(int *, const char ***);
static struct sync_param *parse_option_s(int *, const char ***);
static struct user_param *parse_option_u(int *, const char ***);
static int load_config(const char *);
static int pconfig_aflush(const struct shconf_opt *, const char *);
static int pconfig_crymeth(const struct shconf_opt *, const char *);

// Variables
static int _main_enter_ui = 1;

struct optmap_s Opt = {};
struct adb_module *mm_output = NULL;

//-----------------------------------------------------------------------------
int main(int argc, const char **argv) {
    int eax;
    init_var(argc, argv);
    init_applock();
    print_banner();

    if(!(eax = parse_options(&argc, &argv))) { return !eax; }
    if(_main_enter_ui) { ui_mainmenu(); }
    unlink(Opt.f_lock);
    return EXIT_SUCCESS;
}

static void print_banner(void) {
    if(Opt.nterm) {
        printf(
         "\e[1;32m"">" "\e[0;32m"">" "\e[1;30m""> " // ]]]
         "\e[1;37m""V" "\e[0;37m""italnix" "\n\e[32m -- \e[0m" // ]]]]
        );
    } else {
        printf(">>> Vitalnix\n -- ");
    }

    printf(
      "C-spark %s (%s)\n\n"
      "This program comes with ABSOLUTELY NO WARRANTY; it is free software,\n"
      "you are welcome to redistribute it under certain conditions;\n"
      "for details see the file named \"LICENSE\", which should have\n"
      "come with this program.\n\n",
      Opt.progv, Opt.progd
    );
    return;
}

static void init_var(int argc, const char **argv) {
    struct adb_user *df = &Opt.default_times;
    const char *p;

    Opt.progv = "v2.0.4";
    Opt.progd = __DATE__ " " __TIME__;

    if((p = getenv("TERM")) != NULL && (strcmp(p, "linux") == 0 ||
     strcmp(p, "xterm") == 0 || strncmp(p, "screen", 6) == 0 ||
     strcmp(p, "msys") == 0 || strcmp(p, "rxvt") == 0)) {
        ++Opt.nterm;
    }

#ifdef __unix__
    if(!isatty(fileno(stdin)) && !isatty(fileno(stdout)) &&
     (p = getenv("DISPLAY")) != NULL && *p) {
        HX_fsystem(HX_FSYSTEM_ARGV | HX_FSYSTEM_EXEC, "xterm", argv,
         "-geometry", "80x25", "-bg", "black", "-fg", "gray", "-e", NULL);
        // exec() obviously failed when we get here
        perror("HX_fsystem()");
        exit(EXIT_FAILURE);
    }

    if(!isatty(fileno(stdout))) { Opt.nterm = 0; }
#endif

    HX_strclone(&Opt.f_home, "/home");
    HX_strclone(&Opt.f_lock, "/var/run/cspark.pid");
    HX_strclone(&Opt.f_shell, "/bin/bash");
    HX_strclone(&Opt.f_skeld, "/var/lib/empty");
    HX_strclone(&Opt.module_path, "*");
    Opt.b_pwphon  = 1;
    Opt.c_aflush  = AFLUSH_OFF;
    Opt.c_crymeth = CRYPW_BLOWFISH;
    Opt.c_split   = 0;
    Opt.i_pswdlen = 10;
    Opt.umask     = S_IWGRP | S_IWOTH;

    df->keep_min = 0;
    df->keep_max = ADB_KEEPMAX_DEFL;
    df->warn_age = ADB_WARNAGE_DEFL;
    df->expire   = ADB_EXPIRE_NEVER;
    df->inactive = ADB_INACTIVE_NEVER;

    umask(S_IRWXG | S_IRWXO); // 0077
    load_config("/etc/vitalnix/useradd.conf");
    load_config("/etc/vitalnix/ucmsync.conf");
    setlocale(LC_COLLATE, "");
    return;
}

static void init_applock(void) {
#ifndef _WIN32
    char buf[8];
    int fd;

    if((fd = open(Opt.f_lock, O_RDWR | O_CREAT, 0666)) < 0) {
        perror("lockfile open()");
        return;
    }

    if(lockf(fd, F_TLOCK, 0) < 0) {
        perror("C-spark already running? lockf() returned");
        return;
    }

    ftruncate(fd, 0);
    snprintf(buf, sizeof(buf), "%d\n", getpid());
    write(fd, buf, strlen(buf));
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    // never close that file or the lock will be gone
#endif
    return;
}

int B_OPEN(flag_t rw) {
    int eax;

    if((mm_output = adb_load(Opt.module_path, NULL)) == NULL) {
        fprintf(stderr, "Could not load ACCDB back-end: %s\n",
         strerror(errno));
        return -errno;
    }

    if((eax = mm_output->open(mm_output, rw ? ADB_WRLOCK : 0)) <= 0) {
        fprintf(stderr, "Could not open ACCDB back-end: %s"
         " (eax=%d, errno=%d)\n", strerror(errno), eax, errno);
        adb_unload(mm_output);
        return eax;
    }

    return 1;
}

void B_CLOSE(void) {
    if(mm_output != NULL) {
        mm_output->close(mm_output);
        adb_unload(mm_output);
    }
    return;
}

//-----------------------------------------------------------------------------
static int cli_query(char *buf, size_t count, long opts, ...) {
    const char *prompt = NULL;
    char *answer = NULL;
    va_list argp;
    va_start(argp, opts);

    if((opts & (CQ_DEFL | CQ_EMPTY)) == (CQ_DEFL | CQ_EMPTY)) {
        fprintf(stderr, "%s: CQ_DEFL and CQ_EMPTY cannot currently coexist\n",
         __FUNCTION__);
        return -1;
    }

    if(opts & CQ_MSG)    { printf("\n" "%s", va_arg(argp, const char *)); }
    if(opts & CQ_PROMPT) { prompt = va_arg(argp, const char *); }
    if(opts & CQ_ABORT)  { printf("(Use a single dot to abort) "); }
    if(opts & CQ_DEFL)   { printf("(Leave blank to use the default)"); }
    if(opts & CQ_EMPTY)  { printf("(Answer may be empty)"); }
    printf("\n");

    while(1) {
        if(opts & CQ_PROMPT) { printf("%s ", prompt); }
        if(opts & CQ_DEFL)   { printf("[%s] ", answer); }
        printf("> ");
        fflush(stdout);

        if(HX_getl(&answer, stdin) == NULL) {
            return 0;
        }
        HX_chomp(answer);

        if((opts & CQ_ABORT) && answer[0] == '.' && answer[1] == '\0') {
            hmc_free(answer);
            return -1;
        }

        if(*answer == '\0') {
            if(opts & CQ_DEFL) {
                break;
            } else if(!(opts & CQ_EMPTY)) {
                printf("Answer may not be empty!\n");
                continue;
            }
        }

        buf[count-1] = '\0';
        strncpy(buf, answer, count - 1);
        break;
    }

    hmc_free(answer);
    return strlen(buf);
}

static void ui_mainmenu(void) {
    char buf[MAXSNLEN];
    int m_running = 1;

    while(m_running) {
        printf("\n"
          "MAIN MENU\n"
          "=========\n"
          "<2>  Synchronize user lists\n"
          "<4>  Print password list\n"
          "<5>  Add a single user\n"
          "<0>  Quit\n"
          "\n"
          "CHOICE > "
        );

        fflush(stdout);
        fgets(buf, MAXSNLEN, stdin);

        switch(*buf) {
            case '2':
                ui_sync();
                break;
            case '4':
                ui_pwlist();
                break;
            case '5':
                ui_single();
                break;
            case '9':
            case '0':
            case 'q':
                m_running = 0;
                break;
        }
    }

    return;
}

//-----------------------------------------------------------------------------
static void ui_sync(void) {
    struct HXbtree *al = NULL, *lnlist = NULL;
    struct adb_group dgroup;
    struct HXdeque *dl = NULL;
    char buf[MAXFNLEN];
    FILE *logp = NULL;

    printf("\n\n"
      "IMPORT USERS\n"
      "============\n"
    );

    if(B_OPEN(1) <= 0) { return; }

    if(cli_query(buf, sizeof(buf), CQ_MSG | CQ_PROMPT | CQ_ABORT,
     "Enter the group to which compare users from the Data Source with.\n",
     "Destination group") <= 0) {
        printf("No group specified, aborting.\n");
        goto out;
    }

    if(sync_prep_dgrp(&dgroup, buf) <= 0) { goto out; }

    if(cli_query(buf, sizeof(buf), CQ_MSG | CQ_PROMPT | CQ_ABORT,
     "Please enter the pathname to the file serving as input. The file must\n"
     "be in SDF-4, SDF-5 or XML (doc/sp_xmlds.html) format, and its name\n"
     "must correctly end in .sdf or .xml, respectively.\n",
     "Input file") <= 0) {
        printf("No filename specified, aborting.\n");
        goto out;
    }

    if(sync_read_file(buf, &al, eds_derivefromname(buf)) <= 0) { goto out; }

    if(Opt.default_times.defer_del > 0) {
        printf("Deferred Deletion feature enabled (%ld day(s)).\n",
         Opt.default_times.defer_del);
    }
    sync_compare(&dgroup, al, &dl, &lnlist);
    sync_fixup_addl(al, lnlist);

    if(al->itemcount == 0 && dl->itemcount == 0) {
        printf("Already synchronized\n");
        goto out;
    }

    sync_set_aflush();
    if((logp = sync_open_log(buf, ui_sync_get_log(buf, MAXFNLEN))) == NULL) {
        goto out;
    }

    if(al->itemcount > 0) {
        if(ui_sync_prompt_cont_add() && sync_add(&dgroup, al, logp) <= 0) {
            printf("Error during operation. You might need to redo the"
             " Synchronization process\n" "to complete all jobs.\n");
            goto out2;
        }
    } else {
        printf("No new users to add.\n");
    }

    if(dl->itemcount > 0) {
        if(ui_sync_prompt_cont_del() && sync_del(dl, logp) <= 0) {
            printf("Error during operation. You might need to redo the"
             " synchronization process\n" "to complete all jobs.\n");
            //goto out2;
        }
    } else {
        printf("No old users to delete.\n");
    }

 out2:
    /* We actually need to flush, since we have added home directores in /home,
    thus it would be nice to have at least some users have their account
    working. (Well, just keep a backup of your DB files, operations-so-far are
    recorded in the logfile and could thus be reversed easily.)
    Hm, B_CLOSE also does an implicit flush if none has been done yet... */
    B_MODCTL(ADB_FLUSH);

 out:
    if(logp != NULL) { fclose(logp); }
    sync_free(al, dl);
    B_CLOSE();
    return;
}

static int ui_sync_get_log(char *file, size_t count) {
    // Propose a default logfile name
    strncpy(file, "synclog-", count - 1);
    now_in_ymdhms(file + strlen(file), count - strlen(file));
    HX_strlcat(file, ".log", count);

    int eax = cli_query(file, count, CQ_MSG | CQ_PROMPT | CQ_ABORT | CQ_DEFL,
      "Enter a name for a log file containing a preliminary list of added\n"
      "users and their respective plaintext passwords.\n", "Log file"
    );

    if(eax == 0) { printf("No filename given, aborting.\n"); }
    if(eax < 0)  { return 0; }

    if(file_exists(file)) {
        char buf[MAXSNLEN];
        printf("\n"
          "File already exists. Specify what to do: [O]verwrite, "
          "[A]ppend or [C]ancel? (Leave blank to abort.)\n"
          "[Cancel] > "
        );

        fflush(stdout);
        fgets(buf, MAXSNLEN, stdin);
        HX_strlower(buf);
        if(*buf == 'a') { return 'a'; }
        else if(*buf == 'o') { return 'w'; }
        else if(*buf == '\0') {
            printf("No input, aborting.\n");
            return 0;
        } else {
            printf("You have to specify one of 'a', 'c' or 'o'. Aborting.\n");
            return 0;
        }
    }

    return 'w';
}

static int ui_sync_prompt_cont_add(void) {
    char buf[MAXSNLEN];
    printf("\n"
      "Continue with adding users? (Yes/No)\n"
      "[Yes] > "
    );

    fflush(stdout);
    fgets(buf, MAXSNLEN, stdin);
    HX_chomp(buf);
    HX_strlower(buf);

    if(*buf != '\0' && *buf != 'y') {
        printf("Skipping addition (interactive user request)\n");
        return 0;
    }

    return 1;
}

static int ui_sync_prompt_cont_del(void) {
    char buf[MAXSNLEN];
    printf("\n"
      "Continue with deleting users? (Yes/No)\n"
      "[Yes] > "
    );

    fflush(stdout);
    fgets(buf, MAXSNLEN, stdin);
    HX_chomp(buf);
    HX_strlower(buf);

    if(*buf != '\0' && *buf != 'y') {
        printf("Skipping deletion (interactive user request)\n");
        return 0;
    }

    return 1;
}

//-----------------------------------------------------------------------------
static void ui_pwlist(void) {
    char infile[MAXFNLEN], outfile[MAXFNLEN], style[MAXSNLEN], tpl[MAXFNLEN];
    struct pwl_param tmp = {};
    int eax;

    printf("\n\n"
     "PRINT PASSWORD LISTS\n"
     "====================\n"
    );

    if(cli_query(infile, sizeof(infile), CQ_MSG | CQ_PROMPT | CQ_ABORT,
     "Enter the name of the logfile that was created during user list\n",
     "synchronization.\n", "Input log file") <= 0) {
        printf("No filename specified, aborting.\n");
        return;
    }

    if(cli_query(outfile, sizeof(outfile), CQ_MSG | CQ_PROMPT | CQ_ABORT,
     "Enter the name of the file where the formatted (\"beautified\")\n"
     "output should go.\n", "Output file") <= 0) {
        printf("No filename specified, aborting.\n");
        return;
    }

    if(!ui_pwlist_sty(style, MAXFNLEN)) { return; }

    if((eax = style_need_tpl(style)) < 0) {
        printf("Style not found\n");
        return;
    } else if(eax > 0 && cli_query(tpl, sizeof(tpl), CQ_MSG | CQ_PROMPT |
     CQ_ABORT, "This style needs a template file. Please enter the path to "
     "it.\n", "Template file") <= 0) {
        printf("No filename specified, aborting.\n");
        return;
    }

    tmp.in    = infile;
    tmp.out   = outfile;
    tmp.style = style;
    tmp.tpl   = tpl;
    pwl_proc(&tmp);
    return;
}

static int ui_pwlist_sty(char *buf, size_t count) {
    printf("\n"
      "Enter the canoncial name of a function responsible for output\n"
      "formatting. Enter \"?\" (without quotes) to see a list of names.\n"
      "(Leave blank to abort.)\n"
    );

    while(1) {
        printf("Style > ");
        fflush(stdout);
        fgets(buf, count, stdin);
        HX_chomp(buf);
        if(*buf == '?') {
            pwl_styles();
            continue;
        } else if(*buf == '\0') {
            return 0;
        }
        break;
    }

    return 1;
}

//-----------------------------------------------------------------------------
static void ui_single(void) {
    char nname[MAXSNLEN], vname[MAXSNLEN], xuid[MAXSNLEN],
     sysgrp[MAXSNLEN], subgrp[MAXSNLEN];
    struct {
        const char *msg, *prompt;
        char *dest;
        size_t len;
        int flags;
    } q_bday = {
        "If you wish to use a scrambled birthdate as XUID,"
        " enter the date now, otherwise leave blank. Accepted"
        " formats are DD.MM.YYYY, MM/DD/YYYY, YYYY-MM-DD.\n",
        "Birthdate", xuid, MAXSNLEN,
    }, q_xuid = {
        "Enter the XUID of the new user\n",
         "External UID", xuid, MAXSNLEN,
    }, questions[] = {
        {"Enter the last name of the user to be added.\n",
         "Last name", nname, MAXSNLEN, CQ_ABORT},
        {"Enter the first name of the user to be added.\n",
         "First name", vname, MAXSNLEN, CQ_ABORT},
        {"Enter the group name or GID to add the user to.\n",
         "System group", sysgrp, MAXSNLEN, CQ_ABORT},
        {"Enter a user-defined subgroup of the new user (may be empty)\n",
         "Subgroup/Class", subgrp, MAXSNLEN, CQ_ABORT | CQ_EMPTY},
        {NULL},
    }, *travp = questions;
    struct user_param tmp = {};
    int eax;

    printf("\n\n"
     "ADD A SINGLE USER\n"
     "=================\n"
    );

    travp = questions;
    while(travp->msg != NULL) {
        eax = cli_query(travp->dest, travp->len, travp->flags |
         CQ_MSG | CQ_PROMPT, travp->msg, travp->prompt);
        if(eax < 0) { goto out; }
        ++travp;
    }

 enter_bday:
    eax = cli_query(q_bday.dest, q_bday.len, CQ_MSG | CQ_PROMPT | CQ_ABORT |
          CQ_EMPTY, q_bday.msg, q_bday.prompt);

    if(eax < 0) { goto out; }
    if(eax > 0) {
        if(day_to_xuid(q_bday.dest, q_bday.dest, q_bday.len) == NULL) {
            fprintf(stderr, "Illegal date format\n");
            goto enter_bday;
        }
    } else if(cli_query(q_xuid.dest, q_xuid.len, CQ_MSG | CQ_PROMPT |
     CQ_ABORT | CQ_EMPTY, q_xuid.msg, q_xuid.prompt) < 0) {
        goto out;
    }

    tmp.vname = vname;
    tmp.nname = nname;
    tmp.gname = sysgrp;
    if(*xuid != '\0') {
        tmp.xuid = xuid;
    }
    if(*subgrp != '\0') {
        tmp.sgroup = subgrp;
    }
    autorun_user(&tmp);
 out:
    return;
}

//-----------------------------------------------------------------------------
static int load_config(const char *fn) {
    static const struct shconf_opt options_table[] = {
        {"AUTOFLUSH",       SHCONF_CB,     NULL, pconfig_aflush},
        {"DEFAULT_BACKEND", SHCONF_STRING, &Opt.module_path},
        {"DEFERRED_DELETION", SHCONF_LONG, &Opt.default_times.defer_del},
        {"HOME",            SHCONF_STRING, &Opt.f_home},
        {"MASTER_PREADD",   SHCONF_STRING, &Opt.master_preadd},
        {"MASTER_PREDEL",   SHCONF_STRING, &Opt.master_predel},
        {"MASTER_POSTADD",  SHCONF_STRING, &Opt.master_postadd},
        {"MASTER_POSTDEL",  SHCONF_STRING, &Opt.master_postdel},
        {"PASS_EXPIRE",     SHCONF_LONG,   &Opt.default_times.expire},
        {"PASS_INACTIVE",   SHCONF_LONG,   &Opt.default_times.inactive},
        {"PASS_KEEP_MAX",   SHCONF_LONG,   &Opt.default_times.keep_max},
        {"PASS_KEEP_MIN",   SHCONF_LONG,   &Opt.default_times.keep_min},
        {"PASS_WARN_AGE",   SHCONF_LONG,   &Opt.default_times.warn_age},
        {"PSWD_LEN",        SHCONF_INT,    &Opt.i_pswdlen},
        {"PSWD_METH",       SHCONF_CB,     NULL, pconfig_crymeth},
        {"PSWD_PHON",       SHCONF_BOOL,   &Opt.b_pwphon},
        {"SHELL",           SHCONF_STRING, &Opt.f_shell},
        {"SKEL",            SHCONF_STRING, &Opt.f_skeld},
        {"SPLIT_LVL",       SHCONF_UCHAR,  &Opt.c_split},
        {"UMASK",           SHCONF_LONG,   &Opt.umask},
        {"USER_PREADD",     SHCONF_STRING, &Opt.user_preadd},
        {"USER_PREDEL",     SHCONF_STRING, &Opt.user_predel},
        {"USER_POSTADD",    SHCONF_STRING, &Opt.user_postadd},
        {"USER_POSTDEL",    SHCONF_STRING, &Opt.user_postdel},
        {NULL},
    };

    return HX_shconfig(fn, options_table);
}

static flag_t parse_options(int *argc, const char ***argv) {
    int do_p = 0, do_s = 0, do_u = 0;
    struct HXoption options_table[] = {
        {.sh = 'M', .type = HXOPT_STRING, .ptr = &Opt.module_path,
         .help = "Use a different module than \"*\" (the default)"},
        {.sh = 'P', .type = HXOPT_NONE | HXOPT_INC, .ptr = &do_p,
         .help = "Print password list (try \"-P -h\" for more help)"},
        {.sh = 'S', .type = HXOPT_NONE | HXOPT_INC, .ptr = &do_s,
         .help = "Synchronize with list (try \"-S -h\" for more help)"},
        {.sh = 'U', .type = HXOPT_NONE | HXOPT_INC, .ptr = &do_u,
         .help = "Add single user (try \"-U -h\" for more help)"},
        {.sh = 'c', .type = HXOPT_STRING, .cb = parse_option_c,
         .help = "Load alternate config file", .htyp = "file"},
        HXOPT_AUTOHELP,
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv,
     HXOPT_USAGEONERR | HXOPT_PTHRU) <= 0) {
        return 0;
    }

    if(do_p + do_s + do_u > 1) {
        fprintf(stderr, "You can specify only one of -P, -S or -U!\n");
        return 0;
    }
    if(do_p + do_s + do_u > 0) {
        _main_enter_ui = 0;
    }

    if(do_p) {
        struct pwl_param *d = parse_option_p(argc, argv);
        if(d == NULL) {
            return 0;
        }
        autorun_pwl(d);
        free(d->in);
        free(d->out);
        free(d->style);
        free(d->tpl);
        free(d);
    } else if(do_s) {
        struct sync_param *d = parse_option_s(argc, argv);
        if(d == NULL) {
            return 0;
        }
        autorun_sync(d);
        free(d->gname);
        free(d->in);
        free(d->out);
        free(d);
    } else if(do_u) {
        struct user_param *d = parse_option_u(argc, argv);
        if(d == NULL) {
            return 0;
        }
        autorun_user(d);
        free(d->gname);
        free(d->vname);
        free(d->nname);
        free(d->xuid);
        free(d);
    }
    return 1;
}

static void parse_option_c(const struct HXoptcb *cbi) {
    if(!load_config(cbi->s)) {
        fprintf(stderr, "Parsing of config file %s failed\n", cbi->s);
        exit(EXIT_FAILURE);
    }
    return;
}

static struct pwl_param *parse_option_p(int *argc, const char ***argv) {
    struct pwl_param tmp = {};
    struct HXoption options_table[] = {
        {.sh = 'i', .type = HXOPT_STRING, .ptr = &tmp.in,
         .help = "Input file", .htyp = "FILE"},
        {.sh = 'o', .type = HXOPT_STRING, .ptr = &tmp.out,
         .help = "Output file", .htyp = "FILE"},
        {.sh = 's', .type = HXOPT_STRING, .ptr = &tmp.style,
         .help = "Style to use"},
        {.sh = 't', .type = HXOPT_STRING, .ptr = &tmp.tpl,
         .help = "Template for style (if applies)", .htyp = "FILE"},
        {.sh = 'w', .type = HXOPT_NONE,
         .help = "Show available styles"},

        {.sh = 'h', .type = HXOPT_NONE, .cb = HX_getopt_help_cb,
         .help = "Show this help message"},
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0) {
        return NULL;
    }

    return HX_memdup(&tmp, sizeof(tmp));
}

static struct sync_param *parse_option_s(int *argc, const char ***argv) {
    struct sync_param tmp = {};
    struct HXoption options_table[] = {
        {.sh = 'g', .type = HXOPT_STRING, .ptr = &tmp.gname,
         .help = "System group to synchronize against", .htyp = "NAME"},
        {.sh = 'i', .type = HXOPT_STRING, .ptr = &tmp.in,
         .help = "XML Data Source", .htyp = "FILE"},
        {.sh = 'o', .type = HXOPT_STRING, .ptr = &tmp.out,
         .help = "Output log file (for -S)", .htyp = "FILE"},
        {.ln = "no-add", .type = HXOPT_NONE, .ptr = &tmp.no_add,
         .help = "Do not add any users"},
        {.ln = "no-del", .type = HXOPT_NONE, .ptr = &tmp.no_del,
         .help = "Do not delete any users"},

        {.sh = 'h', .type = HXOPT_NONE, .cb = HX_getopt_help_cb,
         .help = "Show this help message"},
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0) {
        return NULL;
    }

    return HX_memdup(&tmp, sizeof(tmp));
}

static struct user_param *parse_option_u(int *argc, const char ***argv) {
    struct user_param tmp = {};
    char *day = NULL;
    struct HXoption options_table[] = {
        {.sh = 'b', .type = HXOPT_STRING, .ptr = &day,
         .help = "Use provided date as XUID", .htyp = "DAY"},
        {.sh = 'g', .type = HXOPT_STRING, .ptr = &tmp.gname,
         .help = "System group to put this user in", .htyp = "GROUP"},
        {.sh = 'n', .type = HXOPT_STRING, .ptr = &tmp.nname,
         .help = "Last name of the user"},
        {.sh = 's', .type = HXOPT_STRING, .ptr = &tmp.sgroup,
         .help = "User-defined textual subgroup string"},
        {.sh = 'v', .type = HXOPT_STRING, .ptr = &tmp.vname,
         .help = "First name of the user"},
        {.sh = 'x', .type = HXOPT_STRING, .ptr = &tmp.xuid,
         .help = "Unique identifier for user", .htyp = "ID"},

        {.sh = 'h', .type = HXOPT_NONE, .cb = HX_getopt_help_cb,
         .help = "Show this help message"},
        HXOPT_TABLEEND,
    };

    if(HX_getopt(options_table, argc, argv, HXOPT_USAGEONERR) <= 0) {
        return NULL;
    }

    if(day != NULL) {
        if(tmp.xuid != NULL) {
            fprintf(stderr, "Cannot specify both -P -b and -P -x!\n");
            return 0;
        }
        tmp.xuid = day_to_xuid(day, NULL, 0);
        free(day);
    }

    return HX_memdup(&tmp, sizeof(tmp));
}

static int pconfig_aflush(const struct shconf_opt *t, const char *val) {
    char *ptr = &Opt.c_aflush;
    if(stricmp(val, "off") == 0)          { *ptr = AFLUSH_OFF;     }
    else if(stricmp(val, "on") == 0)      { *ptr = AFLUSH_ON;      }
    else if(stricmp(val, "postadd") == 0) { *ptr = AFLUSH_POSTADD; }
    else { *ptr = AFLUSH_DEFAULT; }
    return 1;
}

static int pconfig_crymeth(const struct shconf_opt *t, const char *val) {
    char *ptr = &Opt.c_crymeth;
    if(stricmp(val, "md5") == 0) {
#ifdef _WIN32
        fprintf(stderr, "Warning: No MD5 support under Win32.\n");
#else
        *ptr = CRYPW_MD5;
#endif
    } else if(stricmp(val, "des") == 0) {
#ifdef _WIN32
        fprintf(stderr, "Warning: No DES support under Win32.\n");
#else
        *ptr = CRYPW_DES;
#endif
    } else {
        *ptr = CRYPW_BLOWFISH;
    }
    return 1;
}

//=============================================================================
