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
#include "pin.H"

/*
 * This tool tests correctness of the IARG_MEMORYOP_PTR value in instructions
 * with rewritten memory operands.
 * Note, this tool does not use PIN_GetMemoryAddressTransFunction to register
 * memory translation callback.
 */

// Temporary registers used to rewrite memory operands
REG rewriteReg[2];

/*
 * @return two's complement of the specified value
 */
static ADDRINT NegValue(ADDRINT val)
{
    return 0 - val;
}

/*
 * Verify correctness of the IARG_MEMORYOP_PTR calculation
 * @param[in] ea        IARG_MEMORYOP_EA value
 * @param[in] ptr       IARG_MEMORYOP_PTR value
 * @pre (ptr == NegValue(ea)) 
 */
static VOID CheckMemPtr(ADDRINT ea, ADDRINT ptr)
{
    if (ptr != NegValue(ea))
    {
        cerr << "Wrong IARG_MEMORYOP_PTR value: " << hex << ptr << " != " << NegValue(ea) << endl;
        PIN_ExitProcess(1);
    }
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    UINT32 memOps = INS_MemoryOperandCount(ins);
    if (memOps > 2) { memOps = 2; } // maximum two rewritten operands 

    for (UINT32 i = 0 ; i < memOps; i++)
    {
        // Set rewriteReg[i] = -IARG_MEMORYOP_EA(i)
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)NegValue,
                IARG_CALL_ORDER, CALL_ORDER_DEFAULT,
                IARG_MEMORYOP_EA, (ADDRINT)i,
                IARG_RETURN_REGS, rewriteReg[i],
                IARG_END);

        // Check to see that IARG_MEMORYOP_PTR(i) == -IARG_MEMORYOP_EA(i)
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckMemPtr,
                IARG_CALL_ORDER, CALL_ORDER_DEFAULT + 1,
                IARG_MEMORYOP_EA, (ADDRINT)i,
                IARG_MEMORYOP_PTR, (ADDRINT)i,
                IARG_END);

        // Revert the rewriteReg[i] value back to IARG_MEMORYOP_EA(i)
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)NegValue,
                IARG_CALL_ORDER, CALL_ORDER_DEFAULT + 2,
                IARG_REG_VALUE, rewriteReg[i],
                IARG_RETURN_REGS, rewriteReg[i],
                IARG_END);

        // Rewrite i-th memory operand (with no EA change)
        INS_RewriteMemoryOperand(ins, i, rewriteReg[i]);
    }
}


// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    cout << "Done!" << endl;
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool tests IARG_MEMORY*_PTR arguments for rewritten memory operands" << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    rewriteReg[0] = PIN_ClaimToolRegister();
    rewriteReg[1] = PIN_ClaimToolRegister();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();
}
