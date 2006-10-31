#ifndef LPACCT_GLOBAL_H
#define LPACCT_GLOBAL_H 1

#ifndef MMAP_FAILED
#    define MMAP_FAILED ((void *)-1)
#endif
#define __STRINGIFY_EXPAND(s)   #s
#define __STRINGIFY(s)          __STRINGIFY_EXPAND(s)
#define PREFIX                  "[cmyk_acct] "

#endif // LPACCT_GLOBAL_H
