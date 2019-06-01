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
#
# Test for iretq in 64 bit mode.
# The code for iretd is commented out, since I haven't been able to find
# a coherent description of what it is supposed to do. (Working to the SDM description
# gives code that SEGVs)
#

# iretd_func:
#         mov $-1,%rax
#         .byte 0xcf

# .type iretdTest, @function
# .global iretdTest
# iretdTest:
#         # We have to build the stack frame ourselves
#         sub     $12,%rsp
#         mov     $-1, %rax
#         mov     %eax,8(%rsp)         #  Write the flags to one
#         mov     %cs, %rax
#         mov     %eax,4(%rsp)
#         lea     here,%rax
#         mov     %eax,0(%rsp)
#         jmp     iretd_func
# here:   
#         ret

iret_func:
        mov $-1,%rax
        iretq

#ifndef TARGET_MAC        
.type iretTest, @function
#endif
.global iretTest
iretTest:
        push    %rbx
        # Move the stack pointer down, so that we can check that the stack pointer
        # is correctly restored by the iretq
        mov     %rsp,%rbx
        sub     $80,%rsp
        mov     %ss,%rax
        push    %rax
        push    %rbx    # Restored stack pointer
        pushfq
        mov     %cs,%rax
        push    %rax
        call    iret_func
        pop     %rbx
        ret
        
