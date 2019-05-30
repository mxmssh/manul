; BEGIN_LEGAL 
; Intel Open Source License 
; 
; Copyright (c) 2002-2017 Intel Corporation. All rights reserved.
;  
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are
; met:
; 
; Redistributions of source code must retain the above copyright notice,
; this list of conditions and the following disclaimer.  Redistributions
; in binary form must reproduce the above copyright notice, this list of
; conditions and the following disclaimer in the documentation and/or
; other materials provided with the distribution.  Neither the name of
; the Intel Corporation nor the names of its contributors may be used to
; endorse or promote products derived from this software without
; specific prior written permission.
;  
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
; ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
; LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
; A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
; ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
; SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
; LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
; DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
; THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
; END_LEGAL
.686
.model flat, c

COMMENT // contains important code patterns
COMMENT // doesn't actually do anything
COMMENT // make sure the relocated entry will copy more than one basic block.

ASSUME NOTHING
.code
PUBLIC do_nothing
do_nothing PROC

    test eax, eax
    je lab1

    xor eax, eax
    xor eax, eax

lab1:
    xor eax, eax
    xor eax, eax

    ret
do_nothing ENDP

COMMENT // make sure the relocated entry will correctly handle a jump as the last
COMMENT // instruction in the basic block.

PUBLIC nothing_doing
nothing_doing PROC
    test eax, eax
    test eax, eax
    je lab2

    xor eax, eax
    xor eax, eax

lab2:
    xor eax, eax
    xor eax, eax

    ret
nothing_doing ENDP

COMMENT // call should be replaced with a push/jmp when relocated.

PUBLIC call_function
call_function PROC

    push ebx
    call do_nothing
    pop ebx
    ret
call_function ENDP

COMMENT // make sure this code pattern works

PUBLIC call_nothing
call_nothing PROC
    push eax
    mov  eax, eax
    push ebx
    call do_nothing
    pop  ebx
    pop  eax
    ret
call_nothing ENDP

END
