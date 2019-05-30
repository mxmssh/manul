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
/*! @file
 *  A small test which doesn't link in a C runtime.
 *  It provides a  menagerie of different unpleasant push instructions, and
 *  other instructions which exercise unpleasant corners of the instruction set,
 *  such as access to (virtualised) segment registers.
 */
        .data
inputval:  .long  0x01234567
outputval: .long  0x00000000

        .text
.globl _start
_start:

        # Test 1 push rip relative indirect value to the stack
        mov     $1, %rdi       # %rdi holds the test number, returned as status of exit on failure
        pushq   inputval(%rip)
        movq    inputval(%rip), %rax
        cmp     (%rsp),%rax
        jne     1f

        # Test 2 pop rip relative indirect value from the stack
        incq    %rdi
        popq    outputval(%rip) # Value should be the same as the value previously pushed.
        cmp     outputval(%rip),%rax
        jne     1f

        # Test 3 call a routine through rip.
        incq    %rdi   
        mov     $1,%rax
        lea     stubRoutine(%rip),%r13
        mov     %r13,outputval(%rip)
        call    *outputval(%rip)
        cmp     $0,%rax
        jne     1f

        # Exit
	movq	$0,%rdi		# first argument: exit code
1:  
	movq	$231,%rax	# system call number (sys_exit)
	syscall 		# call kernel

        # Paranoia in case we don't exit
        # Force a SEGV
        movq    $0,0

# Null routine
stubRoutine:
        mov     $0,%rax
        ret
