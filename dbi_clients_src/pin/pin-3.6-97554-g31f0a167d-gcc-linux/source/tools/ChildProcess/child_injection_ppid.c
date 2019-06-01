/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
#ifdef NDEBUG
# undef NDEBUG
#endif
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

char* myExe = NULL;

void executeChildProcess(int pip)
{
    int my_ppid = getppid();
    size_t res = (int)write(pip, &my_ppid, sizeof(my_ppid));
    assert(res == sizeof(my_ppid));
    close(pip);
}

void executeParentProcess(pid_t child, int pip)
{
    int reported_pid = -1;
    int status = -1;
    int res = (int)read(pip, &reported_pid, sizeof(reported_pid));
    assert(res == sizeof(reported_pid));
    close(pip);
    while (0 > (res = waitpid(child, &status, 0)) && errno == EINTR);
    assert(res == child);
    assert(status == 0);
    printf("my pid %d, ppid reported from child: %d\n", (int)getpid(), reported_pid);
    assert((int)getpid() == reported_pid);
}

void testFork(int shouldExec)
{
    int pip[2];
    int res = pipe(pip);
    assert(res == 0);
    pid_t child = fork();
    assert(child >= 0);
    if (0 == child)
    {
        if (shouldExec)
        {
            char pip_str[16];
            char* argv[3] = {myExe, pip_str, NULL};
            snprintf(pip_str, sizeof(pip_str), "%d", pip[1]);
            close(pip[0]);
            execv(myExe, argv);
            assert(0);
        }
        else
        {
            close(pip[0]);
            executeChildProcess(pip[1]);
            _exit(0);
        }
    }
    else
    {
        close(pip[1]);
        executeParentProcess(child, pip[0]);
    }
}

int main(int argc, char* argv[])
{
    myExe = argv[0];
    if (argc == 2)
    {
        char *endptr;
        errno = 0;
        const int pip = (int)strtol(argv[1], &endptr, 10);
        assert (*endptr == 0 && pip >= 0 && 0 == errno);
        executeChildProcess(pip);
    }
    else
    {
        testFork(0);
        testFork(1);
    }
    return 0;
}
