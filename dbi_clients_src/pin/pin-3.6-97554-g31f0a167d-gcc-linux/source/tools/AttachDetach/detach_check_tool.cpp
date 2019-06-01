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
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <tool_macros.h>

using namespace std;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "tls_check_tool.out", "specify file name");

/* ===================================================================== */

ofstream TraceFile;


unsigned long *updateWhenReadyPtr = 0;

VOID DetachPinFromMTApplication(unsigned long *updateWhenReady)
{
    updateWhenReadyPtr = updateWhenReady;
    TraceFile << "Sending detach request" << endl;
    if (PIN_IsProbeMode())
    {
        PIN_DetachProbed();
    }
    else
    {
        PIN_Detach();
    }
}

VOID DetachCompleted(VOID *v)
{
    TraceFile << "Detach completed" << endl;
    *updateWhenReadyPtr = 1;
}

int PinAttached()
{
    TraceFile << "Pin Attached" << endl;
    return 1;
}

VOID ImageLoad(IMG img, void *v)
{
	RTN rtn = RTN_FindByName(img, C_MANGLE("TellPinToDetach"));
	if (RTN_Valid(rtn))
	{
	    TraceFile << "Replacing TellPinToDetach" << endl;
	    if (PIN_IsProbeMode())
        {
            ASSERTX(RTN_IsSafeForProbedReplacement(rtn));
            RTN_ReplaceProbed(rtn, AFUNPTR(DetachPinFromMTApplication));
        }
	    else
	    {
	        RTN_Replace(rtn, AFUNPTR(DetachPinFromMTApplication));
	    }
	}
	
    rtn = RTN_FindByName(img, C_MANGLE("PinAttached"));
    if (RTN_Valid(rtn))
    {
        TraceFile << "Replacing PinAttached" << endl;
        if (PIN_IsProbeMode())
        {
            ASSERTX(RTN_IsSafeForProbedReplacement(rtn));
            RTN_ReplaceProbed(rtn, AFUNPTR(PinAttached));
        }
        else
        {
            RTN_Replace(rtn, AFUNPTR(PinAttached));
        }
    }

}	
/* ===================================================================== */

int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc,argv);

    TraceFile.open(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);
    if (PIN_IsProbeMode())
    {
        TraceFile << "Running app in Probe mode" << endl;
        PIN_AddDetachFunctionProbed(DetachCompleted, 0);
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_AddDetachFunction(DetachCompleted, 0);
        TraceFile << "Running app in Jit mode" << endl;
        PIN_StartProgram();
    }

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
