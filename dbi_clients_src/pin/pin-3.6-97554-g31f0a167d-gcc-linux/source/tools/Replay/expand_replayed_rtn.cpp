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
 * This tool excersizes the fetch_rtn_ins code when replaying an image load.
 */
#include <fstream>
#include <iostream>
#include <iomanip>

#include <string.h>
#include "pin.H"

#ifdef TARGET_MAC
#define NAME(fun) "_" fun
#else
#define NAME(fun) fun
#endif

KNOB<string> KnobTestImage(KNOB_MODE_WRITEONCE,"pintool", "test-image", "", "Image to test");

VOID Image(IMG img, VOID *v)
{
    RTN foo = RTN_FindByName(img, NAME("foo"));
    ASSERTX(RTN_Valid(foo) || !IMG_IsMainExecutable(img));
    if (RTN_Valid(foo))
    {
        RTN_Open(foo);
        RTN_InsHead(foo);
        RTN_Close(foo);
    }
}



int main(int argc, char * argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // We will handle image load operations.
    PIN_SetReplayMode (REPLAY_MODE_IMAGEOPS);

    string testImage = KnobTestImage.Value();
    if (testImage.empty())
    {
        cerr << "ERROR: Must specify " << KnobTestImage.Cmd() << " to this PinTool" << endl;
        cerr << KNOB_BASE::StringKnobSummary();
        return 1;
    }
    // Creates artificial main image
    PIN_LockClient();
    PIN_ReplayImageLoad(testImage.c_str(), testImage.c_str(), 0x40000, REPLAY_IMAGE_TYPE_MAIN_EXE);
    PIN_UnlockClient();

    IMG_AddInstrumentFunction(Image, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

