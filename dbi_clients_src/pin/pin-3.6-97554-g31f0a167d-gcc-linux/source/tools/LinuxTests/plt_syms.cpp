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
#include "pin.H"
#include <iostream>
#include <fstream>

/* ===================================================================== */

/*!
 * Print out the error message and exit the process.
 */
static void AbortProcess(const string & msg, unsigned long code)
{
    cerr << "Test aborted: " << msg << " with code " << code << endl;
    PIN_WriteErrorMessage(msg.c_str(), 1002, PIN_ERR_FATAL, 0);
    PIN_ExitProcess(1);
}

/*!
 * Called before write@plt.
 */
void Before_Write(VOID* s)
{
    string str((CHAR*)s);
    if ("printing using write\n" != str)
    {
        AbortProcess("Argument for write@plt isn't expected!", 0);
    }
    cout << "calling write@plt" << endl;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID *v)
{
    if (IMG_IsMainExecutable(img))
    {
        PROTO proto_write = PROTO_Allocate(PIN_PARG(ssize_t), CALLINGSTD_DEFAULT,
                                             "write@plt", PIN_PARG(INT32),
                                             PIN_PARG(VOID*), PIN_PARG(size_t),
                                             PIN_PARG_END());

        RTN rtn = RTN_FindByName(img, "write@plt");
        if (RTN_Valid(rtn))
        {
            if (!RTN_IsSafeForProbedInsertion(rtn))
            {
                AbortProcess("Cannot insert before " + RTN_Name(rtn) + " in " + IMG_Name(img), 0);
            }
            RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Before_Write),
                    IARG_PROTOTYPE, proto_write,
                    IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                    IARG_END);
            cout << "Inserted probe call before write@plt: " << IMG_Name(img) << endl;
        }

        PROTO_Free(proto_write);
    }
}

/* ===================================================================== */

int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc,argv);

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    PIN_StartProgramProbed();

    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
