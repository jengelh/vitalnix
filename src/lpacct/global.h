/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006 - 2007
    This code is released under version 2.1 of the GNU LGPL.
*/
#ifndef LPACCT_GLOBAL_H
#define LPACCT_GLOBAL_H 1

#ifndef MMAP_FAILED
#    define MMAP_FAILED ((void *)-1)
#endif
#define __STRINGIFY_EXPAND(s)   #s
#define __STRINGIFY(s)          __STRINGIFY_EXPAND(s)
#define SYSLPREFIX              "vxlpacct"
#define PREFIX                  "[" SYSLPREFIX "] "

#endif // LPACCT_GLOBAL_H
