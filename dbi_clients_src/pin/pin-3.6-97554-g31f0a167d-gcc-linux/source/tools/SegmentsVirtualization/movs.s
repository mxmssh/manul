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
.global MovsTest
.type MovsTest, @function

MovsTest:
push %ebp
mov %esp, %ebp
mov 0x8(%ebp), %esi
mov 0xc(%ebp), %edi
movsl %fs:(%esi), %es:(%edi)
mov %es:-4(%edi), %eax
leave
ret

.global MaskMovqTest
.type MaskMovqTest, @function

MaskMovqTest:
push %ebp
mov %esp, %ebp
mov 0x8(%ebp), %edi    # first operand - an offset under fs
mov 0xc(%ebp), %esi    # second operand - the number to be copied
movl $0xffffffff, %eax
movd %eax, %xmm0       # mask
movd %esi, %xmm1       # the number to be copied
.byte 0x64
maskmovdqu %xmm0, %xmm1
leave
ret

.global PushPopTest
.type PushPopTest, @function

PushPopTest:
push %ebp
mov %esp, %ebp
mov 0x8(%ebp), %edi    # first operand - an offset under fs
mov 0xc(%ebp), %esi    # second operand - the number to be copied
mov %esi, %fs:(%edi)
push %fs:(%edi)
addl $4, %edi
pop %fs:(%edi)
mov %fs:(%edi), %eax
leave
ret

.global CallTest
.type CallTest, @function

CallTest:
push %ebp
mov %esp, %ebp
mov 0x8(%ebp), %edi    # first operand - an offset under fs
call *%fs:(%edi)
leave
ret

