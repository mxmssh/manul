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
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

pthread_mutex_t mtx =  PTHREAD_MUTEX_INITIALIZER;

void *threadMain(void *arg)
{
    fprintf(stderr, "In second thread: starting\n");

    pthread_mutex_lock(&mtx);
    pthread_mutex_lock(&mtx);

    /* we should not get here */
    assert(0);
}

int main()
{
    struct sigaction act;
    pthread_t thd;
    int i;

    /* make poke signal restartable to reproduce mantis: 4073 */
    for (i = 1; i < NSIG; i++)
    {
        sigaction(i, 0, &act);
        act.sa_flags |= SA_RESTART;
        sigaction(i, &act, 0);
    }

    pthread_create(&thd, NULL, &threadMain, 0);

    fprintf(stderr, "In the main thread: Waiting for second thread to be ready\n");

    /* wait for the thread to be ready */
    while (1)
    {
        sleep(1);
        if (0 == pthread_mutex_trylock(&mtx))
        {
            pthread_mutex_unlock(&mtx);
        }
        else
        {
            // First pthread_mutex_lock in the other thread was executed.
            break;
        }
    }

    /* wait three more seconds to make sure the thread is blocked in the second pthread_mutex_lock */
    sleep(3);

    fprintf(stderr, "In the main thread: Exiting\n");

    exit(0);
}
