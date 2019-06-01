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
#include <assert.h>
#include <iostream>
#include <fstream>
#include "pin.H"
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "", "specify trace file name");
KNOB<UINT32> KnobNumThreads(KNOB_MODE_WRITEONCE, "pintool",
    "n", "8", "Numbr Of Threads");

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ostream* TraceFile = 0;
PIN_LOCK pinLock;
struct Allocation
{
    CHAR * Addr;
    UINT32 Size;
    PIN_THREAD_UID ThreadUid;
    BOOL   ThreadStarted;
    Allocation() : Addr(0), Size(0), ThreadUid(INVALID_PIN_THREAD_UID), ThreadStarted(FALSE) {}
};
std::vector<Allocation> allocations;

/* ===================================================================== */
static void AbortProcess(const string & msg)
{
    THREADID myTid = PIN_ThreadId();

    PIN_GetLock(&pinLock, myTid + 1);
    *TraceFile << "malloc_stress test aborted: " << msg << "." << endl << flush;
    PIN_ReleaseLock(&pinLock);
    PIN_WriteErrorMessage(msg.c_str(), 1002, PIN_ERR_FATAL, 0);
}

static VOID SetNextSize(UINT32& currSize)
{
    static const UINT32 SIZES_LENGTH = 6;
    static UINT32 sizes[SIZES_LENGTH] =
    {
        100, 4000, 30, 20, 6000, 24000
    };
    for (UINT32 i = 0; i < SIZES_LENGTH; ++i)
    {
        if (currSize == sizes[i])
        {
            ++i;
            currSize = sizes[i % SIZES_LENGTH];
            return;
        }
    }
    if (currSize == 0)
    {
        currSize = sizes[0];
    }
}

static VOID AllocateAndCheck(ADDRINT id)
{
    if (allocations.at(id).Addr)
    {
        for (UINT32 i = 0; i < allocations.at(id).Size; ++i)
        {
            if (allocations.at(id).Addr[i] != CHAR(id))
            {
                AbortProcess("Data corruption detected on allocated buffer");
            }
        }

        free(allocations.at(id).Addr);
    }

    SetNextSize(allocations.at(id).Size);

    allocations.at(id).Addr = (CHAR*)malloc(allocations.at(id).Size);
    if (!allocations.at(id).Addr)
    {
        AbortProcess("Allocation failed");
    }
    for (UINT32 i = 0; i < allocations.at(id).Size; ++i)
    {
        allocations.at(id).Addr[i] = id;
    }
}

VOID ThreadFunc(VOID * arg)
{
    allocations.at((ADDRINT)arg).ThreadStarted = TRUE;

    static const UINT32 ALLOCATIONS_PER_THREAD = 10000;
    for (UINT32 j = 0; j < ALLOCATIONS_PER_THREAD; ++j)
    {
        AllocateAndCheck((ADDRINT)arg);
    }
}

VOID CheckFreeOfZero()
{
    free(0);
}

VOID SpawnThreads()
{
    allocations.resize(KnobNumThreads.Value());
    for (ADDRINT threads_started = 0; threads_started < KnobNumThreads.Value(); ++threads_started)
    {
        PIN_THREAD_UID threadUid;

        *TraceFile << "Creating thread " << endl;
        if (PIN_SpawnInternalThread(ThreadFunc, (VOID*)threads_started, 0, &threadUid) == INVALID_THREADID)
        {
            AbortProcess("PIN_SpawnInternalThread failed");
        }

        allocations.at(threads_started).ThreadUid = threadUid;
    }
}
/* ===================================================================== */
static VOID PrepareForFini(VOID *v)
{
    for (UINT32 i = 0; i < KnobNumThreads.Value(); ++i)
    {
        if (!allocations.at(i).ThreadStarted)
        {
            *TraceFile << "Tool's thread " << i << " did not started" << endl;
            continue;
        }

        INT32 threadExitCode;
        INT32 MaxTimeToWait = 100000;
        BOOL waitStatus = PIN_WaitForThreadTermination(allocations.at(i).ThreadUid, MaxTimeToWait, &threadExitCode);
        if (!waitStatus)
        {
            AbortProcess("PIN_WaitForThreadTermination failed");
        }
        if (threadExitCode != 0)
        {
            AbortProcess("Tool's thread exited abnormally");
        }
    }
    *TraceFile << "All threads joined" << endl;
}


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool test for malloc_stress" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return 1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    PIN_InitLock(&pinLock);
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    // Write to a file since cout and cerr maybe closed by the application
    TraceFile = KnobOutputFile.Value().empty() ? &cout : new std::ofstream(KnobOutputFile.Value().c_str());

    CheckFreeOfZero();

    PIN_AddPrepareForFiniFunction(PrepareForFini, 0);

    SpawnThreads();

    *TraceFile << "All Threads Created" << endl;

    PIN_StartProgram();
	
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
