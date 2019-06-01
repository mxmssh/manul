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
PUBLIC TestAccessViolations


.686
.model flat, c
.XMM

.code


TestAccessViolations PROC
    push ebx
    push ebp
    push edi
    push esi 
    xor ebx, ebx
    xor edx, edx

    mov eax, 1234h
    mov ecx, 2345h
    mov ebp, 0abcdh
    mov edi, 0bcdeh
    mov esi, 0cdefh

    cmpxchg8b QWORD PTR [edx]
    
    cmp eax, 1234h
    jne ErrorLab
    cmp ecx, 2345h
    jne ErrorLab
    cmp ebx, 0
    jne ErrorLab
    cmp edx, 0
    jne ErrorLab
    cmp ebp, 0abcdh
    jne ErrorLab
    cmp edi, 0bcdeh
    jne ErrorLab
    cmp esi, 0cdefh
    jne ErrorLab
    

    mov eax, 3456h
    mov ecx, 4567h

    xlat
    
    cmp eax, 3456h
    jne ErrorLab
    cmp ecx, 4567h
    jne ErrorLab
    cmp ebx, 0
    jne ErrorLab
    cmp edx, 0
    jne ErrorLab
    cmp ebp, 0abcdh
    jne ErrorLab
    cmp edi, 0bcdeh
    jne ErrorLab
    cmp esi, 0cdefh
    jne ErrorLab


    mov eax, 5678h
    mov ecx, 6789h

    cmpxchg8b QWORD PTR [ebx]

    cmp eax, 5678h
    jne ErrorLab
    cmp ecx, 6789h
    jne ErrorLab
    cmp ebx, 0
    jne ErrorLab
    cmp edx, 0
    jne ErrorLab
    cmp ebp, 0abcdh
    jne ErrorLab
    cmp edi, 0bcdeh
    jne ErrorLab
    cmp esi, 0cdefh
    jne ErrorLab

    mov eax, 1
    jmp RetLab

ErrorLab:
    mov eax, 0
RetLab:
    pop esi
    pop edi
    pop ebp
    pop ebx	
    ret
TestAccessViolations ENDP
    

end
