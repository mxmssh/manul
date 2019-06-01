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

/* ===================================================================== */
/*! @file
  Replace an original function with a custom function defined in the tool. 
  Call the original function from the replacement function using
  PIN_CallApplicationFunction
  Verify that the original function and functions it calls are instrumented

*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include <stdlib.h>
#include <fstream>
using namespace std;

static KNOB<string> KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "callapp14.out", "output file");

ADDRINT functionCalledByFunctionToBeReplacedAddr = 0;
ADDRINT functionToBeReplacedAddr = 0;
BOOL replacementFunctionCalled = FALSE;
BOOL replacementDone = FALSE;
BOOL functionToBeReplacedInstrumented = FALSE;
BOOL functionToBeReplacedInstrumentationCalled = FALSE;
BOOL functionCalledByFunctionToBeReplacedInstrumented = FALSE;
BOOL functionCalledByFunctionToBeReplacedInstrumentationCalled = FALSE;
static ofstream out;



/* ===================================================================== */

int MyReplacementFunction( CONTEXT * ctxt,  AFUNPTR origPtr, int one, int two )
{
    out << " MyReplacementFunction: PIN_CallApplicationFunction Replaced Function at address " << hexstr(ADDRINT(origPtr)) << endl;

    int res;

    replacementFunctionCalled = TRUE;
    PIN_CallApplicationFunction( ctxt, PIN_ThreadId(),
                                 CALLINGSTD_DEFAULT, origPtr, NULL,
                                 PIN_PARG(int), &res,
                                 PIN_PARG(int), one,
                                 PIN_PARG(int), two,
                                 PIN_PARG_END() );

    out << " MyReplacementFunction: Returned from Replaced Function res = " << res << endl;

    return res;
}


/* ===================================================================== */
VOID ImageLoad(IMG img, VOID *v)
{
    PROTO proto = PROTO_Allocate( PIN_PARG(int), CALLINGSTD_DEFAULT,
                                      "FunctionToBeReplaced", PIN_PARG(int), PIN_PARG(int),
                                      PIN_PARG_END() );
    
    RTN rtn = RTN_FindByName(img, "FunctionToBeReplaced");
    if (RTN_Valid(rtn))
    {
        out << " RTN_ReplaceSignature " << RTN_Name(rtn) << " in " << IMG_Name(img) << " at address "
             << hexstr(RTN_Address(rtn)) << " with MyReplacementFunction" << endl;

        functionToBeReplacedAddr = RTN_Address(rtn);
        
        RTN_ReplaceSignature(
                rtn, AFUNPTR(MyReplacementFunction),
                IARG_PROTOTYPE, proto,
                IARG_CONTEXT,
                IARG_ORIG_FUNCPTR,
                IARG_UINT32, 1,
                IARG_UINT32, 2,
                IARG_END);


        RTN rtn2 = RTN_FindByName(img, "FunctionCalledByFunctionToBeReplaced");
        if (RTN_Valid(rtn2))
        {
            functionCalledByFunctionToBeReplacedAddr = RTN_Address(rtn2);
        }

        replacementDone = TRUE;
    }    
    PROTO_Free( proto );
}


VOID FunctionToBeReplacedAnalysisFunc()
{
    functionToBeReplacedInstrumentationCalled = TRUE;
}


VOID FunctionCalledByFunctionToBeReplacedAnalysisFunc()
{
    functionCalledByFunctionToBeReplacedInstrumentationCalled = TRUE;
}

VOID Instruction(INS ins, VOID *v)
{
    if (INS_Address(ins) == functionToBeReplacedAddr)
    {
        functionToBeReplacedInstrumented = TRUE;
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)FunctionToBeReplacedAnalysisFunc, IARG_END);
    }
    else if (INS_Address(ins) == functionCalledByFunctionToBeReplacedAddr)
    {
        functionCalledByFunctionToBeReplacedInstrumented = TRUE;
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)FunctionCalledByFunctionToBeReplacedAnalysisFunc, IARG_END);
    }
}

VOID Fini(INT32 code, VOID *v)
{
    BOOL hadError = FALSE;
    if (!replacementDone)
    {
        out << "***Error !replacementDone" << endl;
        hadError = TRUE;
    }
    if (!functionToBeReplacedInstrumented)
    {
        out << "***Error !functionToBeReplacedInstrumented" << endl;
        hadError = TRUE;
    }
    if (!functionCalledByFunctionToBeReplacedInstrumented)
    {
        out << "***Error !functionCalledByFunctionToBeReplacedInstrumented" << endl;
        hadError = TRUE;
    }
    if (!functionToBeReplacedInstrumentationCalled)
    {
        out << "***Error !functionToBeReplacedInstrumentationCalled" << endl;
        hadError = TRUE;
    }
    if (!functionCalledByFunctionToBeReplacedInstrumentationCalled)
    {
        out << "***Error !functionCalledByFunctionToBeReplacedInstrumentationCalled" << endl;
        hadError = TRUE;
    }
    if (hadError)
    {
        out << "***Error hadError" << endl;
        exit (-1);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "Tool: callapp14" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
int main(INT32 argc, CHAR *argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    out.open(KnobOutput.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(Fini, 0);
    
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
