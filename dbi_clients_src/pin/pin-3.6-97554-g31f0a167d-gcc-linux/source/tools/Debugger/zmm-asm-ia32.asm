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
include asm_macros.inc

PROLOGUE

.code

LoadYmm0 PROC
    mov dword ptr [esp]+4, ecx

    ; This is "VMOVDQU ymm0, YMMWORD PTR [ecx]".  We directly specify the machine code,
    ; so this test runs even when the compiler doesn't support AVX.
    db 0C5h, 0FEh, 06Fh, 001h

    ret
LoadYmm0 ENDP

LoadZmm0 PROC
    mov dword ptr [esp]+4, ecx

    ; This is "VMOVUPD zmm0, ZMMWORD PTR [ecx]".  We directly specify the machine code,
    ; so this test runs even when the compiler doesn't support AVX512.
    db 062h, 0F1h, 0FDh, 048h, 010h, 001h

    ret
LoadZmm0 ENDP

LoadK0 PROC
    mov dword ptr [esp]+4, ecx

    ; This is "KMOVW k0, WORD PTR [ecx]".  We directly specify the machine code,
    ; so this test runs even when the compiler doesn't support AVX512.
    db 0C5h, 0F8h, 090h, 001h

    ret
LoadK0 ENDP

end
