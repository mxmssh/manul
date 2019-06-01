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
#include <fstream>
#include <sys/syscall.h>
#include "pin.H"

// The highest bit value in a pointer
#define HIGHEST_BIT ((ADDRINT)1 << (8 * sizeof(void*) - 1))

// Tool's output file
ofstream OutFile;

// Temporary registers used to rewrite memory operands
REG rewrite_reg[2];

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "malloc_translation.out", "specify output file name");


/*
 * Translates a memory address.
 * malloc() related functions return address with the higest
 * bit on. so, we'll turn this bit off if we get such address.
 */
static ADDRINT TranslateMemRef(ADDRINT ea)
{
    return ea & ~HIGHEST_BIT;
}

/*
 * Pin calls this function everytime it needs to translate a memory address
 */
ADDRINT PIN_FAST_ANALYSIS_CALL memoryCallback(PIN_MEM_TRANS_INFO* memTransInfo, VOID *v)
{
    return TranslateMemRef(memTransInfo->addr);
}

/*
 * Translates address that are sent to syscalls.
 * Currently, we know of only one syscall that are called and requires
 * this translation - which is SYS_open.
 */
VOID SysBefore(ADDRINT num, ADDRINT *arg0, ADDRINT *arg1, ADDRINT *arg2, ADDRINT *arg3, ADDRINT *arg4, ADDRINT *arg5)
{
    switch (num)
    {
        case SYS_open:
            // The filename string needs to be translated
            *arg0 = TranslateMemRef(*arg0);
            break;
        case SYS_openat:
            // The filename string needs to be translated
            *arg1 = TranslateMemRef(*arg1);
            break;
        case SYS_write:
            // The 'buf' argument needs to be translated
            *arg1 = TranslateMemRef(*arg1);
            break;
    }
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    if (INS_IsSyscall(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SysBefore),
                       IARG_SYSCALL_NUMBER,
                       IARG_SYSARG_REFERENCE, 0, IARG_SYSARG_REFERENCE, 1,
                       IARG_SYSARG_REFERENCE, 2, IARG_SYSARG_REFERENCE, 3,
                       IARG_SYSARG_REFERENCE, 4, IARG_SYSARG_REFERENCE, 5,
                       IARG_END);
    }
    else
    {
        const UINT32 memOps = INS_MemoryOperandCount(ins);
        ASSERTX(memOps <= 2);
        for (UINT32 i = 0 ; i < memOps; i++)
        {
            OutFile << "Instrumenting at " << hex << INS_Address(ins) << " " << INS_Disassemble(ins).c_str() << " operand #" << i << std::endl;
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)TranslateMemRef,
                    IARG_MEMORYOP_EA, (ADDRINT)i,
                    IARG_RETURN_REGS, rewrite_reg[i],
                    IARG_END);
            INS_RewriteMemoryOperand(ins, i, rewrite_reg[i]);
        }
    }
}


// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    OutFile << "Done!" << endl;
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool tests memory address translation with malloc replacement" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    rewrite_reg[0] = PIN_ClaimToolRegister();
    rewrite_reg[1] = PIN_ClaimToolRegister();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register memory callback
    PIN_AddMemoryAddressTransFunction(memoryCallback, NULL);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();
}
