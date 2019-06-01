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
#include <Windows.h>
#include <stdio.h>

// Thread creation - exercise windows APC mechanism

extern "C" __declspec(dllexport) int ThreadRoutine()
{
    return 0;
}

bool ThreadCreation()
{
    const unsigned long num_threads = 64;
    static HANDLE aThreads[num_threads] = { 0 };
    unsigned long slot = 0;
    unsigned long thread_id = 0;
    unsigned long cnt_th = 0;

    for (cnt_th = 0; cnt_th < num_threads; cnt_th++)
    {
        aThreads[cnt_th] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadRoutine,0,0,(LPDWORD)&thread_id);
        if (aThreads[cnt_th] == NULL)
        {
            fprintf(stderr, "CreateThread failed with GetLastError = %X \n", GetLastError()); 
            return false;
        }
    }

    fprintf(stderr, "created %d threads \n", num_threads); 
    // Calling WaitForMultipleObjects with bWaitAll=FALSE will return for each terminated thread,
    // returning the index of the signalled object.
    // It allows the application to call CloseHandle for each terminated thread while other threads are still running.
    while (cnt_th  > 0)
    {
        bool waitAll = FALSE;
        slot = WaitForMultipleObjects(cnt_th, aThreads, waitAll, INFINITE);
        if (slot >= cnt_th) 
        {
            fprintf(stderr, "WaitForMultipleObjects returned %X with GetLastError = %X\n", slot, GetLastError());
            return false;
        }
        CloseHandle(aThreads[slot]);
        aThreads[slot] = aThreads[cnt_th-1];
        cnt_th--;
    }
    fprintf(stderr, "all %d threads terminated\n", num_threads);
    fflush(stderr);
    return true;
}

extern "C" __declspec(dllimport) void Nothing();

int main()
{
    Nothing();
    return ThreadCreation() ? 0 : 1;
}
