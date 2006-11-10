/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.
*/
#ifndef LPACCT_LPACCT_H
#define LPACCT_LPACCT_H 1

/*
 *      DEFINITIONS
 */
enum colorspace {
    COLORSPACE_GRAY,
    COLORSPACE_CMYK,
    COLORSPACE_CMY,
};

enum {
    ARGP_QUEUE = 0,
    ARGP_JOBID,
    ARGP_USER,
    ARGP_TITLE,
    ARGP_COPIES,
    ARGP_OPTIONS,
    ARGP_FILE,
};

struct options {
    // generic
    unsigned int dpi;
    enum colorspace colorspace;
    unsigned int per_doc_stats, per_page_stats, rasterize;

    // filter
    const char **cups_args;
    unsigned int do_account;
};

/*
 *      FUNCTIONS
 */
extern void pr_exit(const char *, const char *, ...);
extern void pr_warn(const char *, const char *, ...);

#endif // LPACCT_LPACCT_H

//=============================================================================
