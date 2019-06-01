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
 * This tool performs IARG_MEMORYXX_SIZE check on fxsave and fxrstor
 */
#include <fstream>
#include <iostream>
#include <utility>
#include <cstdlib>
#include "pin.H"

using namespace std;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,        "pintool",
    "o", "", "specify output file name");

static ostream* out = NULL;

#define DELETE_OUT if (&cerr != out) delete out

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    cerr <<
        "This tool performs IARG_MEMORYXX_SIZE check on fxsave and fxrstor" << endl;

    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return 1;
}

/* =====================================================================
 * Called upon program finish
 * ===================================================================== */
VOID Fini(int, VOID * v)
{
    *out << "Fini" << endl;
    DELETE_OUT;
}

/* =====================================================================
 * The analysis routine that is instrumented before FXSAVE instruction
 * ===================================================================== */
VOID MemOpAnalysisFXSAVE(const UINT32 size)
{
    if (FPSTATE_SIZE_FXSAVE != size)
    {
        *out << "mismatch of fxsave size. exiting..." << endl;
        DELETE_OUT;
        PIN_ExitProcess(1);
    }
    *out << "FXSAVE of size " << size << endl;
}

/* =====================================================================
 * The analysis routine that is instrumented before FXRSTOR instruction
 * ===================================================================== */
VOID MemOpAnalysisFXRSTOR(const UINT32 size)
{
    if (FPSTATE_SIZE_FXSAVE != size)
    {
        *out << "mismatch of fxsave size. exiting..." << endl;
        DELETE_OUT;
        PIN_ExitProcess(1);
    }
    *out << "FXRSTOR of size " << size << endl;
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
            OPCODE oc = INS_Opcode(ins);
            if (INS_IsMemoryWrite(ins) &&
                (XED_ICLASS_FXSAVE == oc || XED_ICLASS_FXSAVE64 == oc))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(MemOpAnalysisFXSAVE),
                               IARG_MEMORYWRITE_SIZE, IARG_END);
            }
            if (INS_IsMemoryRead(ins) &&
                (XED_ICLASS_FXRSTOR == oc || XED_ICLASS_FXRSTOR64 == oc))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(MemOpAnalysisFXRSTOR),
                               IARG_MEMORYREAD_SIZE, IARG_END);
            }
        }
    }
}

/* =====================================================================
 * Entry point for the tool
 * ===================================================================== */
int main(int argc, CHAR *argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    // Initialize the output stream.
    const string fileName = KnobOutputFile.Value();
    out = (fileName.empty()) ? &cerr : new ofstream(fileName.c_str());
    if (NULL == out || out->fail())
    {
        cerr << "TOOL ERROR: Unable to open " << fileName << " for writing." << endl;
        return 1;
    }

    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();
    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
