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
# this code pattern tests an ip-relative displacement
# on a call instruction in the probe area.
	
    .text
.globl Bar
    .type   Bar, @function
Bar:
    pushq   %rbp
    movq    %rsp, %rbp
    leave
    ret

.globl pf
    .text
    .align 8
    .type   pf, @object
    .size   pf, 8
pf:
    .quad   Bar

    .text
.globl Foo
    .type   Foo, @function
Foo:
    pushq   %rbp
    movq    %rsp, %rbp

    call    *(pf-.-6)(%rip)  # 6 = size of this call instruction

    leave
    ret
	
# this code pattern tests an ip-relative jmp in the probe area.
	
.globl pt
    .text
    .align 8
    .type   pt, @object
    .size   pt, 8
pt:
    .quad   Bar

    .text
.globl Haha
    .type   Haha, @function
Haha:
    jmp    *(pt-.-6)(%rip)  # 6 = size of this instruction
