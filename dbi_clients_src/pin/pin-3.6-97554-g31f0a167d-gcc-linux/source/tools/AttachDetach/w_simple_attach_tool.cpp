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
#include "pin.H"
#include <iostream>
using namespace std;

namespace WIND
{
#include <windows.h>
}

/* ===================================================================== */
/* Global variables and declarations */
/* ===================================================================== */

static volatile int isAppStarted = 0;

static volatile int isReadyForDetach = 0;

static volatile int isReadyForAttach = 0;

static volatile int pinCompletelyAttached = 0;

/* ===================================================================== */

int rep_PinIsAttached()
{
    return pinCompletelyAttached;
}

int rep_PinIsDetached()
{
    return 0;
}

void rep_FirstProbeInvoked()
{
    cerr << "rep_FirstProbeInvoked" << endl;
    isReadyForDetach = 1;
}

void rep_SecondProbeInvoked()
{
    cerr << "rep_SecondProbeInvoked" << endl;
    isReadyForDetach = 1;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID *v)
{
    if(isReadyForAttach == 1)
    {
        //can't get callbacks from pin after detach completion
        std::cerr << "failure - got follow child notification when pin is detached" << endl;
        exit(-1);
    }

    if ( ! IMG_IsMainExecutable(img) )
    {
        return;
    }

    RTN rtn = RTN_FindByName(img, "PinIsAttached");
    ASSERTX(RTN_Valid(rtn));
    RTN_ReplaceProbed(rtn, AFUNPTR(rep_PinIsAttached));

    rtn = RTN_FindByName(img, "PinIsDetached");
    ASSERTX(RTN_Valid(rtn));
    RTN_ReplaceProbed(rtn, AFUNPTR(rep_PinIsDetached));

    rtn = RTN_FindByName(img, "FirstProbeInvoked");
    ASSERTX(RTN_Valid(rtn));
    RTN_ReplaceProbed(rtn, AFUNPTR(rep_FirstProbeInvoked));

    rtn = RTN_FindByName(img, "SecondProbeInvoked");
    ASSERTX(RTN_Valid(rtn));
    RTN_ReplaceProbed(rtn, AFUNPTR(rep_SecondProbeInvoked));

    pinCompletelyAttached = 1;
}

VOID DetachComplete(VOID *v)
{
    isReadyForDetach = 0;
    isAppStarted = 0;
    isReadyForAttach = 1;
    pinCompletelyAttached = 0;
}

VOID AppStart(VOID *v)
{
    isAppStarted = 1;
}

VOID AttachMain(VOID *v)
{
    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddApplicationStartFunction(AppStart, 0);

    PIN_AddDetachFunctionProbed(DetachComplete, 0);

    isReadyForAttach = 0;
}

WIND::DWORD WINAPI ThreadProc(VOID * p)
{
    while(isReadyForDetach == 0)
    {
        WIND::SwitchToThread();
    }
    PIN_DetachProbed();

    while(isReadyForAttach == 0)
    {
        WIND::SwitchToThread();
    }

    PIN_AttachProbed(AttachMain, 0);

    while(isReadyForDetach == 0)
    {
        WIND::SwitchToThread();
    }
    PIN_DetachProbed();

    while(isReadyForAttach == 0)
    {
        WIND::SwitchToThread();
    }

    PIN_AttachProbed(AttachMain, 0);

    return 0;
}

int main(INT32 argc, CHAR **argv)
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddApplicationStartFunction(AppStart, 0);

    PIN_AddDetachFunctionProbed(DetachComplete, 0);

    WIND::HANDLE threadHandle = WIND::CreateThread(NULL, 0, (WIND::LPTHREAD_START_ROUTINE)ThreadProc, NULL, 0, NULL);
    WIND::CloseHandle(threadHandle);

    // Never returns
    PIN_StartProgramProbed();

    return 0;
}