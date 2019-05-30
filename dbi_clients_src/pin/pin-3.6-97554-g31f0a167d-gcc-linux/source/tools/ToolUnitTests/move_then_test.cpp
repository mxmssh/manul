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
 This tool tests that THEN instrumentation is executed correctly.
*/
#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <map>

#if defined(TARGET_LINUX)
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
# ifdef TARGET_IA32E
# include <asm/prctl.h>
# include <sys/prctl.h>
# endif // TARGET_LINUX
#endif // TARGET_IA32E

#include "pin.H"
#include "instlib.H"

// windows.h must be included after pin.H
#ifdef TARGET_WINDOWS
namespace WIND
{
#include <windows.h>
}
#endif // TARGET_WINDOWS

// Knob for specifying if the test is for exception.
KNOB<int> KnobException(KNOB_MODE_WRITEONCE, "pintool",
    "exception", "0", "specify if the test is for exception");

int isException = 0;

BOOL x = TRUE;
ADDRINT curIp = 0;
ADDRINT curIp2 = 0;
map<ADDRINT,BOOL> mem;

struct THENS_CALLED
{
    BOOL contextInThenCalled;
    BOOL noContextInThenCalled;
};

typedef std::map<ADDRINT, THENS_CALLED> THENS_CALLED_INFO;
THENS_CALLED_INFO thensCalledInfo;


const int numInstructionsToInstrument = 1000;


BOOL  IfReturnTrue(ADDRINT  ip)
{
    curIp = ip;
    return x;
}

BOOL  IfReturnTrue2(ADDRINT  ip)
{
    curIp2 = ip;
    return x;
}

VOID  ContextInThen(
                ADDRINT  ip,
                CONTEXT * context)
{
     if (ip != curIp)
     {
         printf ("***Error ContextInThen got ip 0x%lx expected 0x%lx\n", static_cast<unsigned long>(ip), static_cast<unsigned long>(curIp));
         exit(-1);
     }
     if (PIN_GetContextReg( context, REG_INST_PTR )!=ip)
     {
         printf ("***Error ContextInThen argument ip 0x%lx does not match ip in context (0x%lx)\n", static_cast<unsigned long>(ip),
            static_cast<unsigned long>(PIN_GetContextReg( context, REG_INST_PTR )));
         exit(-1);
     }
     THENS_CALLED_INFO::iterator it = thensCalledInfo.find(ip);
     if (it == thensCalledInfo.end())
     {
         printf ("***Error ContextInThen with no record of instrumentation ip 0x%lx\n", static_cast<unsigned long>(ip));
     }
     THENS_CALLED thensCalled = it->second;
     thensCalledInfo.erase(ip);
     thensCalled.contextInThenCalled = TRUE;
     thensCalledInfo.insert(std::make_pair(ip, thensCalled));
}


VOID  NoContextInThen(
                ADDRINT  ip)
{
    if (ip != curIp)
    {
        printf ("***Error NoContextInThen got ip 0x%lx expected curIp 0x%lx\n", static_cast<unsigned long>(ip), static_cast<unsigned long>(curIp));
        exit(-1);
    }
     if (ip != curIp2)
     {
         printf ("***Error NoContextInThen got ip 0x%lx expected curIp2 0x%lx\n", static_cast<unsigned long>(ip), static_cast<unsigned long>(curIp2));
         exit(-1);
     }
     curIp = 0;
     curIp2 = 0;
     THENS_CALLED_INFO::iterator it = thensCalledInfo.find(ip);
     if (it == thensCalledInfo.end())
     {
         printf ("***Error NoContextInThen with no record of instrumentation ip 0x%lx\n", static_cast<unsigned long>(ip));
     }
     THENS_CALLED thensCalled = it->second;
     thensCalledInfo.erase(ip);
     thensCalled.noContextInThenCalled = TRUE;
     thensCalledInfo.insert(std::make_pair(ip, thensCalled));
}



