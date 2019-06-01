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
 * Among other things this test checks:
 *  - Verify that all image load callbacks and all thread attach callbacks are being called before starting to Probe.
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sched.h>
#include <assert.h>
#if defined(TARGET_LINUX)
#include <elf.h>
#endif
#include "tool_macros.h"

using namespace std;


/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "probe_tool.out", "specify file name");

#ifdef TARGET_LINUX
KNOB<BOOL> KnobJustQueryAuxv(KNOB_MODE_WRITEONCE, "pintool",
    "just_auxv", "0", "just test availability of auxv values");
#endif

ofstream TraceFile;
/* ===================================================================== */

INT32 Usage()
{
    cerr <<
        "This pin tool tests MT attach in probe mode.\n"
        "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

PIN_LOCK pinLock;

UINT32 threadCounter=0;
BOOL   isAppStartReceived = FALSE;

volatile BOOL probeBegan = FALSE; // True if probing has started

#ifdef TARGET_LINUX
void QueryAuxv(const char* name, ADDRINT value)
{
    bool found = false;
    ADDRINT vdso = PIN_GetAuxVectorValue(value, &found);
    if (found)
    {
        TraceFile << name << " value: " << vdso << endl;
    }
    else
    {
        TraceFile << "Could not find auxv value " << name << endl;
    }
}
#endif

VOID AppStart(VOID *v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Application Start Callback is called from thread " << dec << PIN_GetTid() << endl;
    isAppStartReceived = TRUE;
    PIN_ReleaseLock(&pinLock);
}

VOID AttachedThreadStart(VOID *sigmask, VOID *v)
{
    ASSERT(!probeBegan, "Probe began before all thread attach callbacks were called");
    PIN_GetLock(&pinLock, PIN_GetTid());
    TraceFile << "Thread counter is updated to " << dec <<  (threadCounter+1) << endl;
    ++threadCounter;
    PIN_ReleaseLock(&pinLock);
}

int PinReady(unsigned int numOfThreads)
{
    probeBegan = TRUE;
    PIN_GetLock(&pinLock, PIN_GetTid());
	// Check that we don't have any extra thread
	assert(threadCounter <= numOfThreads);
    if ((threadCounter == numOfThreads) && isAppStartReceived)
    {
        TraceFile.close();
        PIN_ReleaseLock(&pinLock);
        return 1;
    }
    PIN_ReleaseLock(&pinLock);
    return 0;
}

VOID ImageLoad(IMG img, void *v)
{
    ASSERT(!probeBegan, "Probe began before all image load callbacks were called");
	RTN rtn = RTN_FindByName(img, C_MANGLE("ThreadsReady"));
	if (RTN_Valid(rtn))
	{
		RTN_ReplaceProbed(rtn, AFUNPTR(PinReady));
	}
}


/* ===================================================================== */

int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();

    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }

#if defined(TARGET_LINUX) && defined(TARGET_IA32)
    int gs_reg_value = 0;
    asm("mov $0, %%eax\n"
            "mov %%gs, %%eax\n"
            "mov %%eax, %0\n"
            : "=r" (gs_reg_value));
    ASSERTX(0 != gs_reg_value);
#endif

    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);

#ifdef TARGET_LINUX
    if (KnobJustQueryAuxv) {
        QueryAuxv("AT_ENTRY", AT_ENTRY);
        QueryAuxv("UNDEFINED_ENTRY", 0xFFFFFFF);
        TraceFile.close();
        PIN_ExitProcess(0);
    }
#endif

    PIN_InitLock(&pinLock);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddApplicationStartFunction(AppStart, 0);
    PIN_AddThreadAttachProbedFunction(AttachedThreadStart, 0);
    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
