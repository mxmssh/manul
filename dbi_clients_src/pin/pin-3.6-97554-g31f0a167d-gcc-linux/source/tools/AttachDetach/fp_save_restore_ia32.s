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
	push %ebp
	mov %esp, %ebp
	mov 8(%ebp), %eax
	fxsave (%eax)
	leave
	ret
END_FUNCTION(ReadFpContext)

DECLARE_FUNCTION(WriteFpContext)
NAME(WriteFpContext):
	push %ebp
	mov %esp, %ebp
	mov 8(%ebp), %eax
	fxrstor (%eax)
	leave
	ret
END_FUNCTION(WriteFpContext)

.global sched_yield

// void GetLock(long *mutex, long newVal)
DECLARE_FUNCTION(GetLock)
NAME(GetLock):
    push %ebp
    mov %esp, %ebp
    push %esi
    push %edi
    mov 8(%ebp), %esi # %esi <- mutex
    mov 0xc(%ebp), %edi # %edi <- new value
    xor %eax, %eax

try_again:
    lock cmpxchg %edi, (%esi)
    je done
    call PLT_ADDRESS(sched_yield)
    jmp try_again
done:
    pop %edi
    pop %esi
    leave
    ret
END_FUNCTION(GetLock)
        
// void ReleaseLock(long *mutex)

DECLARE_FUNCTION(ReleaseLock)
NAME(ReleaseLock):
    push %ebp
    mov %esp, %ebp
    push %edi
    mov 8(%ebp), %edi
    xor %eax, %eax
    lock xchg %eax, (%edi)
    pop %edi
    leave
    ret
END_FUNCTION(ReleaseLock)

// void InitLock(long *mutex)    
DECLARE_FUNCTION(InitLock)
NAME(InitLock):
    push %ebp
    mov %esp, %ebp
    push %edi
    mov 8(%ebp), %edi
    xor %eax, %eax
    lock xchg %eax, (%edi)
    pop %edi
    leave
    ret
END_FUNCTION(InitLock)

// extern "C" void SetXmmRegs(long v1, long v2, long v3);
// extern "C" void GetXmmRegs(long *v1, long *v2, long *v3);



DECLARE_FUNCTION(SetXmmRegs)
NAME(SetXmmRegs):
  movss 0x4(%esp), %xmm1
  movss 0x8(%esp), %xmm2
  movss 0xc(%esp), %xmm3
  ret
END_FUNCTION(SetXmmRegs)
 
DECLARE_FUNCTION(GetXmmRegs)
NAME(GetXmmRegs):
  mov 0x4(%esp), %eax
  movss %xmm1, (%eax)
  mov 0x8(%esp), %eax
  movss %xmm2, (%eax)
  mov 0xc(%esp), %eax
  movss %xmm3, (%eax)
  ret
END_FUNCTION(GetXmmRegs)

