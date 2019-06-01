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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

using namespace std;

extern "C" __declspec(noinline, dllexport) int PinIsAttached(unsigned int numOfThreads)
{
    return 0;
}

const unsigned long num_threads = 10;
HANDLE threadCreatedSemaphore;
HANDLE neverReleasedSemaphore;
volatile bool loop = true;

int ThreadRoutine(LPVOID lpParam)
{
    bool enterEndlessSyscall = *(bool*)lpParam;
    if (!ReleaseSemaphore(
            threadCreatedSemaphore,  // handle to semaphore
            1,            // increase count by one
            NULL) )       // not interested in previous count
    {
        printf("ReleaseSemaphore error: %d\n", GetLastError());
    }

    if (enterEndlessSyscall)
    {   // One thread enter endless system call. In order to check test pass in this case as well
        WaitForSingleObject(
                neverReleasedSemaphore, // handle to semaphore
                INFINITE);              // wait until signaled
    }

    while(loop)
    {
        void * h =  malloc(13);
        if (h)
        {
            free(h);
        }
    }
    return 0;
}

void ThreadCreation()
{

    unsigned long thread_id = 0;
    unsigned long cnt_th = 0;

    fprintf(stderr, "  App: creating %d additional threads \n", num_threads);

    bool firstThread = true;
    for (cnt_th = 0; cnt_th < num_threads; cnt_th++)
    {
        CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadRoutine,&firstThread,0,(LPDWORD)&thread_id);
        WaitForSingleObject(
                threadCreatedSemaphore, // handle to semaphore
                INFINITE);              // wait until signaled
        firstThread = false;
    }

}

int main()
{
    threadCreatedSemaphore = CreateSemaphore(
        NULL,   // default security attributes
        0,      // initial count
        1,      // maximum count
        NULL);  // unnamed semaphore
    neverReleasedSemaphore = CreateSemaphore(
        NULL,   // default security attributes
        0,      // initial count
        1,      // maximum count
        NULL);  // unnamed semaphore

    if ((threadCreatedSemaphore == NULL) || (neverReleasedSemaphore==NULL))
    {
        printf("CreateSemaphore error: %d\n", GetLastError());
        return 1;
    }

    //
    // Create Threads
    //
    ThreadCreation();

    //
    // Ready to be attached by Pin. Notify app launcher it can proceed by releasing the below semaphore
    //
	HANDLE readySemaphore;
	std::ostringstream stream;
	stream << GetCurrentProcessId();
	string semaphoreHandleName = "semaphore_handle_" + stream.str();
    readySemaphore = OpenSemaphore(SYNCHRONIZE|SEMAPHORE_MODIFY_STATE, TRUE, semaphoreHandleName.c_str());
    assert(readySemaphore != NULL);

	fprintf(stderr, "  App: ready to be attached by Pin, about to release %s\n", semaphoreHandleName.c_str());
    if (!ReleaseSemaphore(
            readySemaphore,  // handle to semaphore
            1,            // increase count by one
            NULL) )       // not interested in previous count
    {
        printf("ReleaseSemaphore error: %d\n", GetLastError());
    }

    //
    // Waiting for Pin to attach to the current process
    //
    while (!PinIsAttached(num_threads+1)) SwitchToThread();

    loop = false;

    CloseHandle(threadCreatedSemaphore);
    CloseHandle(neverReleasedSemaphore);

    fprintf(stderr, "  App: ended successfully\n");

    return 0;
}

