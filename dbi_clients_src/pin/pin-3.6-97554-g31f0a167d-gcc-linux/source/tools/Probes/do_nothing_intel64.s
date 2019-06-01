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

# 
# contains important code patterns
# doesn't actually do anything

# make sure the relocated entry will copy more than one basic block.
.text
.global NAME(do_nothing)
DECLARE_FUNCTION(do_nothing)

NAME(do_nothing):
    test %rax, %rax
    je lab1

    xor %rax, %rax
    xor %rax, %rax

lab1:
    xor %rax, %rax
    xor %rax, %rax

    ret

#make sure the relocated entry will correctly handle a jump as the last
#instruction in the basic block.
.global NAME(nothing_doing)
DECLARE_FUNCTION(nothing_doing)

NAME(nothing_doing):
    xor %rax, %rax
    test %rax, %rax
    je lab1

    xor %rax, %rax
    xor %rax, %rax

lab2:
    xor %rax, %rax
    xor %rax, %rax

    ret


# call should be replaced with a push/jmp when relocated.
#
.global NAME(call_function)
DECLARE_FUNCTION(call_function)

NAME(call_function):
    push %rax
    push %rbx
    call NAME(do_nothing)
    pop %rbx
    pop %rax
    ret


# make sure this code pattern works
#
.global NAME(call_nothing)
DECLARE_FUNCTION(call_nothing)

NAME(call_nothing):
    push %rax
    mov %rax, %rax
    push %rbx
    call NAME(do_nothing)
    pop %rbx
    pop %rax
    ret

#make sure this code pattern works
#
.global NAME(shorty)
DECLARE_FUNCTION(shorty)
	
NAME(shorty):	
	xor %rax, %rax
	jmp lab3
	nop
	nop
	nop
lab3:	
	nop
	nop
	nop
	ret
			
