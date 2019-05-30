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


.686
.model flat, c
COMMENT // use of segment register is not an ERROR
ASSUME NOTHING
.code
TestSegmentedEA PROC
    mov        eax, 18
    mov        ecx, 2
    mov        edx, DWORD PTR fs:[5]
    mov        DWORD PTR fs:[5], edx
    mov        edx, DWORD PTR fs:[eax]
    mov        DWORD PTR fs:[eax], edx
    mov        edx, DWORD PTR fs:[eax+5]
    mov        DWORD PTR fs:[eax+5], edx
    mov        edx, DWORD PTR fs:[ecx*2]
    mov        DWORD PTR fs:[ecx*2], edx
    mov        edx, DWORD PTR fs:[ecx*2+5]
    mov        DWORD PTR fs:[ecx*2+5], edx
    mov        edx, DWORD PTR fs:[eax+ecx]
    mov        DWORD PTR fs:[eax+ecx], edx
    mov        edx, DWORD PTR fs:[eax+ecx*2+5]
    mov        DWORD PTR fs:[eax+ecx*2+5], edx
    ret

TestSegmentedEA ENDP

end