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
 * A tool that creates internal Pin thread
 * and verifies that the thread is finished gracefully.
 * Probe mode.
 */

#include <cstdlib>
#include <string>
#include <iostream>
#include "pin.H"

using std::cerr;
using std::endl;

PIN_LOCK intThreadCanOpen;
PIN_LOCK mainThreadCanOpen;

const char* MailSlotName = "\\\\.\\mailslot\\sample_mailslot";

// UID of the internal thread. It is created in the application thread by the
// main() tool's procedure.
PIN_THREAD_UID intThreadUid;


/*!
 * Print out the error message and exit the process.
 */
static void AbortProcess(const string & msg, unsigned long code)
{
    cerr << "Test aborted: " << msg << " with code " << code << endl;
    PIN_WriteErrorMessage(msg.c_str(), 1002, PIN_ERR_FATAL, 0);
    PIN_ExitProcess(1);
}

VOID IntThread(VOID *arg)
{
    NATIVE_FD hWriteSlot;
    PIN_GetLock(&intThreadCanOpen, 1);

    cerr << "IntThread calling OS_OpenFD" << endl;
    OS_OpenFD(MailSlotName, OS_FILE_OPEN_TYPE_WRITE, 0, &hWriteSlot);
    cerr << "IntThread returning from OS_OpenFD" << endl;

    PIN_ReleaseLock(&mainThreadCanOpen);

    PIN_ExitThread(0);
}

/*!
 * Process exit callback (unlocked).
 */
VOID PrepareForFini()
{
    BOOL waitStatus;
    INT32 threadExitCode;
    NATIVE_FD hWriteSlot;
    
    PIN_LockClient();
    
    PIN_ReleaseLock(&intThreadCanOpen);

    PIN_GetLock(&mainThreadCanOpen, 1);
    
    cerr << "main thread calling OS_OpenFD" << endl;
    OS_OpenFD(MailSlotName, OS_FILE_OPEN_TYPE_WRITE, 0, &hWriteSlot);
    cerr << "main thread returning from OS_OpenFD" << endl;
    
    PIN_UnlockClient();

    // First, wait for termination of the main internal thread. When this thread exits,
    // all secondary internal threads are already created and, so <uidSet> can be safely
    // accessed without lock.
    waitStatus = PIN_WaitForThreadTermination(intThreadUid, 1000, &threadExitCode);
    if (!waitStatus)
    {
        AbortProcess("PIN_WaitForThreadTermination(intThreadUid) failed", 0);
    }
    if (0 != threadExitCode)
    {
        AbortProcess("Tool's thread 1 exited abnormally", 0);
    }

    cerr << "Tool's thread finished successfully." << endl;
}


typedef void  (*FUNCPTR)(int status);
static void  (*pf_exit)(int status);

VOID MyExit(int arg)
{
    PrepareForFini();

    (pf_exit)(arg);
}

VOID ImageLoad(IMG img, VOID *v)
{
    RTN rtn = RTN_FindByName(img, "exit");

    if ( RTN_Valid(rtn))
    {
        if (RTN_IsSafeForProbedReplacementEx(rtn, PROBE_MODE_ALLOW_RELOCATION))
        {
            pf_exit = (FUNCPTR)RTN_ReplaceProbedEx( rtn, PROBE_MODE_ALLOW_RELOCATION, AFUNPTR(MyExit));
        }
        else
        {
            AbortProcess("ImageLoad: Pin can't replace exit()", 0);
        }
    }
}

/*!
 * The main procedure of the tool.
 */
int main(int argc, char *argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();
    
    PIN_InitLock(&intThreadCanOpen);
    PIN_InitLock(&mainThreadCanOpen);
    PIN_GetLock(&intThreadCanOpen, 1);
    PIN_GetLock(&mainThreadCanOpen, 1);

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    // Spawn the main internal thread. When this thread starts it spawns all other internal threads.
    THREADID intThreadId = PIN_SpawnInternalThread(IntThread, NULL, 0, &intThreadUid);
    if (INVALID_THREADID == intThreadId)
    {
        AbortProcess("PIN_SpawnInternalThread(intThread) failed", 0);
    }

    // Never returns
    PIN_StartProgramProbed();
    return 1;
}
