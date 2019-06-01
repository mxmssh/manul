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
.data
dummy QWORD 0, 0, 0 , 0

.code

Fld1_a PROC
    fld1
	fld1
	fld1
    ret
Fld1_a ENDP


Fld1_b PROC
    fld1
	fld1
	fld1
    ret
Fld1_b ENDP

FldzToTop3_a PROC
    fstp dummy
	fstp dummy
	fstp dummy
	fldz
	fldz
	fldz
    ret

FldzToTop3_a ENDP

mmx_save PROC buf:QWORD
  fxsave [rcx]
  RET
mmx_save ENDP

mmx_restore PROC buf:QWORD
  fxrstor [rcx]
  RET
mmx_restore ENDP

set_xmm_reg0 PROC xmm_reg:DWORD
  
  movdqu xmm0,  [rcx]
  RET
set_xmm_reg0 ENDP

get_xmm_reg0 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm0
  RET
get_xmm_reg0 ENDP

set_xmm_reg1 PROC xmm_reg:DWORD
  
  movdqu xmm1,  [rcx]
  RET
set_xmm_reg1 ENDP

get_xmm_reg1 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm1
  RET
get_xmm_reg1 ENDP

set_xmm_reg2 PROC xmm_reg:DWORD
  
  movdqu xmm2,  [rcx]
  RET
set_xmm_reg2 ENDP

get_xmm_reg2 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm2
  RET
get_xmm_reg2 ENDP

set_xmm_reg3 PROC xmm_reg:DWORD
  
  movdqu xmm3,  [rcx]
  RET
set_xmm_reg3 ENDP

get_xmm_reg3 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm3
  RET
get_xmm_reg3 ENDP

set_xmm_reg4 PROC xmm_reg:DWORD
  
  movdqu xmm4,  [rcx]
  RET
set_xmm_reg4 ENDP

get_xmm_reg4 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm4
  RET
get_xmm_reg4 ENDP

set_xmm_reg5 PROC xmm_reg:DWORD
  
  movdqu xmm5,  [rcx]
  RET
set_xmm_reg5 ENDP

get_xmm_reg5 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm5
  RET
get_xmm_reg5 ENDP

set_xmm_reg6 PROC xmm_reg:DWORD
  
  movdqu xmm6,  [rcx]
  RET
set_xmm_reg6 ENDP

get_xmm_reg6 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm6
  RET
get_xmm_reg6 ENDP

set_xmm_reg7 PROC xmm_reg:DWORD
  
  movdqu xmm7,  [rcx]
  RET
set_xmm_reg7 ENDP

get_xmm_reg7 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm7
  RET
get_xmm_reg7 ENDP


set_xmm_reg8 PROC xmm_reg:DWORD
  
  movdqu xmm8,  [rcx]
  RET
set_xmm_reg8 ENDP

get_xmm_reg8 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm8
  RET
get_xmm_reg8 ENDP

set_xmm_reg9 PROC xmm_reg:DWORD
  
  movdqu xmm9,  [rcx]
  RET
set_xmm_reg9 ENDP

get_xmm_reg9 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm9
  RET
get_xmm_reg9 ENDP

set_xmm_reg10 PROC xmm_reg:DWORD
  
  movdqu xmm10,  [rcx]
  RET
set_xmm_reg10 ENDP

get_xmm_reg10 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm10
  RET
get_xmm_reg10 ENDP

set_xmm_reg11 PROC xmm_reg:DWORD
  
  movdqu xmm11,  [rcx]
  RET
set_xmm_reg11 ENDP

get_xmm_reg11 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm11
  RET
get_xmm_reg11 ENDP

set_xmm_reg12 PROC xmm_reg:DWORD
  
  movdqu xmm12,  [rcx]
  RET
set_xmm_reg12 ENDP

get_xmm_reg12 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm12
  RET
get_xmm_reg12 ENDP

set_xmm_reg13 PROC xmm_reg:DWORD
  
  movdqu xmm13,  [rcx]
  RET
set_xmm_reg13 ENDP

get_xmm_reg13 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm13
  RET
get_xmm_reg13 ENDP

set_xmm_reg14 PROC xmm_reg:DWORD
  
  movdqu xmm14,  [rcx]
  RET
set_xmm_reg14 ENDP

get_xmm_reg14 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm14
  RET
get_xmm_reg14 ENDP

set_xmm_reg15 PROC xmm_reg:DWORD
  
  movdqu xmm15,  [rcx]
  RET
set_xmm_reg15 ENDP

get_xmm_reg15 PROC xmm_reg:DWORD
  
  movdqu [rcx], xmm15
  RET
get_xmm_reg15 ENDP

set_mmx_reg0 PROC mmx_reg:QWORD
  movq   mm0,   [rcx]
  RET
set_mmx_reg0 ENDP

get_mmx_reg0 PROC mmx_reg:QWORD
  movq   [rcx], mm0
  RET
get_mmx_reg0 ENDP

end
