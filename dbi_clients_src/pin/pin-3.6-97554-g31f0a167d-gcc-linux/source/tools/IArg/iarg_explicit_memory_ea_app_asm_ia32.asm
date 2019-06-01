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


.686
.model flat, c
extern globalVar:dword
extern dynVar:dword
extern lblPtr:dword
extern autoVarPtr:dword

COMMENT // use of segment register is not an ERROR
ASSUME FS:NOTHING

.code
 ALIGN 4
 DoExplicitMemoryOps PROC
    push ebp
    mov ebp, esp
    sub esp, 16

lbl1:
    lea eax, globalVar

lbl2:
    lea eax, [esp + 8] ; <--- this will be autoVar

    mov ebx, [dynVar]
lbl3:
    lea eax, [ebx]

    mov eax, 0cafebabeH
lbl4:
    lea eax, [eax]

    xor eax, eax
lbl5:
    lea eax, [eax+0deadbeeH]

lbl6:
    mov eax, globalVar

lbl7:
    mov [esp + 8], eax

lbl8:
    lea eax, fs:[-8]

    mov eax, 0deadbeefH
lbl9:
    lea eax, fs:[eax]

    lea eax, [esp + 8]
    mov [autoVarPtr], eax

    mov ebx, [lblPtr]
    mov eax, offset lbl1
    mov [ebx], eax
    mov eax, offset lbl2
    mov [ebx+4], eax
    mov eax, offset lbl3
    mov [ebx+8], eax
    mov eax, offset lbl4
    mov [ebx+12], eax
    mov eax, offset lbl5
    mov [ebx+16], eax
    mov eax, offset lbl6
    mov [ebx+20], eax
    mov eax, offset lbl7
    mov [ebx+24], eax
    mov eax, offset lbl8
    mov [ebx+28], eax
    mov eax, offset lbl9
    mov [ebx+32], eax

    mov esp, ebp
    pop ebp
    ret
DoExplicitMemoryOps ENDP

end
