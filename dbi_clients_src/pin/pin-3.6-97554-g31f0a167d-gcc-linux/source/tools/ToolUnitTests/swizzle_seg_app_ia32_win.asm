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
PUBLIC SegAccessRtn


.686
.model flat, c
COMMENT // replacement memory operands with segment registers
ASSUME NOTHING
.code
SegAccessRtn PROC
	push ebp
	mov  ebp, esp
	push ecx
	mov  eax, DWORD PTR [ebp+8h]
	mov  DWORD PTR fs:[10h], eax
	mov  eax, DWORD PTR fs:[10h]
	mov  ecx, 10h
	mov  eax, DWORD PTR fs:[ecx]
	mov  DWORD PTR fs:[14h], 100
	mov  ecx, 10h
	add  eax, DWORD PTR fs:[ecx + 4]	
	pop  ecx
	leave
	ret
	 

SegAccessRtn ENDP

SegAccessStrRtn PROC

	push ebp
	mov  ebp, esp
	push esi
	
	mov  eax, DWORD PTR [ebp+8h]
	mov  DWORD PTR fs:[14h], eax
	mov  esi, 14h
	lods DWORD PTR fs:[esi]
	
	pop esi
	leave
	ret

SegAccessStrRtn ENDP

dummy PROC
    nop
dummy ENDP

end






