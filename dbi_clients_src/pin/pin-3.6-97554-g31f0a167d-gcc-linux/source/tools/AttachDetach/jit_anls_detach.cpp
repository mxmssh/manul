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
 * Test detach from an analysis routine (in Jit). Also check detach callback was called
 */
#include <stdio.h>
#include "pin.H"
#include <iostream>
#include <fstream>

using namespace std;

UINT64 icount = 0;

UINT32 icountMax = 10000;
volatile bool detached = false; // True if detach callback was called

ofstream outfile;

KNOB<string> KnobOutFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "jit_anls_detach.out", "Specify file name for the tool's output.");


VOID docount() 
{ 
    ASSERT(!detached, "Analysis function was called after detach ended");
    if (++icount ==  icountMax)
	{
		outfile << "Send detach request form analysis routine after " << icount << " instructions." << endl;
		PIN_Detach();
	}
}
    
VOID Instruction(INS ins, VOID *v)
{
    ASSERT(!detached, "Instrumentation function was called after detach ended");
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
}

VOID Detach(VOID *v)
{
    if (detached) // sanity check
    {
        // This should never be reached because only one detach request should be executed.
        cerr << "TOOL ERROR: jit_instr_detach is executing the Detach callback twice." << endl;
        exit(20); // use exit instead of PIN_ExitProcess because we don't know if it is available at this point.
    }
    detached = true;
    outfile << "Pin detached after " << icount << " executed instructions." << endl;
    outfile.close();
}


VOID Fini(INT32 code, VOID *v)
{
    cerr << "Count: " << icount << endl;
    ASSERT(0, "Error, Fini called although we detached");
}

int main(int argc, char * argv[])
{
    PIN_Init(argc, argv);

    outfile.open(KnobOutFile.Value().c_str());
    if (!outfile.good())
    {
        cerr << "Failed to open output file " << KnobOutFile.Value().c_str() << "." << endl;
        PIN_ExitProcess(10);
    }


    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_AddDetachFunction(Detach, 0);
    
    // Never returns
    PIN_StartProgram();
    
    return 0;
}
