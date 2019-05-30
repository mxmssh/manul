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
#include "asm_macros.h"

DECLARE_FUNCTION(CheckFlags)

.global NAME(CheckFlags)

NAME(CheckFlags):
    BEGIN_STACK_FRAME
    push    GBX_REG
    push    GCX_REG
    # Set the ZF and OF flags.
    mov     $0x80000000, RETURN_REG
    shl     $32, RETURN_REG
    shl     $1, RETURN_REG
    # Save the flags register before the analysis routine.
    pushf
    pop     GBX_REG
    mov     PARAM1, RETURN_REG
    mov     %ebx, (RETURN_REG)
    # The tool will create an artificial RTN here and add instrumentation.
    # Save the flags register after the analysis routine.
    pushf
    pop     GCX_REG
    mov     PARAM2, RETURN_REG
    mov     %ecx, (RETURN_REG)
    # Compare the flags before and after the analysis routine.
    cmp     %ebx, %ecx
    jnz     end
    # GAX is not zero since it holds a valid address on the stack. If the flags are identical, indicate success.
    mov     $0, RETURN_REG
end:
    pop     GCX_REG
    pop     GBX_REG
    END_STACK_FRAME
    ret
