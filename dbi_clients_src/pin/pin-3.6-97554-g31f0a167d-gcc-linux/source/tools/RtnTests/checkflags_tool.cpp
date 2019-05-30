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
#include <iostream>
#include "pin.H"
#include "tool_macros.h"

using std::cout;
using std::cerr;
using std::endl;

const char* checkFlagsFuncName = C_MANGLE("CheckFlags");
const char* toolRtnName = C_MANGLE("ToolRtn");


void PushfAnalysis()
{
    cout << "TOOL INFO: In PushfAnalysis." << endl;
}


VOID Image(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img)) return;
    const RTN rtn = RTN_FindByName(img, checkFlagsFuncName);
    if (!RTN_Valid(rtn))
    {
        cerr << "TOOL ERROR: Unable to find " << checkFlagsFuncName << "." << endl;
        PIN_ExitProcess(11);
    }
    unsigned int numOfPushfFound = 0;
    bool instrumentationComplete = false;
    RTN_Open(rtn);
    for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        const OPCODE opcode = INS_Opcode(ins);
        if (XED_ICLASS_PUSHF == opcode || XED_ICLASS_PUSHFD == opcode || XED_ICLASS_PUSHFQ == opcode)
        {
            ++numOfPushfFound;
            if (1 == numOfPushfFound) continue;
            if (2 < numOfPushfFound)
            {
                cerr << "TOOL ERROR: Unexpected number of pushf instructions found - " << numOfPushfFound << "." << endl;
                PIN_ExitProcess(12);
            }
            RTN_Close(rtn);
            const RTN toolRtn = RTN_CreateAt(INS_Address(ins), toolRtnName);
            if (!RTN_Valid(toolRtn))
            {
                cerr << "TOOL ERROR: Unable to create " << toolRtnName << "." << endl;
                PIN_ExitProcess(13);
            }
            RTN_InsertCallProbed(toolRtn, IPOINT_BEFORE, PushfAnalysis, IARG_END);
            RTN_Open(rtn);
            instrumentationComplete = true;
        }
    }
    RTN_Close(rtn);
    if (!instrumentationComplete)
    {
        cerr << "TOOL ERROR: Failed to add instrumentation." << endl;
        PIN_ExitProcess(14);
    }
}

int main(int argc, char *argv[])
{
    // Initialization.
    PIN_InitSymbols();
    PIN_Init(argc,argv);
    
    // Add instrumentation.
    IMG_AddInstrumentFunction(Image, 0);
    
    // Start the application.
    PIN_StartProgramProbed(); // never returns
    return 0;
}
