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
//
//  This tool tests the functionality of PIN_AddSmcDetectedFunction
//
#include <cstdio>
#include "pin.H"


/* ================================================================== */
/* Global Variables                                                   */
/* ================================================================== */
ADDRINT startOfFuncInBuf = 0;
ADDRINT expectedSmcDetectedtraceStartAddress = 0;
ADDRINT expectedSmcDetectedtraceEndAddress = 0;
BOOL foundExpectedSmc = FALSE;
int numSmcDetected = 0;
FILE *fp;
/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

/* ================================================================== */


VOID SmcDetected (ADDRINT traceStartAddress, ADDRINT traceEndAddress, VOID *v)
{
    printf ("Tool: SmcDetected in range %p - %p\n", (void *)(traceStartAddress), (void *)(traceEndAddress));
    if (expectedSmcDetectedtraceStartAddress == traceStartAddress &&
        expectedSmcDetectedtraceEndAddress == traceEndAddress)
    {
        foundExpectedSmc = TRUE;
    }
    ASSERTX (traceStartAddress != 0);
    ASSERTX (traceEndAddress != 0);
    numSmcDetected++;
}


VOID Trace(TRACE trace, VOID *v)
{
    if (startOfFuncInBuf == 0)
    {
        if (fp)
        {
            fscanf (fp, "%p", (void **)(&startOfFuncInBuf));
            if (startOfFuncInBuf)
            {
                fclose(fp);
                fp = NULL;
                printf ("Tool: startOfFuncInBuf %p TRACE_Address(trace) %p\n",
                       (void *)startOfFuncInBuf, (void *)TRACE_Address(trace));
            }
        }
    }
    if (TRACE_Address(trace) == startOfFuncInBuf
        && expectedSmcDetectedtraceStartAddress == 0)
    {
        printf ("Tool: TRACE_Address(trace) %p TRACE_EndAddress(trace) %p\n",
                (void *)TRACE_Address(trace), (void *)(TRACE_Address(trace) + TRACE_Size(trace)));
        expectedSmcDetectedtraceStartAddress = TRACE_Address(trace);
        expectedSmcDetectedtraceEndAddress = TRACE_Address(trace) + TRACE_Size(trace);
    }
}


/* ================================================================== */
/*
 This routine is called once at the end.
*/
VOID Fini(INT32 c, VOID *v)
{
    ASSERTX (foundExpectedSmc);
}

/* ================================================================== */
/*
 Initialize and begin program execution under the control of Pin
*/
int main(INT32 argc, CHAR **argv)
{
    if (PIN_Init(argc, argv) ) return 1;

    TRACE_AddInstrumentFunction(Trace, 0);

    // Register a routine that gets called when the program ends
    PIN_AddFiniFunction(Fini, 0);

    TRACE_AddSmcDetectedFunction (SmcDetected, 0);

    fp = fopen ("smcapp1.out", "w+");

    PIN_StartProgram();  // Never returns

    return 0;
}
