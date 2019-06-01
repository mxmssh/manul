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
#include <stdio.h>
#include <set>
#include "pin.H"

using std::set;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "thread_count.out", "Output file");

FILE * out;
PIN_LOCK pinLock;
INT32 threadCreated = 0;
INT32 threadEnded = 0;

THREADID myThread = INVALID_THREADID;
set<THREADID> appThreads;

static VOID AppThreadStart(THREADID threadIndex)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    threadCreated++;
    appThreads.insert(threadIndex);
    PIN_ReleaseLock(&pinLock);
}

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    if (myThread == INVALID_THREADID)
    {
        // We will execute this code only once, and we are assuming that this is for the main thread or
        // at least NOT one of the threads that the application created with CreateThread().
        // We are registering this additional thread because ThreadFini is called for all threads including the main thread.
        myThread = threadid;
        AppThreadStart(threadid);
    }
}

VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    // Use the Pin lock to protect writes to appThreads between this callback function and the analysis routine AppThreadStart
    PIN_GetLock(&pinLock, PIN_GetTid());
    set<THREADID>::iterator it = appThreads.find(threadid);
    if (it != appThreads.end())
    {
        threadEnded++;
        appThreads.erase(it);
    }
    PIN_ReleaseLock(&pinLock);
}

VOID Fini(INT32 code, VOID *v)
{
    fprintf(out, "Number of threads created  - %d\n", (int)threadCreated);
    fprintf(out, "Number of threads terminated  - %d\n", (int)threadEnded);
    fclose(out);
}

static VOID InstrumentImg(IMG img, VOID *)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, "ThreadRoutine");
        if (RTN_Valid(rtn))
        {
            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(AppThreadStart), IARG_THREAD_ID, IARG_END);
            RTN_Close(rtn);
        }
    }
}

int main(INT32 argc, CHAR **argv)
{
    PIN_Init(argc, argv);
    PIN_InitLock(&pinLock);

    out = fopen(KnobOutputFile.Value().c_str(), "w");

    PIN_InitSymbols();

    IMG_AddInstrumentFunction(InstrumentImg, NULL);
    PIN_AddThreadStartFunction(ThreadStart, NULL);
    PIN_AddThreadFiniFunction(ThreadFini, NULL);
    PIN_AddFiniFunction(Fini, NULL);

    // Never returns
    PIN_StartProgram();

    return 0;
}
