/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.
*/
#ifndef LPACCT_UTIL_H
#define LPACCT_UTIL_H 1

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
    unsigned int per_doc_stats, per_page_stats;

    // filter
    const char **cups_args;
    unsigned int do_account;
};

/*
 *      FUNCTIONS
 */

#endif // LPACCT_UTIL_H

//=============================================================================
