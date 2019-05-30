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
 * This test tool verifies that PIN asserts that trace re-generation is equal
 * to the original trace generation.
 * When PIN generates a trace in the code cache, it does not generate the 
 * meta-data for that trace (App instructions vs. instrumentation code vs. spill/fill code). 
 * This meta-data is created only in case of an exception, when PIN needs to figure out what 
 * code caused the exception, by re-generating the trace. If the instrumentation scheme changes 
 * between the first and second trace generation, the results of the exception analysis are bogus.
 */

#include "pin.H"

//=================================================================================================
/*!
 * Global variables
 */
volatile void *ptr = NULL;

//=================================================================================================

/*!
 * INS analysis routines.
 */
VOID OnIns()
{
    // Generate exception in inlined analysis routine.
    *(int *)ptr = 11111;
}

/*!
 * INS instrumentation routine.
 * This is an inconsistent instrumentation function as it instruments only once.
 * In case of trace re-generation, the original instrumented instruction won't be
 * instrumented again. 
 */
VOID Instruction(INS ins, VOID *v)
{
    static bool isFirst = true;
    if (isFirst)
    {
        isFirst = false;
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)OnIns, IARG_END);
    }
}

//=================================================================================================
/*!
 * The main procedure of the tool.
 */
int main(int argc, char *argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_StartProgram();
    return 0;
}