INT32 Usage()
{
    cerr <<
        "This is a test of  moving the then sequence to the stub"
        "\n";

    cerr << endl;

    return -1;
}

ADDRINT imgStartAdd;
USIZE imgSize;

VOID ImageLoad(IMG img, VOID *v)
{
    if (IMG_IsMainExecutable(img))
    {
        imgStartAdd = IMG_StartAddress(img);
        imgSize = IMG_SizeMapped(img);
    }
}


int numInstructionsInstrumented = 0;
//intruction for noException tests
VOID Instruction(INS ins, VOID *v)
{
    if (numInstructionsInstrumented >= numInstructionsToInstrument)
    {
        return;
    }
    THENS_CALLED_INFO::iterator it = thensCalledInfo.find(INS_Address(ins));
    if (it != thensCalledInfo.end())
    {
        return;
    }
    //instrument if ins is app instruction, also not exception test
    if (INS_Address(ins) >= imgStartAdd && INS_Address(ins) < (imgStartAdd + imgSize))
    {
        numInstructionsInstrumented++;
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfReturnTrue,
                            IARG_INST_PTR,
                            IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ContextInThen,
                            IARG_INST_PTR,
                            IARG_CONTEXT,
                            IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfReturnTrue2,
                            IARG_INST_PTR,
                            IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)NoContextInThen,
                            IARG_INST_PTR,
                            IARG_END);
        THENS_CALLED thensCalled;
        thensCalled.contextInThenCalled = thensCalled.noContextInThenCalled = FALSE;
        thensCalledInfo.insert(std::make_pair(INS_Address(ins), thensCalled));
    }
}

//intruction for Exception tests
VOID InstructionEx(INS ins, VOID *v)
{
    if (numInstructionsInstrumented >= numInstructionsToInstrument)
    {
        return;
    }
    THENS_CALLED_INFO::iterator it = thensCalledInfo.find(INS_Address(ins));
    if (it != thensCalledInfo.end())
    {
        return;
    }

    numInstructionsInstrumented++;
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfReturnTrue,
                        IARG_INST_PTR,
                        IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ContextInThen,
                        IARG_INST_PTR,
                        IARG_CONTEXT,
                        IARG_END);

    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfReturnTrue2,
                        IARG_INST_PTR,
                        IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)NoContextInThen,
                        IARG_INST_PTR,
                        IARG_END);
    THENS_CALLED thensCalled;
    thensCalled.contextInThenCalled = thensCalled.noContextInThenCalled = FALSE;
    thensCalledInfo.insert(std::make_pair(INS_Address(ins), thensCalled));
}

VOID Fini(INT32 code, VOID *v)
{
    int numTested = 0;
    for (THENS_CALLED_INFO::iterator it = thensCalledInfo.begin();  it != thensCalledInfo.end();  ++it)
    {
        numTested++;
        if (it->second.contextInThenCalled != TRUE)
        {
            printf ("***Error ContextInThen NOT called for instruction at ip 0x%lx\n", static_cast<unsigned long>(it->first));
            exit (-1);
        }
        if (it->second.noContextInThenCalled != TRUE)
        {
            printf ("***Error NoContextInThen NOT called for instruction at ip 0x%lx\n", static_cast<unsigned long>(it->first));
            exit (-1);
        }
    }
    if (numTested != numInstructionsInstrumented)
    {
        printf ("***Error numTested != numInstructionsToInstrument   numTested %d numInstructionsToInstrument %d\n",
                numTested, numInstructionsToInstrument);
        exit (-1);
    }
    exit (0);
}

int main(int argc, char *argv[])
{
    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }

    isException = KnobException.Value();

    if (isException)
    {
        INS_AddInstrumentFunction(InstructionEx, NULL);
    }
    else
    {
        INS_AddInstrumentFunction(Instruction, NULL);
    }
    PIN_AddFiniFunction(Fini, NULL);

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    // Never returns
    PIN_StartProgram();

    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
