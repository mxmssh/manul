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
PUBLIC TestSegmentedEA


.code
TestSegmentedEA PROC
    mov        rax, 18
    mov        rcx, 2
    mov        rdx, QWORD PTR gs:[5]
    mov        QWORD PTR gs:[5], rdx
    mov        rdx, QWORD PTR gs:[rax]
    mov        QWORD PTR gs:[rax], rdx
    mov        rdx, QWORD PTR gs:[rax+5]
    mov        QWORD PTR gs:[rax+5], rdx
    mov        rdx, QWORD PTR gs:[rcx*2]
    mov        QWORD PTR gs:[rcx*2], rdx
    mov        rdx, QWORD PTR gs:[rcx*2+5]
    mov        QWORD PTR gs:[rcx*2+5], rdx
    mov        rdx, QWORD PTR gs:[rax+rcx]
    mov        QWORD PTR gs:[rax+rcx], rdx
    mov        rdx, QWORD PTR gs:[rax+rcx*2+5]
    mov        QWORD PTR gs:[rax+rcx*2+5], rdx
    ret

TestSegmentedEA ENDP

end
