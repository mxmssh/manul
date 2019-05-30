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
    
#ifdef TARGET_MAC
.globl _TestAccessViolations
_TestAccessViolations:
#else
.globl TestAccessViolations
    .type	TestAccessViolations, @function
TestAccessViolations:
#endif

    push %ebx
    push %ebp
    push %edi
    push %esi 
    xor %ebx, %ebx
    xor %edx, %edx

    movl  $0x1234, %eax
    movl  $0x2345, %ecx
    movl  $0xabcd, %ebp
    movl  $0xbcde, %edi
    movl  $0xcdef, %esi

    cmpxchg8b  (%edx)
    
    cmp  $0x1234, %eax
    jne ErrorLab
    cmp  $0x2345, %ecx
    jne ErrorLab
    cmp  $0, %ebx
    jne ErrorLab
    cmp  $0, %edx
    jne ErrorLab
    cmp  $0xabcd, %ebp
    jne ErrorLab
    cmp  $0xbcde, %edi
    jne ErrorLab
    cmp  $0xcdef, %esi
    jne ErrorLab
    

    movl  $0x3456, %eax
    movl  $0x4567, %ecx

    xlat
    

    cmp  $0x3456, %eax
    jne ErrorLab
    cmp  $0x4567, %ecx
    jne ErrorLab
    cmp  $0, %ebx
    jne ErrorLab
    cmp  $0, %edx
    jne ErrorLab
    cmp  $0xabcd, %ebp
    jne ErrorLab
    cmp  $0xbcde, %edi
    jne ErrorLab
    cmp  $0xcdef, %esi
    jne ErrorLab


    movl  $0x5678, %eax
    movl  $0x6789, %ecx

    cmpxchg8b  (%ebx)

    cmp  $0x5678, %eax
    jne ErrorLab
    cmp  $0x6789, %ecx
    jne ErrorLab
    cmp  $0, %ebx
    jne ErrorLab
    cmp  $0, %edx
    jne ErrorLab
    cmp  $0xabcd, %ebp
    jne ErrorLab
    cmp  $0xbcde, %edi
    jne ErrorLab
    cmp  $0xcdef, %esi
    jne ErrorLab

    movl  $1, %eax
    jmp RetLab

ErrorLab:
    movl  $0, %eax
RetLab:
    pop %esi
    pop %edi
    pop %ebp
    pop %ebx	
    ret	


