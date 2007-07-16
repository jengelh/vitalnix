/*
 *	lpacct/global.h
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2003 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
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
