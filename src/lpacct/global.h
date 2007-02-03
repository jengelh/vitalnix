/*
    lpacct/global.h
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2003 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
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
