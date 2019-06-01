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
// Application that when running under Pin, may reproduce a bug in OS X* kernel that
// causes wait*() calls to inproperly fail with ECHILD.
#ifndef NDEBUG
# undef NDEBUG
#endif
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
using namespace std;

pid_t executeProcess(int closePip, int* outPip)
{
    int pip[2];
    int res = pipe(pip);
    assert(0 == res);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0)
    {
        // child process
        close(pip[1]);
        if (closePip != -1)
        {
            close(closePip);
        }
        do
        {
            char c;
            res = (int)read(pip[0], &c, sizeof(c));
        }
        while (res > 0 || errno == EINTR);
        assert(res == 0);
        close(pip[0]);
        _exit(0);
    }
    close(pip[0]);
    *outPip = pip[1];
    return pid;
}

void* WaitPidThread(void* arg)
{
    pid_t pid = (pid_t)(uintptr_t)arg;

    int status;
    pid_t ret;
    while (0 > (ret = waitpid(pid, &status, 0)) && errno == EINTR);
    if (ret < 0)
    {
        cout << "waitpid() failed with errno " << errno << endl;
        abort();
    }
    else if (pid != ret)
    {
        cout << "waitpid() waited for the wrong process" << endl;
        abort();
    }
    else if (status !=0)
    {
        cout << "Parent report: Child process failed. Status of the child process is "<< WEXITSTATUS(status) << endl;
        abort();
    }

    return (void*)123;
}

void* WaitIdThread(void* arg)
{
    pid_t pid = (pid_t)(uintptr_t)arg;

    siginfo_t siginfo;
    pid_t ret;
    do
    {
        siginfo.si_pid = 0;
    }
    while (0 > (ret = waitid(P_PID, pid, &siginfo, WEXITED)) &&  errno == EINTR);
    if (ret < 0)
    {
        cout << "waitid() failed with errno " << errno << endl;
        abort();
    }
    else if (siginfo.si_pid != pid)
    {
        cout << "waitid() waited for the wrong process" << endl;
        abort();
    }
    else if (siginfo.si_status !=0)
    {
        cout << "Parent report: Child process failed. Status of the child process is "<< WEXITSTATUS(siginfo.si_status) << endl;
        abort();
    }

    return (void*)246;
}

//Wait for a process completion
//Verify it returned the expected exit code

int main(int argc, char * argv[])
{
    int pip1, pip2;
    pthread_t thd1, thd2;
    pid_t pid1 = executeProcess(-1, &pip1);
    pid_t pid2 = executeProcess(pip1, &pip2);

    pthread_create(&thd1, NULL, WaitPidThread, (void*)(uintptr_t)pid1);
    pthread_create(&thd2, NULL, WaitIdThread, (void*)(uintptr_t)pid2);

    for (int i = 0; i < 5000; i += 100)
    {
        usleep(100000);
    }

    close(pip1);
    close(pip2);
    void* ret;
    pthread_join(thd1, &ret);
    assert(ret == (void*)123);
    pthread_join(thd2, &ret);
    assert(ret == (void*)246);

    return 0;
}

