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
/*! @file
 * We test two aspects:
   - tls value before and after PIN_Detach()
   - creation new threads while PIN is detached from application

 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/utsname.h>


#define NTHREADS 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned int numOfThreadsReadyForDetach = 0;
unsigned long pinDetached = 0;

extern "C" void TellPinToDetach(unsigned long *updateWhenReady)
{
    return;
}

// Get TLS value
void* ReadTLSBase()
{
    void* tlsBase;
#if defined (TARGET_IA32)
    asm("mov %%gs:0, %%eax\n"
        "mov %%eax, %0"
        : "=r"(tlsBase)
        :
        : "eax");
#else
    asm("mov %%gs:0, %%rax\n"
        "mov %%rax, %0"
        : "=r"(tlsBase)
        :
        : "rax");
#endif
    return tlsBase;
}
/*
 * Compare TLS_BASE values before and after detach.
 */
void * thread_func (void *arg)
{
    unsigned long thread_no = (unsigned long)arg+1;

    void* tlsBase = 0;
    tlsBase = ReadTLSBase();
    pthread_mutex_lock(&mutex);
    numOfThreadsReadyForDetach++;
    pthread_mutex_unlock(&mutex);

    while (!pinDetached)
    {
        usleep(100000); //0.1 seconds
    }

    void* tlsBaseAfterDetach = 0;
    tlsBaseAfterDetach = ReadTLSBase();
    if (tlsBase != tlsBaseAfterDetach)
    {
        fprintf(stderr, "ERROR in thread %lu: GTLSBASE before detach %p; after detach %p\n",
                thread_no, tlsBase, tlsBaseAfterDetach);
        return (void*)1;
    }
    else
    {
        fprintf(stderr, "tls base in thread %lu: %p\n", thread_no, tlsBase);
    }
    return 0;
}

int main (int argc, char *argv[])
{
    pthread_t h[NTHREADS];

    void* tlsBase = 0;
    tlsBase = ReadTLSBase();
    fprintf(stderr, "tls base in main thread: %p\n", tlsBase);
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_create (&h[i], 0, thread_func, (void *)i);
    }

    /*
     * If the number of threads is big, some threads leave system call "clone"
     * while PIN is detached. This functionality is also tested here.
     */

    TellPinToDetach(&pinDetached);

    void * result[NTHREADS];
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_join (h[i], &(result[i]));
    }
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        if (result[i] != 0)
        {
            fprintf(stderr, "TEST FAILED\n");
            return -1;
        }
    }
    void* tlsBaseAfterDetach = 0;
    tlsBaseAfterDetach = ReadTLSBase();
    if (tlsBase != tlsBaseAfterDetach)
    {
        fprintf(stderr, "ERROR in the main thread: TLS_BASE before detach %p; after detach %p\n",
                tlsBase, tlsBaseAfterDetach);
        return -1;
    }
    fprintf(stderr, "TEST PASSED\n");
    return 0;
}

