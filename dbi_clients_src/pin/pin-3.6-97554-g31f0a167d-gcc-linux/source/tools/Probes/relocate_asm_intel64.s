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
#include <asm_macros.h>
nothing:
    ret

set_dead_beef:
    mov $0xdeadbeef, %rax
    ret

# A function with call in the first 5 bytes
DECLARE_FUNCTION(relocatable_1)
.global NAME(relocatable_1)
NAME(relocatable_1):
    call nothing
    push %rbp
    mov %rsp, %rbp
    call nothing
    call nothing
    leave
    ret
END_FUNCTION(relocatable_1)

#  A function with a call in the first 5 bytes, that does an indirect call to another function
DECLARE_FUNCTION(relocatable_1a)
.global NAME(relocatable_1a)
NAME(relocatable_1a):
    call nothing
    push %rbp
    mov %rsp, %rbp
    call nothing
    call nothing
    mov $1, %rax
    lea set_dead_beef(%rip), %rcx
    callq *%rcx
    leave
    ret
END_FUNCTION(relocatable_1a)




# A function with short first bb
DECLARE_FUNCTION(relocatable_2)
.global NAME(relocatable_2)
NAME(relocatable_2):
    xor %rax, %rax
    L:
    call nothing
    cmpq $1, %rax
    je LBB0
    inc %rax
    jmp L    
 LBB0:
    ret
    xor %rax, %rax
    xor %rbx, %rbx
END_FUNCTION(relocatable_2)

# A function with short first bb
DECLARE_FUNCTION(relocatable_3)
.global NAME(relocatable_3)
NAME(relocatable_3):
    xor %rax, %rax
    LBB1:
    mov 2(%rip), %rax
    call nothing
    cmpq $1, %rax
    jne LBB2
    movq $0, %rax
    jmp LBB1    
 LBB2:
    ret
    xor %eax, %eax
    xor %ebx, %ebx
END_FUNCTION(relocatable_3)

# A function with indirect jump
DECLARE_FUNCTION(non_relocatable_1)
.global NAME(non_relocatable_1)
NAME(non_relocatable_1):
    push %rbp
NR1L:
    mov %rsp, %rbp
    mov %rdi, %rax
    call nothing
    call nothing
    call nothing
    jmp *%rax
    je NR1L
    leave
    ret
END_FUNCTION(non_relocatable_1)

# A function with fallthru at the end
DECLARE_FUNCTION(non_relocatable_2)
.global NAME(non_relocatable_2)
NAME(non_relocatable_2):
    push %rbp
NR2M:
    mov %rsp, %rbp
    test %rax, %rax
    jb NR2M
    je NR2L
    leave
    ret
NR2L:
    inc %rax
    test %rax, %rax
    je NAME(non_relocatable_2)
END_FUNCTION(non_relocatable_2)

