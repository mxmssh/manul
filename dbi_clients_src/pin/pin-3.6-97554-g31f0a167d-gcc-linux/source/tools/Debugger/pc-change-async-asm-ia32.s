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
.globl One
    .type   One, @function
One:
    /*
     * We force the thread to get an ASYNC_BREAK on this instruction, which is the
     * target of an indirect jump.
     */
    jmp     *%eax

    /*
     * The tool should change the PC to Two() when the ASYNC_BREAK occurs, so the
     * rest of this function should be skipped.
     */
    movl    $1, %eax
    ret


.globl Two
    .type   Two, @function
Two:
    movl    $2, %eax
    ret


.globl GetValue
    .type   GetValue, @function
GetValue:
    mov     4(%esp), %eax
    call    *%eax
    ret


.globl Breakpoint
    .type   Breakpoint, @function
Breakpoint:
    mov     $0, %eax
.L1:
    add     $1, %eax
    cmp     $99999999, %eax
    jbe     .L1

    /*
     * The debugger places a breakpoint here, but the tool intercepts the BREAKPOINT event.
     * If the breakpoint triggers before the other thread enters the One() function, the tool
     * squashes the breakpoint and moves the PC back to the start of Breakpoint().  The delay
     * loop waits a while, and then the breakpoint re-triggers.  This repeats until the other
     * thread is at the first instruction in One(), so the breakpoint is guaranteed to trigger
     * when the other thread is at that instruction.
     */
.globl BreakpointLocation
    .type   BreakpointLocation, @function
BreakpointLocation:
    nop

    ret
