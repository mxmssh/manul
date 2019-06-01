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
PUBLIC cmpxchg8_base
PUBLIC cmpxchg8_plus8

.686
.model flat, c
.code
cmpxchg8_base PROC
    push ebp
    mov ebp, esp

    push ebx
    push ecx
    push edx
    push esi

    mov esi, [ ebp + 8 ]
    mov eax, 1
    mov edx, 1
    mov ebx, 2
    mov ecx, 2

    cmpxchg8b QWORD PTR [esi]
    jz success1

fail1:
    mov eax, 0
    jmp end1

success1:
    mov eax, 1

end1:
    pop esi
    pop edx
    pop ecx
    pop ebx
    leave
    ret

cmpxchg8_base ENDP

cmpxchg8_plus8 PROC
    push ebp
    mov ebp, esp

    push ebx
    push ecx
    push edx
    push esi

    mov esi, [ ebp + 8 ]
    mov eax, 1
    mov edx, 1
    mov ebx, 2
    mov ecx, 2

    cmpxchg8b QWORD PTR [ esi + 8 ]
    jz success2

fail2:
    mov eax, 0
    jmp end2

success2:
    mov eax, 1

end2:
    pop esi
    pop edx
    pop ecx
    pop ebx
    leave
    ret

cmpxchg8_plus8 ENDP

cmpxchg8_esp PROC
    push ebp
    mov ebp, esp

    push ebx
    push ecx
    push edx
    push esi

    mov eax, 1
    mov edx, 1
	
    lea esp,[esp + 8]
    mov [esp],eax
    mov [esp+4],edx
	
    mov ebx, 2
    mov ecx, 2

    cmpxchg8b QWORD PTR [ esp ]
    jz success3

fail3:
    mov eax, 0
    jmp end3

success3:
    mov eax, 1

end3:
    lea esp,[esp-8]
    pop esi
    pop edx
    pop ecx
    pop ebx
    leave
    ret

cmpxchg8_esp ENDP
	
end
