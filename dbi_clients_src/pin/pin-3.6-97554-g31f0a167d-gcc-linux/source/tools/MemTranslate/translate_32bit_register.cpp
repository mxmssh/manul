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
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include "pin.H"

static int* replacement_ptr = NULL;
static KNOB<bool> KnobVerbose(KNOB_MODE_WRITEONCE, "pintool",
    "verbose", "0", "Verbose output");


static int* replaceMemoryReadFunc(int* readAddr, void* pc)
{
    if (KnobVerbose)
    {
        cerr << pc << ") replaceMemoryReadFunc called with " << (void*)readAddr << endl;
    }
    if (*readAddr != 0xbadc0de)
        return readAddr;
    if (KnobVerbose)
    {
        cerr << pc << ") replacing " << (void*)readAddr << " to " << (void*)replacement_ptr << endl;
    }
    return replacement_ptr;
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    if (INS_IsMov(ins)
            && 1 == memOperands
            && INS_MemoryOperandIsRead(ins, 0)
            && REG_Width(INS_MemoryBaseReg(ins)) == REGWIDTH_32
            && INS_MemoryIndexReg(ins) == REG_INVALID()
            && INS_MemoryDisplacement(ins) == 0)
    {
        if (KnobVerbose)
        {
            cerr << "found " << INS_Disassemble(ins) << endl;
        }
        INS_InsertCall(ins,
                IPOINT_BEFORE,
                (AFUNPTR)replaceMemoryReadFunc,
                IARG_MEMORYOP_EA, 0,
                IARG_REG_VALUE, REG_INST_PTR,
                IARG_RETURN_REGS, REG_INST_G0,
                IARG_END);
        INS_RewriteMemoryOperand(ins, 0, REG_INST_G0);
    }
 }


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool tests rewrite memory operand prefixed with segment register" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}


/*
 * Allocate an address that falls outside the lower 32 bit address space
 * This ensures that we get an address cannot be accessed by memory operand
 * with only 32 bit base register
 */
void* allocateMemoryAbove32bitAddressSpace()
{
    void* startAddr = (void*)0x100000000;
    size_t memSize = getpagesize();
    for (void* curAddr = startAddr; curAddr >= startAddr; curAddr = (void*)((ADDRINT)curAddr + memSize))
    {
        void* mem = mmap(curAddr, memSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if (NULL == mem)
            continue;
        else if (mem >= startAddr)
            return mem;

        // We got memory in the low 32 bit address space.
        // Try to allocate again in a different region
        munmap(mem, memSize);
    }
    return NULL;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    replacement_ptr = (int*)allocateMemoryAbove32bitAddressSpace();
    if (NULL == replacement_ptr)
    {
        cerr <<  "Failed to allocate memory above the 32 bit address space" << endl;
        exit(2);
    }
    if (KnobVerbose)
    {
        cerr << "replacement_ptr = " << (void*)replacement_ptr << endl;
    }
    *replacement_ptr = 0xdeadbee;

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
