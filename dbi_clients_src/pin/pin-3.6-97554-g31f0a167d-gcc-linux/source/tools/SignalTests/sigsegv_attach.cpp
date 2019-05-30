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
 *  Test that SIGSEGV signal is handled correctly. This tool lets the app know we are attached.
 *  The tool should be used with the sigsegv_app application.
 */

#include <iostream>
#include <fstream>
#include "pin.H"


// Global variables


ofstream outFile; // The tool's output file for printing the loaded images.

bool attached = false;


static void OnDoRelease(ADDRINT doRelease)
{
    if (!attached) return;
    *((bool*)doRelease) = true;
}


static VOID ImageLoad(IMG img, VOID *v)
{
    outFile << IMG_Name(img) << endl;
    if (IMG_IsMainExecutable(img))
    {
        RTN doReleaseRtn = RTN_FindByName(img, "DoRelease");
        ASSERTX(RTN_Valid(doReleaseRtn));

        RTN_Open(doReleaseRtn);
        RTN_InsertCall(doReleaseRtn, IPOINT_BEFORE, AFUNPTR(OnDoRelease), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        RTN_Close(doReleaseRtn);
    }
}


static VOID OnAppStart(VOID *v)
{
    attached = true;
}

int main( INT32 argc, CHAR *argv[] )
{
    // Initialization.
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // Add instrumentation.
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddApplicationStartFunction(OnAppStart, 0);

    // Start the program.
    PIN_StartProgram(); // never returns

    return 0;
}
