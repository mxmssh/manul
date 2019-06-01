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
/*! @file */

#include <iostream>
#include "pin.H"

static int ifunc_resolver_count = 0;

void Before_ifunc_resolver()
{
    ifunc_resolver_count++;
    cout << "Before_ifunc_resolver" << endl;
}

void Before_ifunc_exist()
{
    cout << "Before_ifunc_exist" << endl;
}

void Before_ifunc_not_exist()
{
    cout << "Before_ifunc_not_exist" << endl;
}

VOID ImageLoad( IMG img, VOID *v )
{
    if (IMG_Name(img).find("ifunc_complex_resolver_lib_app") == string::npos)
        return;
    RTN ifunc = RTN_FindByName(img, "ifunc");
    RTN ifunc_exist = RTN_FindByName(img, "ifunc_exist");
    RTN ifunc_not_exist = RTN_FindByName(img, "ifunc_not_exist");
    ASSERTX(RTN_Valid(ifunc));
    ASSERTX(RTN_Valid(ifunc_exist));
    ASSERTX(RTN_Valid(ifunc_not_exist));

    RTN_Open(ifunc);
    RTN_InsertCall(ifunc, IPOINT_BEFORE, AFUNPTR(Before_ifunc_resolver), IARG_END);
    RTN_Close(ifunc);

    RTN_Open(ifunc_exist);
    RTN_InsertCall(ifunc_exist, IPOINT_BEFORE, AFUNPTR(Before_ifunc_exist), IARG_END);
    RTN_Close(ifunc_exist);

    RTN_Open(ifunc_not_exist);
    RTN_InsertCall(ifunc_not_exist, IPOINT_BEFORE, AFUNPTR(Before_ifunc_not_exist), IARG_END);
    RTN_Close(ifunc_not_exist);
}

VOID Fini(INT32 code, VOID* v)
{
    ASSERTX(1 == ifunc_resolver_count);
}

int main (INT32 argc, CHAR *argv[])
{
    // Initialize pin
    //
    if (PIN_Init(argc, argv)) return 0;

    // Initialize symbol processing
    //
    PIN_InitSymbolsAlt(IFUNC_SYMBOLS);

    // Register ImageLoad to be called when an image is loaded
    //
    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    //
    PIN_StartProgram();

    return 0;
}
