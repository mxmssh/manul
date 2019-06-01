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
// This tool is used to test passing the IARG_TSC argument to an analysis function in probe mode.


#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "pin.H"

#if defined(TARGET_MAC)
    const char* FACTORIAL_NAME="_factorial";
#else
    const char* FACTORIAL_NAME="factorial";
#endif

std::ostream* OutFile = 0;

/* ===================================================================== */
/* Command line switches                                                 */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "", "Specify file name for the tool's output. If no filename is specified, the output will be directed to stdout.");


VOID AtRtn(VOID* name, ADDRINT arg1, UINT64 tsc)
{
    *OutFile << reinterpret_cast<CHAR *>(name) << "(" << arg1 << ")" << "\tTime Stamp Counter is: " << hex << tsc << dec << endl;
}

VOID Image(IMG img, VOID * v)
{
    if ( ! IMG_IsMainExecutable(img)) {
        return;        
    }

    RTN rtn = RTN_FindByName(img, FACTORIAL_NAME);
    if (RTN_Valid(rtn))
    {
        if ( ! RTN_IsSafeForProbedInsertion( rtn ) )
        {
            *OutFile << "It is not safe to insert a call before " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            return;
        }

        RTN_InsertCallProbed( rtn, IPOINT_BEFORE,  AFUNPTR(AtRtn), IARG_PTR, RTN_Name(rtn).c_str(), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_TSC, IARG_END);
    } else {
        *OutFile << FACTORIAL_NAME << " wasn't found." << endl;
    }
}

int main(int argc, char **argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    OutFile = KnobOutputFile.Value().empty() ? &cout : new std::ofstream(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(Image, 0);

    PIN_StartProgramProbed();
    return 0;
}
