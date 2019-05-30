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
// This tool tests the IARG_UINT64 argument type


#include <iostream>
#include <fstream>
#include "pin.H"

#define CONST_U32_1 0x11111111UL
#define CONST_U32_2 0x22222222UL
#define CONST_U64_1 0x3333333344444444ULL

ostream *OutFile;

/* ===================================================================== */
/* Command line switches                                                 */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
                            "o", "", "Specify file name for the tool's output. If no filename is specified,"
                            " the output will be directed to stdout.");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool tests the IARG_UINT64 argument type." << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return 1;
}

VOID AtIns1(UINT64 arg1, UINT32 arg2, UINT32 arg3)
{
    *OutFile << arg1 << "," << arg2 << "," << arg3 << endl;
}

VOID PIN_FAST_ANALYSIS_CALL AtIns2(UINT64 arg1, UINT32 arg2, UINT32 arg3)
{
    *OutFile << arg1 << "," << arg2 << "," << arg3 << endl;
}

VOID AtIns3(UINT32 arg1, UINT64 arg2, UINT32 arg3)
{
    *OutFile << arg1 << "," << arg2 << "," << arg3 << endl;
}

VOID PIN_FAST_ANALYSIS_CALL AtIns4(UINT32 arg1, UINT64 arg2, UINT32 arg3)
{
    *OutFile << arg1 << "," << arg2 << "," << arg3 << endl;
}

VOID AtIns5(UINT32 arg1, UINT32 arg2, UINT64 arg3)
{
    *OutFile << arg1 << "," << arg2 << "," << arg3 << endl;
}

VOID PIN_FAST_ANALYSIS_CALL AtIns6(UINT32 arg1, UINT32 arg2, UINT64 arg3)
{
    *OutFile << arg1 << "," << arg2 << "," << arg3 << endl;
    PIN_ExitApplication(0);
}


VOID Instruction(INS ins, void *v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(AtIns1), IARG_UINT64, CONST_U64_1, IARG_UINT32, CONST_U32_1, IARG_UINT32, CONST_U32_2, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(AtIns2), IARG_FAST_ANALYSIS_CALL, IARG_UINT64, CONST_U64_1, IARG_UINT32, CONST_U32_1, IARG_UINT32, CONST_U32_2, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(AtIns3), IARG_UINT32, CONST_U32_1, IARG_UINT64, CONST_U64_1, IARG_UINT32, CONST_U32_2, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(AtIns4), IARG_FAST_ANALYSIS_CALL, IARG_UINT32, CONST_U32_1, IARG_UINT64, CONST_U64_1, IARG_UINT32, CONST_U32_2, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(AtIns5), IARG_UINT32, CONST_U32_1, IARG_UINT32, CONST_U32_2, IARG_UINT64, CONST_U64_1, IARG_END);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(AtIns6), IARG_FAST_ANALYSIS_CALL, IARG_UINT32, CONST_U32_1, IARG_UINT32, CONST_U32_2, IARG_UINT64, CONST_U64_1, IARG_END);
}


int main(int argc, char **argv)
{
    if( PIN_Init(argc,argv) ) {
        return Usage();
    }

    OutFile = KnobOutputFile.Value().empty() ? &cout : new std::ofstream(KnobOutputFile.Value().c_str());
    *OutFile << hex;

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_StartProgram();
    return 0;
}
