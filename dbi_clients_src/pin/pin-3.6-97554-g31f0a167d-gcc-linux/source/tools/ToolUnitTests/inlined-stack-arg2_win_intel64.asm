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
PUBLIC Analysis_func
PUBLIC Analysis_func_immed
PUBLIC Analysis_func_reg_overwrite



.code
Analysis_func PROC
    push      rbx
    push      rsi
    push      rdi
    sub       rsp, 16
    mov       QWORD PTR[rsp], rbx
    mov       QWORD PTR[rsp+8], rcx
    mov       rbx, QWORD PTR [rsp+80]
    mov       ebx, DWORD PTR [rsp+80]
    mov       bx,  WORD PTR [rsp+80]
    mov       bl,  BYTE PTR [rsp+80]
    cmp       rbx, QWORD PTR [rsp+80]
    cmp       ebx, DWORD PTR [rsp+80]
    cmp       bx,  WORD PTR [rsp+80]
    cmp       bl,  BYTE PTR [rsp+80]
    cmp       QWORD PTR [rsp+80], rbx
    cmp       DWORD PTR [rsp+80], ebx
    cmp       WORD PTR [rsp+80], bx
    cmp       BYTE PTR [rsp+80], bl
    add       rbx, QWORD PTR [rsp+80]
    add       ebx, DWORD PTR [rsp+80]
    add       bx,  WORD PTR [rsp+80]
    add       bl,  BYTE PTR [rsp+80]
    adc       rbx, QWORD PTR [rsp+80]
    adc       ebx, DWORD PTR [rsp+80]
    adc       bx,  WORD PTR [rsp+80]
    adc       bl,  BYTE PTR [rsp+80]
    sub       rbx, QWORD PTR [rsp+80]
    sub       ebx, DWORD PTR [rsp+80]
    sub       bx,  WORD PTR [rsp+80]
    sub       bl,  BYTE PTR [rsp+80]
    sbb       rbx, QWORD PTR [rsp+80]
    sbb       ebx, DWORD PTR [rsp+80]
    sbb       bx,  WORD PTR [rsp+80]
    sbb       bl,  BYTE PTR [rsp+80]
    xor       rbx, QWORD PTR [rsp+80]
    xor       ebx, DWORD PTR [rsp+80]
    xor       bx,  WORD PTR [rsp+80]
    xor       bl,  BYTE PTR [rsp+80]
    or        rbx, QWORD PTR [rsp+80]
    or        ebx, DWORD PTR [rsp+80]
    or        bx,  WORD PTR [rsp+80]
    or        bl,  BYTE PTR [rsp+80]
    movzx     rbx, WORD PTR [rsp+80]
    movzx     rbx, BYTE PTR [rsp+80]
    movzx     ebx, BYTE PTR [rsp+80]
    movzx     bx, BYTE PTR [rsp+80]
    movsx     rbx, WORD PTR [rsp+80]
    movsx     rbx, BYTE PTR [rsp+80]
    movsx     ebx, BYTE PTR [rsp+80]
    movsx     bx, BYTE PTR [rsp+80]
    mov       rbx, QWORD PTR[rsp]
    mov       rcx, QWORD PTR[rsp+8] 
    add       rsp, 16
    pop rdi
    pop rsi
    pop rbx
    ret

Analysis_func ENDP

