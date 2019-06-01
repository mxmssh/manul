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
This test verifies "Isret" and "IsIret" return the appropriate return values
that defined in the documentation.
*/

#include <cstdio>
#include <iostream>
#include "pin.H"

BOOL testIsRetForRet = false;
BOOL testIsRetForIret = false;
BOOL testIsIretForIret = false;
BOOL testNotIsIretForRet = false;

inline BOOL isIretOpcode(INS ins)
{
    return (INS_Opcode(ins) == XED_ICLASS_IRET || INS_Opcode(ins) == XED_ICLASS_IRETD || \
           INS_Opcode(ins) == XED_ICLASS_IRETQ);
}

inline BOOL isRetOpcode(INS ins)
{
    return (INS_Opcode(ins) == XED_ICLASS_RET_NEAR || INS_Opcode(ins) == XED_ICLASS_RET_FAR);
}

VOID Instruction(INS ins, VOID *v)
{
    //ret not iret instruction
    if(isRetOpcode(ins))
    {
        if (!INS_IsRet(ins))
        {
            printf ("***Error Isret returns false for ret instruction. \n");
            PIN_ExitProcess(3);
        }
        testIsRetForRet = true;
        if (INS_IsIRet(ins))
        {
            printf ("***Error IsIret returns true for ret (not iret) instruction. \n");
            PIN_ExitProcess(4);
        }
        testNotIsIretForRet = true;
    }
    //iret instruction
    else if(isIretOpcode(ins))
    {
        if (!INS_IsRet(ins))
        {
            printf ("***Error Isret returns false for ret(itret) instruction. \n");
            PIN_ExitProcess(5);
        }
        testIsRetForIret = true;
        if (!INS_IsIRet(ins))
        {
            printf ("***Error IsIret returns false for  iret instruction. \n");
            PIN_ExitProcess(6);
        }
        testIsIretForIret = true;
    }
    else //not ret or iret
    {
        if (INS_IsRet(ins))
        {
            printf ("***Error Isret returns true for not ret instruction. \n");
            PIN_ExitProcess(1);
        }
        if (INS_IsIRet(ins))
        {
            printf ("***Error IsIret returns true for not iret(not even ret) instruction. \n");
            PIN_ExitProcess(2);
        }
    }
}

VOID Fini(INT32 code, VOID *v)
{
    if (!testIsRetForRet)
    {
        printf ("***Error did not check: Isret returns true for ret instruction. \n");
        PIN_ExitProcess(1);
    }
    if (!testIsRetForIret)
    {
        printf ("***Error did not check: Isret returns true for iret instruction. \n");
        PIN_ExitProcess(2);
    }
    if (!testIsIretForIret)
    {
        printf ("***Error did not check: IsIret returns true for iret instruction. \n");
        PIN_ExitProcess(3);
    }
    if (!testNotIsIretForRet)
    {
        printf ("***Error did not check: IsIret returns false for ret instruction. \n");
        PIN_ExitProcess(4);
    }

}
int main(INT32 argc, CHAR **argv)
{
    PIN_Init(argc, argv);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
    // Never returns
    PIN_StartProgram();
    return 7;
}


