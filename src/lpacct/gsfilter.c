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
/*  ghostscript_init
    @input_file:        gs param -- postscript file to parse
    @pid:               store point for PID
    @dpi:               gs param -- dots per inch

    Starts the GhostScript interpreter on @input_file with @dpi x @dpi
    resolution. Puts the PID of the subprocess into @pid and returns the
    file descriptor for it. Returns -errno on error.
*/
int ghostscript_init(const char *input_file, pid_t *pid, int dpi)
{
    char dpi_string[sizeof("-r3600")];
    const char *argv[] = {
        "gs", "-dBATCH", "-dNOPAUSE", "-dQUIET", "-dSAFER", dpi_string,
        "-sDEVICE=ppmraw", "-sOutputFile=-", input_file, NULL,
    };
    int fd, ret, output_pipe[2];

    if(pipe(output_pipe) != 0) {
        ret = errno;
        fprintf(stderr, PREFIX "%s: pipe() failed: %s\n",
                __func__, strerror(ret));
        goto err;
    }

    if((fd = open(input_file, O_RDONLY)) < 0) {
        ret = errno;
        fprintf(stderr, PREFIX "%s: Could not open %s: %s\n",
                __func__, input_file, strerror(ret));
        goto err;
    }

    snprintf(dpi_string, sizeof(dpi_string), "-r%d", (dpi < 2) ? 2 : dpi);

    if((*pid = fork()) < 0) {
        ret = errno;
        fprintf(stderr, PREFIX "%s: fork() failed: %s\n",
                __func__, strerror(ret));
        goto err;
    } else if(*pid == 0) {
        dup2(fd, STDIN_FILENO);
        dup2(output_pipe[1], STDOUT_FILENO);
        close(fd);
        close(output_pipe[0]);
        close(output_pipe[1]);
        // keep STDERR_FILENO -- user or cups pick it up
        ret = execvp(*argv, (char * const *)argv);
        fprintf(stderr, "%s: execvp() failed: %s\n", __func__, strerror(ret));
        exit(127);
    }

    if(waitpid(*pid, &ret, WNOHANG) > 0) {
        fprintf(stderr, PREFIX "%s: Ghostscript terminated%s, exit status %d\n",
                __func__, !WIFEXITED(ret) ? " abnormally" : "",
                WEXITSTATUS(ret));
        goto err;
    }

    close(fd);
    close(output_pipe[1]);
    return output_pipe[0];

 err:
    close(fd);
    close(output_pipe[0]);
    close(output_pipe[1]);
    return -ret;
}

void ghostscript_exit(pid_t pid)
{
    int status;
    waitpid(pid, &status, 0);
    if(WIFEXITED(status))
        return;
    if(WIFSIGNALED(status))
        fprintf(stderr, "%s: GhostScript terminated abnormally, signal %d\n",
                __func__, WTERMSIG(status));
    else
        fprintf(stderr, "%s: GhostScript terminated abnormally\n", __func__);

    return;
}

//=============================================================================
