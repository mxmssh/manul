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
PUBLIC MMXSequence

.386
.XMM
.model flat, c

.code 
MMXSequence PROC
    
    movd        mm0, dword ptr [esp+4]    
    movd        mm1, dword ptr [esp+8] 
    movq        mm7, qword ptr [esp+4]
    movd        mm3, dword ptr [esp+12]   
    pxor        mm2, mm2
    punpcklbw   mm0, mm2
    punpcklbw   mm1, mm2
    punpcklbw   mm3, mm2
    movq        mm5, mm3
    psrlw       mm5, 6
    pcmpeqw     mm4, mm4
    psllw       mm4, 10
    paddw       mm3, mm5
    psubw       mm4, mm3
    pmullw      mm2, mm4
    pmullw      mm1, mm3
    pmullw      mm1, mm3
    psrlw       mm1, 9
    packuswb    mm1, mm1
    mov         ecx, dword ptr [esp+16]
    movq        qword ptr [ecx] , mm7
    movd        eax, mm1
    emms
    ret

MMXSequence ENDP

end
