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
#include "tool_macros.h"
#include "pin.H"

ADDRINT FlushParm;
const UINT32 CacheLineSize = 64;
ADDRINT LowAddress = 0;
ADDRINT HighAddress = 0;

/*!
 * Print out the error message and exit the process.
 */
 VOID AbortProcess(const string & msg, unsigned long code)
{
    cerr << "Test aborted: " << msg << " with code " << code << endl;
    PIN_WriteErrorMessage(msg.c_str(), 1002, PIN_ERR_FATAL, 0);
    PIN_ExitProcess(1);
}

VOID FlushInstruction(ADDRINT add, UINT32 size)
{
    if (CacheLineSize != size)
    {
        AbortProcess("Cache flush line instruction with size!=64 !", 0);
    }
    if (FlushParm != add)
    {
        cout << std::hex << FlushParm << "    " << add << endl;
        AbortProcess("Cache flush line instruction with unexpected parm!", 0);
    }
}

VOID GetFlushParm(ADDRINT flushParm)
{
    FlushParm = flushParm;
}


/* ================================================================== */


VOID Instruction(INS ins, VOID *v)
{
    if ((INS_Address(ins) >= LowAddress) && (INS_Address(ins) <= HighAddress)
            && INS_IsCacheLineFlush(ins) && INS_IsMemoryRead(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)FlushInstruction,
               IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
    }
}

VOID ImageLoad(IMG img, VOID *v)
{
    if (IMG_IsMainExecutable(img))
    {
        LowAddress = IMG_LowAddress(img);
        HighAddress = IMG_HighAddress(img);

        RTN rtn = RTN_FindByName(img, C_MANGLE("TellPinFlushParm"));
        if (RTN_Valid(rtn))
        {
            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(GetFlushParm),
                IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_Close(rtn);
        }
    }
}


/* ================================================================== */
/*
 Initialize and begin program execution under the control of Pin
*/
int main(INT32 argc, CHAR **argv)
{
    PIN_Init(argc, argv);

    PIN_InitSymbols();

    INS_AddInstrumentFunction(Instruction, NULL);

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    PIN_StartProgram();

    return 0;
}
