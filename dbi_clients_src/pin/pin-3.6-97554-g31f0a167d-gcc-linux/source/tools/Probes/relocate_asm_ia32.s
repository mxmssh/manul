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
    mov $0xdeadbeef, %eax
    ret

# A function with jmp in the first 5 bytes
DECLARE_FUNCTION(relocatable_1)
.global NAME(relocatable_1)
NAME(relocatable_1):
    push %ebp
    mov %esp, %ebp
LBB0:
    call nothing
    call nothing
    movl $1, %eax
    test %eax, %eax
    je LBB0
    leave
    ret
END_FUNCTION(relocatable_1)


# A function with jmp in the first 5 bytes, that does an indirect call to another function
DECLARE_FUNCTION(relocatable_1a)
.global NAME(relocatable_1a)
NAME(relocatable_1a):
    push %ebp
    mov %esp, %ebp
LBB1:
    mov $1, %eax
    lea set_dead_beef, %ecx
    calll *%ecx
    test %eax, %eax
    je LBB1
    leave
    ret
END_FUNCTION(relocatable_1a)



# A function with short first bb
DECLARE_FUNCTION(relocatable_2)
.global NAME(relocatable_2)
NAME(relocatable_2):
    xor %eax, %eax
    LBB2:
    call nothing
    cmpl $1, %eax
    je LBB3
    inc %eax
    jmp LBB2    
 LBB3:
    ret
    xor %eax, %eax
    xor %ebx, %ebx
END_FUNCTION(relocatable_2)

# A function with short first bb
DECLARE_FUNCTION(relocatable_3)
.global NAME(relocatable_3)
NAME(relocatable_3):
    xor %eax, %eax
    LBB4:
    mov $2, %eax
    call nothing
    cmpl $1, %eax
    jne LBB5
    mov $0, %eax
    jmp LBB4    
 LBB5:
    ret
    xor %eax, %eax
    xor %ebx, %ebx
END_FUNCTION(relocatable_3)

# A function with indirect jump
DECLARE_FUNCTION(non_relocatable_1)
.global NAME(non_relocatable_1)
NON_RELOCATABLE_1_START:
NAME(non_relocatable_1):
    push %ebp
LBB6:
    mov %esp, %ebp
    mov 8(%ebp), %eax
    call nothing
    call nothing
    call nothing
    je LBB6
    jmpl *%eax
    leave
    ret
END_FUNCTION(non_relocatable_1)

# A function with fallthru at the end
DECLARE_FUNCTION(non_relocatable_2)
.global NAME(non_relocatable_2)
NAME(non_relocatable_2):
    push %ebp
LBB8:
    mov %esp, %ebp
    test %eax, %eax
    jb LBB8
    je LBB7
    leave
    ret
LBB7:
    inc %eax
    test %eax, %eax
    je NAME(non_relocatable_2)
END_FUNCTION(non_relocatable_2)

