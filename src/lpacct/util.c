/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.
*/
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

//-----------------------------------------------------------------------------
void fd_tee(int ifd, int ofd1, int ofd2)
{
    char buffer[4096];
    ssize_t n;

    while((n = read(ifd, buffer, sizeof(buffer))) > 0) {
        write(ofd1, buffer, n);
        if(ofd2 >= 0)
            write(ofd2, buffer, n);
    }

    return;
}

//=============================================================================