Analysis_func_immed PROC
    push      rbx
    mov       rbx, QWORD PTR [rsp+48]
    mov       ebx, DWORD PTR [rsp+48]
    mov       bx,  WORD PTR [rsp+48]
    mov       bl,  BYTE PTR [rsp+48]
    cmp       rbx, QWORD PTR [rsp+48]
    cmp       ebx, DWORD PTR [rsp+48]
    cmp       bx,  WORD PTR [rsp+48]
    cmp       bl,  BYTE PTR [rsp+48]
    cmp       QWORD PTR [rsp+48], rbx
    cmp       DWORD PTR [rsp+48], ebx
    cmp       WORD PTR [rsp+48],  bx
    cmp       BYTE PTR [rsp+48],  bl
    cmp       QWORD PTR [rsp+48], 0baadf00dH
    cmp       DWORD PTR [rsp+48], 0baadf00dH
    cmp       WORD PTR [rsp+48],  0baadH
    cmp       BYTE PTR [rsp+48],  0baH
    add       rbx, QWORD PTR [rsp+48]
    add       ebx, DWORD PTR [rsp+48]
    add       bx,  WORD PTR [rsp+48]
    add       bl,  BYTE PTR [rsp+48]
    adc       rbx, QWORD PTR [rsp+48]
    adc       ebx, DWORD PTR [rsp+48]
    adc       bx,  WORD PTR [rsp+48]
    adc       bl,  BYTE PTR [rsp+48]
    sub       rbx, QWORD PTR [rsp+48]
    sub       ebx, DWORD PTR [rsp+48]
    sub       bx,  WORD PTR [rsp+48]
    sub       bl,  BYTE PTR [rsp+48]
    sbb       rbx, QWORD PTR [rsp+48]
    sbb       ebx, DWORD PTR [rsp+48]
    sbb       bx,  WORD PTR [rsp+48]
    sbb       bl,  BYTE PTR [rsp+48]
    xor       rbx, QWORD PTR [rsp+48]
    xor       ebx, DWORD PTR [rsp+48]
    xor       bx,  WORD PTR [rsp+48]
    xor       bl,  BYTE PTR [rsp+48]
    or        rbx, QWORD PTR [rsp+48]
    or        ebx, DWORD PTR [rsp+48]
    or        bx,  WORD PTR [rsp+48]
    or        bl,  BYTE PTR [rsp+48]
    movzx     rbx,  WORD PTR [rsp+48]
    movzx     rbx,  BYTE PTR [rsp+48]
    movzx     ebx,  BYTE PTR [rsp+48]
    movzx     bx,  BYTE PTR [rsp+48]
    movsx     rbx,  WORD PTR [rsp+48]
    movsx     rbx,  BYTE PTR [rsp+48]
    movsx     bx,  BYTE PTR [rsp+48]
    movsx     ebx,  BYTE PTR [rsp+48]
    pop rbx
    ret

Analysis_func_immed ENDP

Analysis_func_reg_overwrite PROC
    push      rbx
    push      rsi
    push      rdi
    sub       rsp, 16
    mov       esi, eax
    mov       QWORD PTR[rsp], rbx
    mov       QWORD PTR[rsp+8], rcx
    mov       rbx, QWORD PTR [rsp+80]
    mov       ebx, DWORD PTR [rsp+80]
    mov       bx,  WORD PTR [rsp+80]
    mov       bl,  BYTE PTR [rsp+80]
    cmp       rbx, QWORD PTR [rsp+80]
    cmp       ebx, DWORD PTR [rsp+80]
    cmp       bx,  WORD PTR [rsp+80]
    cmp       bl,  BYTE PTR [rsp+80]
    cmp       QWORD PTR [rsp+80], rbx
    cmp       DWORD PTR [rsp+80], ebx
    cmp       WORD PTR [rsp+80], bx
    cmp       BYTE PTR [rsp+80], bl
    add       rbx, QWORD PTR [rsp+80]
    add       ebx, DWORD PTR [rsp+80]
    add       bx,  WORD PTR [rsp+80]
    add       bl,  BYTE PTR [rsp+80]
    adc       rbx, QWORD PTR [rsp+80]
    adc       ebx, DWORD PTR [rsp+80]
    adc       bx,  WORD PTR [rsp+80]
    adc       bl,  BYTE PTR [rsp+80]
    sub       rbx, QWORD PTR [rsp+80]
    sub       ebx, DWORD PTR [rsp+80]
    sub       bx,  WORD PTR [rsp+80]
    sub       bl,  BYTE PTR [rsp+80]
    sbb       rbx, QWORD PTR [rsp+80]
    sbb       ebx, DWORD PTR [rsp+80]
    sbb       bx,  WORD PTR [rsp+80]
    sbb       bl,  BYTE PTR [rsp+80]
    xor       rbx, QWORD PTR [rsp+80]
    xor       ebx, DWORD PTR [rsp+80]
    xor       bx,  WORD PTR [rsp+80]
    xor       bl,  BYTE PTR [rsp+80]
    or        rbx, QWORD PTR [rsp+80]
    or        ebx, DWORD PTR [rsp+80]
    or        bx,  WORD PTR [rsp+80]
    or        bl,  BYTE PTR [rsp+80]
    movzx     rbx, WORD PTR [rsp+80]
    movzx     rbx, BYTE PTR [rsp+80]
    movzx     bx, BYTE PTR [rsp+80]
    movzx     ebx, BYTE PTR [rsp+80]
    movsx     rbx, WORD PTR [rsp+80]
    movsx     rbx, BYTE PTR [rsp+80]
    movsx     bx, BYTE PTR [rsp+80]
    movsx     ebx, BYTE PTR [rsp+80]
    mov       rbx, QWORD PTR[rsp]
    mov       rcx, QWORD PTR[rsp+8] 
    add       rsp, 16
    pop rdi
    pop rsi
    pop rbx
    ret

Analysis_func_reg_overwrite ENDP

end