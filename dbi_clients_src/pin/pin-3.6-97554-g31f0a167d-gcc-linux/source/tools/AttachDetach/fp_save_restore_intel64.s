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

DECLARE_FUNCTION(ReadFpContext)
NAME(ReadFpContext):
	fxsave (%rdi)
	ret

DECLARE_FUNCTION(WriteFpContext)
NAME(WriteFpContext):
	fxrstor (%rdi)
	ret
END_FUNCTION(WriteFpContext)

.global sched_yield

// void GetLock(long *mutex, long newVal)
DECLARE_FUNCTION(GetLock)
NAME(GetLock):
    push %rbp
    mov %rsp, %rbp
    xor %rax, %rax 
    
    // rdi - mutex
    // rsi - newVal

try_again:
    lock cmpxchg %rsi, (%rdi)
    je done
    push %rsi
    push %rdi
    call PLT_ADDRESS(sched_yield)
    pop %rdi
    pop %rsi
    jmp try_again
done:
    leave
    ret
END_FUNCTION(GetLock)

// void ReleaseLock(long *mutex)

DECLARE_FUNCTION(ReleaseLock)
NAME(ReleaseLock):
    push %rdi
    xor %rax, %rax
    lock xchg %rax, (%rdi) # put 0 in *mutex
    pop %rdi
    ret
END_FUNCTION(ReleaseLock)

// void InitLock(long *mutex)
DECLARE_FUNCTION(InitLock)
NAME(InitLock):
    push %rdi
    xor %rax, %rax
    lock xchg %rax, (%rdi) # put 0 in *mutex
    pop %rdi
    ret
END_FUNCTION(InitLock)
    
// extern "C" void SetXmmRegs(long v1, long v2, long v3);
// extern "C" void GetXmmRegs(long *v1, long *v2, long *v3);



DECLARE_FUNCTION(SetXmmRegs)
NAME(SetXmmRegs):
  movd %rdi, %xmm1
  movd %rsi, %xmm2
  movd %rdx, %xmm3
  ret
END_FUNCTION(SetXmmRegs)

DECLARE_FUNCTION(GetXmmRegs)
NAME(GetXmmRegs):
  movsd %xmm1, (%rdi)
  movsd %xmm2, (%rsi)
  movsd %xmm3, (%rdx)
  ret
END_FUNCTION(GetXmmRegs)
