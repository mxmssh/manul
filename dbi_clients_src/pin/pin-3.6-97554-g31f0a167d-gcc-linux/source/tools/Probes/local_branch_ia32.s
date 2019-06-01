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

# this routine should generate an error because the target of a local branch
# is within the probe space.
#
.global NAME(do_nothing)
DECLARE_FUNCTION(do_nothing)

NAME(do_nothing):
    xor %eax, %eax
lab1:
    xor %eax, %eax
    test %eax, %eax
    je lab2

    xor %eax, %eax
    xor %eax, %eax
    jmp lab1

lab2:
    xor %eax, %eax
    xor %eax, %eax

    ret


# This code pattern will result in an error because the call 
# is less than the size of the probe, and when relocated, the 
# push of the original IP will cause a branch into the probe.
#
.global NAME(call_function)
DECLARE_FUNCTION(call_function)

NAME(call_function):
    call NAME(do_nothing)
    push %ebx
    pop %ebx
    ret


# Test an unconditional jump in the middle of the probe area.
#	
.global NAME(good_jump)
DECLARE_FUNCTION(good_jump)

NAME(good_jump):
    xchg %eax, %eax
    jmp lab3

    push %ebx
    pop %ebx

lab3:
    xor %eax, %eax
    xor %eax, %eax

    ret

	
	
# This code should not be able to be relocated because of the
# conditional jump as the last instruction.
#	
.global NAME(fall_thru)
DECLARE_FUNCTION(fall_thru)

NAME(fall_thru):
    xchg %eax, %eax
    jmp lab5

    push %ebx
    pop %ebx

lab5:
    xor %eax, %eax
    xor %eax, %eax
    mov %eax, 0x5
    cmp %eax, 0	
    jz lab5 
    nop


# This code should not be able to be relocated because of the
# jump outside of the function.
#	
.global NAME(bad_jump)
DECLARE_FUNCTION(bad_jump)

NAME(bad_jump):
    xchg %eax, %eax
    jmp lab6

    push %ebx
    pop %ebx

lab6:
    jmp NAME(do_nothing)	
    xor %eax, %eax
    xor %eax, %eax
    ret
	


# This code should not be able to be relocated because of the
# call function.
#	
.global NAME(bad_call)
DECLARE_FUNCTION(bad_call)

NAME(bad_call):
    xchg %eax, %eax
    jmp lab7

    push %ebx
    pop %ebx

lab7:
    call NAME(do_nothing)
    xor %eax, %eax
    xor %eax, %eax
    ret



# This code should not be able to be relocated because of the
# target after the last instruction.
#	
.global NAME(high_target)
DECLARE_FUNCTION(high_target)

NAME(high_target):
    xchg %eax, %eax
    jmp lab8

    push %ebx
    pop %ebx

    xor %eax, %eax
    xor %eax, %eax
    mov %eax, 0x5
    cmp %eax, 0	
    ret
	
lab8:	
    nop
    nop
    nop
	
	
	
