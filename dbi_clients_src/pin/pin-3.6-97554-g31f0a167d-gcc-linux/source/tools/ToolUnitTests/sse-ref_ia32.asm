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
.386
.XMM
.model flat, c

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

mmx_save PROC buf:DWORD
  push   eax
  mov    eax,  buf
  fxsave [eax]
  pop    eax
  RET
mmx_save ENDP

mmx_restore PROC buf:DWORD
  push    eax
  mov     eax,  buf 
  fxrstor [eax]
  pop     eax
  RET
mmx_restore ENDP

set_xmm_reg0 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm0,  [eax]
  RET
set_xmm_reg0 ENDP

get_xmm_reg0 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm0
  RET
get_xmm_reg0 ENDP

set_xmm_reg1 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm1,  [eax]
  RET
set_xmm_reg1 ENDP

get_xmm_reg1 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm1
  RET
get_xmm_reg1 ENDP

set_xmm_reg2 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm2,  [eax]
  RET
set_xmm_reg2 ENDP

get_xmm_reg2 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm2
  RET
get_xmm_reg2 ENDP

set_xmm_reg3 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm3,  [eax]
  RET
set_xmm_reg3 ENDP

get_xmm_reg3 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm3
  RET
get_xmm_reg3 ENDP

set_xmm_reg4 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm4,  [eax]
  RET
set_xmm_reg4 ENDP

get_xmm_reg4 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm4
  RET
get_xmm_reg4 ENDP

set_xmm_reg5 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm5,  [eax]
  RET
set_xmm_reg5 ENDP

get_xmm_reg5 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm5
  RET
get_xmm_reg5 ENDP

set_xmm_reg6 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm6,  [eax]
  RET
set_xmm_reg6 ENDP

get_xmm_reg6 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm6
  RET
get_xmm_reg6 ENDP

set_xmm_reg7 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu xmm7,  [eax]
  RET
set_xmm_reg7 ENDP

get_xmm_reg7 PROC xmm_reg:DWORD
  mov    eax,   xmm_reg
  movdqu [eax], xmm7
  RET
get_xmm_reg7 ENDP
set_mmx_reg0 PROC mmx_reg:DWORD
  mov    eax,   mmx_reg
  movq   mm0,   [eax]
  RET
set_mmx_reg0 ENDP

get_mmx_reg0 PROC mmx_reg:DWORD
  mov    eax,   mmx_reg
  movq   [eax], mm0
  RET
get_mmx_reg0 ENDP

end
