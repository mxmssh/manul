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
#include <unistd.h>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>

static ADDRINT mainImageRelroAddr = 0;
static ADDRINT mainImageRelroSize = 0;
static ADDRINT mainImageRelroPageStart = 0;
static ADDRINT mainImageRelroPageSize = 0;
static ADDRINT relocatedMainImageRelroPageAddr = 0;
static bool relroUnmapped = false;

// Temporary registers used to rewrite memory operands
REG rewrite_reg[2];

/* ===================================================================== */

ADDRINT PageStart(ADDRINT p)
{
    ADDRINT sz = (ADDRINT)getpagesize();
    return (p & ~(sz - 1));
}

ADDRINT PageEnd(ADDRINT p)
{
    ADDRINT sz = (ADDRINT)getpagesize();
    return ((p + sz - 1) & ~(sz - 1));
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID *v)
{
    if (IMG_IsMainExecutable(img))
    {
        ElfW(Ehdr)* hdr = (ElfW(Ehdr)*)IMG_LowAddress(img);
        ElfW(Phdr)* phdr = (ElfW(Phdr)*)((char*)hdr + hdr->e_phoff);
        for (int i = 0; i < hdr->e_phnum; i++)
        {
            if (phdr[i].p_type == PT_GNU_RELRO)
            {
                mainImageRelroAddr = phdr[i].p_vaddr + IMG_LoadOffset(img);
                mainImageRelroSize = phdr[i].p_memsz;
                mainImageRelroPageStart = PageStart(mainImageRelroAddr);
                mainImageRelroPageSize = PageEnd(mainImageRelroAddr + mainImageRelroSize) - mainImageRelroPageStart;
                fprintf(stderr, "Main image's PT_RELRO is at [%p,%p]\n", (void*)mainImageRelroAddr, (void*)mainImageRelroSize);
                break;
            }
        }
        ASSERTX(0 != mainImageRelroAddr);
        ASSERTX(0 != mainImageRelroSize);
    }
    else if (!IMG_IsInterpreter(img) && !IMG_IsVDSO(img) && !relroUnmapped)
    {
        // This is the first image which was actually loader by the loader
        ASSERTX(0 != mainImageRelroAddr);
        ASSERTX(0 != mainImageRelroSize);
        relroUnmapped = true;

        relocatedMainImageRelroPageAddr = (ADDRINT)mmap(NULL, mainImageRelroPageSize, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        ASSERTX((ADDRINT)-1 != relocatedMainImageRelroPageAddr);
        fprintf(stderr, "Moving [%p, %p] -> %p\n", (void*)mainImageRelroPageStart, (void*)mainImageRelroPageSize, (void*)relocatedMainImageRelroPageAddr);

        memcpy((void*)relocatedMainImageRelroPageAddr, (void*)mainImageRelroPageStart, mainImageRelroPageSize);
        mprotect((void*)relocatedMainImageRelroPageAddr, mainImageRelroPageSize, PROT_READ);
        int res = munmap((void*)mainImageRelroPageStart, mainImageRelroPageSize);
        ASSERTX(0 == res);
    }
}

/*
 * Translates a memory address.
 * After we moved the PT_GNU_RELRO region to a different place in the memory,
 * here we redirect all memory accesses to the original PT_GNU_RELRO region
 * to the new region that they moved to.
 */
static ADDRINT TranslateMemRef(ADDRINT ea, ADDRINT pc)
{
    if (0 == relocatedMainImageRelroPageAddr)
    {
        return ea;
    }
    if ((ea >= mainImageRelroPageStart) && (ea < (mainImageRelroPageStart + mainImageRelroPageSize)))
    {
        return ea - mainImageRelroPageStart + relocatedMainImageRelroPageAddr;
    }
    return ea;
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    const UINT32 memOps = INS_MemoryOperandCount(ins);
    ASSERTX(memOps <= 2);
    for (UINT32 i = 0 ; i < memOps; i++)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)TranslateMemRef,
                IARG_MEMORYOP_EA, (ADDRINT)i,
                IARG_REG_VALUE, REG_INST_PTR,
                IARG_RETURN_REGS, rewrite_reg[i],
                IARG_END);
        INS_RewriteMemoryOperand(ins, i, rewrite_reg[i]);
    }
}

/* ===================================================================== */

int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc,argv);

    rewrite_reg[0] = PIN_ClaimToolRegister();
    rewrite_reg[1] = PIN_ClaimToolRegister();

    IMG_AddInstrumentFunction(ImageLoad, NULL);
    INS_AddInstrumentFunction(Instruction, NULL);

    PIN_StartProgram();

    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
