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

static ADDRINT rtn_address = 0;
static size_t rtn_size = 0;
static unsigned char* rtn_bytes = NULL;


void SaveRtnBytes(RTN rtn)
{
    rtn_address = RTN_Address(rtn);
    rtn_size = (size_t)RTN_Size(rtn);
    rtn_bytes = (unsigned char*)malloc(rtn_size);

    if (rtn_bytes)
    {
        memcpy((void*)rtn_bytes, (const void*)rtn_address, rtn_size);
    }
}

void ValidateProbeSize(int nbytes)
{
    if (rtn_bytes)
    {
        int compare_probe_bytes = memcmp((const void*)rtn_bytes, (const void*)rtn_address, nbytes);
        int compare_rest_of_rtn = memcmp((const void*)(rtn_bytes+nbytes), (const void*)(rtn_address+nbytes), rtn_size-nbytes);

        delete[] rtn_bytes;

        if ((compare_probe_bytes == 0) || (compare_rest_of_rtn != 0))
        {
            cerr << "Invalid probe size. Probe seems to be bigger than the expected " << nbytes << " bytes." << endl;
            PIN_ExitProcess(1);
        }
    }
}

VOID foo_before(void)
{
#ifdef TARGET_IA32E
    int expected_probe_size = 6;
#else
    int expected_probe_size = 5;
#endif

    ValidateProbeSize(expected_probe_size);
}


VOID ImageLoad(IMG img, VOID *v)
{
    RTN rtn = RTN_FindByName(img, "foo");

    if (RTN_Valid(rtn))
    {
        SaveRtnBytes( rtn );
        RTN_InsertCallProbed( rtn, IPOINT_BEFORE, AFUNPTR(foo_before), IARG_END);
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
