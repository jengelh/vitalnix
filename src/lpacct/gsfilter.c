/*
    Copyright © Jan Engelhardt <jengelh@gmx.de>, 2006
    This code is released under version 2.1 of the GNU LGPL.
*/
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "global.h"
#include "gsfilter.h"

//-----------------------------------------------------------------------------
/*  proc_gs
    @input_file:        Input file, in application/postscript
    @output_file:       Output file, either PGM or PPM

    Runs Ghostscript on the file to produce something we can read easily.
*/
int proc_gs(const char *input_file, const char *output_file)
{
    char of_param[256];
    const char *argv[] = {
        "gs",
        "-dBATCH",
        "-dNOPAUSE",
        "-r300",
        "-sDEVICE=ppmraw",
        of_param,
        input_file,
        NULL,
    };
    int s, status;
    pid_t pid;

    snprintf(of_param, sizeof(of_param), "-sOutputFile=%s", output_file);
    if((pid = fork()) < 0) {
        fprintf(stderr, PREFIX "run_gs: fork() failed with %d\n", errno);
        return 0;
    } else if(pid == 0) {
        int null_fd = open("/dev/null", O_WRONLY);
        close(STDOUT_FILENO);
        dup2(STDOUT_FILENO, null_fd);
        execvp(*argv, (char * const *)argv);
        exit(99);
    }

    waitpid(pid, &status, 0);
    if(!WIFEXITED(status)) {
        fprintf(stderr, PREFIX "%s: gs subprocess terminated abnormally\n", __FUNCTION__);
        return 0;
    }
    if((s = WEXITSTATUS(status)) != 0) {
        fprintf(stderr, PREFIX "%s: gs subprocess exited with %d\n", __FUNCTION__, s);
        return 0;
    }
    return 1;
}

//=============================================================================
