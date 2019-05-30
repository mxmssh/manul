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
.text
.global cmpxchg8_base
cmpxchg8_base:
    push %ebp
    mov %esp, %ebp
    push %ebx
    push %ecx
    push %edx
    push %esi

    mov 8(%ebp), %esi
    mov $0x1, %eax
    mov $0x1, %edx
    mov $0x2, %ebx
    mov $0x2, %ecx

    cmpxchg8b (%esi)
    jz success1

fail1:
    mov $0, %eax
    jmp end1

success1:
    mov $1, %eax

end1:
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx
    leave
    ret

.global cmpxchg8_plus8
cmpxchg8_plus8:
    push %ebp
    mov %esp, %ebp
    push %ebx
    push %ecx
    push %edx
    push %esi

    mov 8(%ebp), %esi
    mov $0x1, %eax
    mov $0x1, %edx
    mov $0x2, %ebx
    mov $0x2, %ecx

    cmpxchg8b 8(%esi)
    jz success2

fail2:
    mov $0, %eax
    jmp end2

success2:
    mov $1, %eax

end2:
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx
    leave
    ret

.global cmpxchg8_esp
cmpxchg8_esp:
    push %ebp
    mov %esp, %ebp
    push %ebx
    push %ecx
    push %edx
    push %esi

    mov $0x1, %eax
    mov $0x1, %edx
	
    lea  8(%esp),%esp
    mov %eax,(%esp)
    mov %edx,4(%esp)
	
    mov $0x2, %ebx
    mov $0x2, %ecx

    cmpxchg8b (%esp)
    jz success3

fail3:
    mov $0, %eax
    jmp end3

success3:
    mov $1, %eax

end3:
    lea  -8(%esp),%esp
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx
    leave
    ret
	
