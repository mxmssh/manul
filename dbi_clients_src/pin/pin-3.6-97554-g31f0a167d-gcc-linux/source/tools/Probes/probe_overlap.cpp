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
using namespace std;

#include "pin.H"

VOID before(void)
{
}


VOID ImageLoad(IMG img, VOID *v)
{
    RTN rtn = RTN_FindByName(img, "foo");

    if (RTN_Valid(rtn))
    {
        // Insert a BEFORE call at the second instruction of foo().
        // Then check probe safety for inserting a BEFORE call at foo().
        // We expect to fail due to overlapping probes.
        if (! RTN_IsSafeForProbedInsertion(rtn))
        {
            cerr << RTN_Name(rtn) << "() is not safe for probing " << endl;
            PIN_ExitProcess(1);
        }

        RTN_Open(rtn);
        INS ins = RTN_InsHead(rtn);
        if (! INS_Valid(ins))
        {
            cerr << "Failed to get first instruction of " << RTN_Name(rtn) << endl;
            PIN_ExitProcess(1);
        }

        ins = INS_Next(ins);
        if (! INS_Valid(ins))
        {
            cerr << "Failed to get next instruction after first of " << RTN_Name(rtn) << endl;
            PIN_ExitProcess(1);
        }

        ADDRINT addr = INS_Address(ins);
        RTN_Close(rtn);

        RTN fake_rtn = RTN_CreateAt(addr, "FakeRtn");
        if (RTN_Valid(fake_rtn))
        {
            RTN_InsertCallProbed( fake_rtn, IPOINT_BEFORE, AFUNPTR(before), IARG_END);
        }
        else
        {
            cerr << "RTN_CreateAt failed. " << endl;
            PIN_ExitProcess(1);
        }

        if (RTN_IsSafeForProbedInsertion(rtn))
        {
            cerr << RTN_Name(rtn) << "() is expected to be unsafe for probing after inserting overlapping probe with RTN_CreateAt " << endl;
            PIN_ExitProcess(1);
        }
    }
}


/* ===================================================================== */

int main(INT32 argc, CHAR *argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_StartProgramProbed();

    return 0;
}



/* ===================================================================== */
/* eof */
/* ===================================================================== */
