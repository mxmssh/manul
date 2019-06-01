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
/*
 * This tool preforms various test cases which are related to PIN's behavior
 * when it tries to instrument instructions with non-standard memory operand
 * such as XSAVE/GATHER/SCATER.
  */
#include "pin.H"
#include <fstream>
#include <iostream>
#include <utility>
#include <cstdlib>

using namespace std;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,        "pintool",
    "o", "standard-memop.out", "specify output file name");

// The knob below specifies the testcase number to perform.
// Available test cases:
// 0: Instrument xsave typed instructions with IARG_MEMORYWRITE_EA.
// 1: Instrument xsave typed instructions with IARG_MEMORYWRITE_SIZE.
KNOB<int> KnobTestCase(KNOB_MODE_WRITEONCE,        "pintool",
    "c", "0", "specify the test case number to perform (0/IARG_MEMORYWRITE_EA 1/IARG_MEMORYWRITE_SIZE)");

// The knob below specifies the instrumentation point to perform.
// Available points cases:
// 0: Instrument before the instruction.
// 1: Instrument after the instruction.
KNOB<int> KnobIpoint(KNOB_MODE_WRITEONCE,        "pintool",
    "i", "0", "specify the instrumentation point (0/before 1/after)");

const static IARG_TYPE TESTCASE_IARG_WRITE[] =
{
        IARG_MEMORYWRITE_EA,
        IARG_MEMORYWRITE_SIZE,
};

const static IARG_TYPE TESTCASE_IARG_READ[] =
{
        IARG_MEMORYREAD_EA,
        IARG_MEMORYREAD_SIZE,
};


const static IPOINT TESTCASE_IPOINT[] =
{
        IPOINT_BEFORE,
        IPOINT_AFTER,
};

static ofstream* out = NULL;

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    cerr <<
        "This tool preforms various test cases which are related to PIN's behavior" << endl
        << "when it tries to instrument XSAVE family instructions which has non-standard memory operand" << endl;

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* =====================================================================
 * Called upon program finish
 * ===================================================================== */
VOID Fini(int, VOID * v)
{
    *out << "Fini" << endl;
    out->close();
    out = NULL;
}



/* =====================================================================
 * The analysis routine that is instrumented before/after XSAVE instruction
 * ===================================================================== */
VOID MemOpAnalysisXSAVE_EA(ADDRINT addr)
{
    *out << hex << "XSAVE on 0x" << addr << endl;
}

VOID MemOpAnalysisXSAVE_Size(UINT32 addr)
{
    *out << hex << "XSAVE of size 0x" << addr << endl;
}

/* =====================================================================
 * The analysis routine that is instrumented before/after XSAVEOPT instruction
 * ===================================================================== */
VOID MemOpAnalysisXSAVEOPT_EA(ADDRINT addr)
{
    *out << hex << "XSAVEOPT on 0x" << addr << endl;
}

VOID MemOpAnalysisXSAVEOPT_Size(UINT32 addr)
{
    *out << hex << "XSAVEOPT of size 0x" << addr << endl;
}

/* =====================================================================
 * The analysis routine that is instrumented before/after XRSTOR instruction
 * ===================================================================== */
VOID MemOpAnalysisXRSTOR_EA(ADDRINT addr)
{
    *out << hex << "XRSTOR on 0x" << addr << endl;
}

VOID MemOpAnalysisXRSTOR_Size(UINT32 addr)
{
    *out << hex << "XRSTOR of size 0x" << addr << endl;
}


/* =====================================================================
 * Iterate over a trace and instrument its memory related instructions
 * ===================================================================== */
VOID Trace(TRACE trace, VOID *v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_IsMemoryWrite(ins) &&
                INS_Category(ins) == XED_CATEGORY_XSAVE)
            {
                INS_InsertCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()], KnobTestCase.Value() ? AFUNPTR(MemOpAnalysisXSAVE_Size) : AFUNPTR(MemOpAnalysisXSAVE_EA),
                               TESTCASE_IARG_WRITE[KnobTestCase.Value()], IARG_END);
            }
            if (INS_IsMemoryWrite(ins) &&
                INS_Category(ins) == XED_CATEGORY_XSAVEOPT)
            {
                INS_InsertCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()], KnobTestCase.Value() ? AFUNPTR(MemOpAnalysisXSAVEOPT_Size) : AFUNPTR(MemOpAnalysisXSAVEOPT_EA),
                               TESTCASE_IARG_WRITE[KnobTestCase.Value()], IARG_END);
            }
            if (INS_IsMemoryRead(ins) &&
                INS_Opcode(ins) == XED_ICLASS_XRSTOR)
            {
                INS_InsertCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()], KnobTestCase.Value() ? AFUNPTR(MemOpAnalysisXRSTOR_Size) : AFUNPTR(MemOpAnalysisXRSTOR_EA),
                               TESTCASE_IARG_READ[KnobTestCase.Value()], IARG_END);
            }
        }
    }
}

/* =====================================================================
 * Entry point for the tool
 * ===================================================================== */
int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    out = new std::ofstream(KnobOutputFile.Value().c_str());
    if (KnobTestCase < 0 || KnobTestCase > (int)(sizeof(TESTCASE_IARG_WRITE)/sizeof(TESTCASE_IARG_WRITE[0]) - 1))
    {
        *out << "Bad test case number specified on command line: " << KnobTestCase << endl;
        return 4;
    }
    if (KnobIpoint < 0 || KnobIpoint > (int)(sizeof(TESTCASE_IPOINT)/sizeof(TESTCASE_IPOINT[0]) - 1))
    {
        *out << "Bad test case number specified on command line: " << KnobIpoint << endl;
        return 4;
    }
    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
