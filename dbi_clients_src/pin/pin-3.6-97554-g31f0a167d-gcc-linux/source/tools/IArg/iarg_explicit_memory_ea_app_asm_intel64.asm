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
PUBLIC DoExplicitMemoryOps

.code

extern globalVar:qword
extern dynVar:qword
extern lblPtr:qword
extern autoVarPtr:qword

 DoExplicitMemoryOps PROC
    push rbp
    mov rbp, rsp
    sub rsp, 16

lbl1:
    lea rax, globalVar

lbl2:
    lea rax, [rsp + 8] ; <-- this will be autoVar

    mov rbx, [dynVar]
lbl3:
    lea rax, [rbx]

    mov rax, 0cafebabeH
lbl4:
    lea rax, [rax]

lbl5:
    lea rax, [0deadbeeH]
lbl6:
    mov rax, globalVar

lbl7:
    mov [rsp + 8], rax

    lea rax, [rsp + 8]
    mov [autoVarPtr], rax

lbl8:
    lea rax, fs:[-8]

    mov rax, 0deadbeefH
lbl9:
    lea rax, fs:[rax]

    mov rbx, [lblPtr]
    mov rax, offset lbl1
    mov [rbx], rax
    mov rax, offset lbl2
    mov [rbx+8], rax
    mov rax, offset lbl3
    mov [rbx+16], rax
    mov rax, offset lbl4
    mov [rbx+24], rax
    mov rax, offset lbl5
    mov [rbx+32], rax
    mov rax, offset lbl6
    mov [rbx+40], rax
    mov rax, offset lbl7
    mov [rbx+48], rax
    mov rax, offset lbl8
    mov [rbx+56], rax
    mov rax, offset lbl9
    mov [rbx+64], rax

    mov rsp, rbp
    pop rbp
    ret
DoExplicitMemoryOps ENDP

end
