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
// This tool demonstrates the use of PIN CONTEXT IARG and to examine the performance 
// change between the implementations.

#include <iostream>
#include <fstream>
#include <cassert>
#include "pin.H"
#include <map>

using std::ofstream;


/////////////////////
// GLOBAL VARIABLES
/////////////////////

// A knob for defining the output file name
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "context.out", "specify output file name");

// A knob for selecting the needed CONTEXT
KNOB<int> KnobContextType(KNOB_MODE_WRITEONCE, "pintool",
    "c", "0" , "specify context to use (0-partial no regs in/out 1-const 2-canonical 3-partial all regs in/out 4-no context)");

KNOB<int> KnobVerbose(KNOB_MODE_WRITEONCE, "pintool",
    "v", "0" , "verbose mode (specify 1 for verbose, default is 0)");

// ofstream object for handling the output
ofstream OutFile;

/////////////////////
// ANALYSIS FUNCTIONS
/////////////////////

static void lookupRegister(CONTEXT * ctxt, ADDRINT iaddr, THREADID tid)
{
    // For the integer registers, it is safe to use ADDRINT. But make sure to pass a pointer to it.
    ADDRINT val = PIN_GetContextReg(ctxt, REG_GDX);
    if (KnobVerbose.Value())
        OutFile << hexstr(val) << " at addr " << hexstr(iaddr) << " " << decstr(tid) << endl;
}

static void lookupRegisterNoContext(ADDRINT iaddr, THREADID tid)
{
    if (KnobVerbose.Value())
        OutFile << "noContext " << " at addr " << hexstr(iaddr) << " " << decstr(tid) << endl;
}

/////////////////////
// INSTRUMENTATION FUNCTIONS
/////////////////////
static VOID Trace(TRACE trace, VOID *v)
{
    const static REGSET allRegs = PIN_GetFullContextRegsSet();
    if (0 == KnobContextType.Value())
    {
       REGSET regsin;
       REGSET regsout;
       TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)lookupRegister, IARG_PARTIAL_CONTEXT, &regsin, &regsout,
                       IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
    }
    else if (1 == KnobContextType.Value())
    {
        TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)lookupRegister, IARG_CONST_CONTEXT,
                         IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
    }
    else if (3 == KnobContextType.Value()) {
         TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)lookupRegister, IARG_PARTIAL_CONTEXT, &allRegs, &allRegs,
                          IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
    }
    else if (4 == KnobContextType.Value())
    {
        TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)lookupRegisterNoContext,
                         IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
    }
    else
    {
        TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)lookupRegister, IARG_CONTEXT,
                         IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
    }
}

/////////////////////
// MAIN FUNCTION
/////////////////////

int main(int argc, char * argv[])
{
    // Initialize Pin
    PIN_Init(argc, argv);

    // Open the output file
    OutFile.open(KnobOutputFile.Value().c_str());
    assert(OutFile.is_open());

    // Add instrumentation
    TRACE_AddInstrumentFunction(Trace, 0);

    // Start running the application
    PIN_StartProgram(); // never returns

    return 1;
}
