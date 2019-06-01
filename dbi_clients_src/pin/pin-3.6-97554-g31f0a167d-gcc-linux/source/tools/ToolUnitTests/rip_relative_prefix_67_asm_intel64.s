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
    .globl func_is_5

# Return 1 if function argument equals to 5, Return 0 othersize.
# --- func_is_5(int)
func_is_5:
# parameter 1: %edi
L1:
        # 67H prefix (Address-size override prefix) which mean use low 32 bit of RIP
        # Copy 5 to RAX
        .byte 0x67
        movq      var@GOTPCREL(%rip), %rax
        movl      (%rax), %edx
        xorl      %eax, %eax
        cmpl      %edx, %edi
        sete      %al
        ret
L2:
    .type   func_is_5,@function
    .size   func_is_5,.-func_is_5
    .data
# -- End  func_is_5
    .data
    .align 4
    .globl var
var:
    .long   5
    .type   var,@object
    .size   var,4

    .globl func_is_5_size
func_is_5_size:
    .long   L2-L1
    .type   func_is_5_size,@object
    .size   func_is_5_size,4



