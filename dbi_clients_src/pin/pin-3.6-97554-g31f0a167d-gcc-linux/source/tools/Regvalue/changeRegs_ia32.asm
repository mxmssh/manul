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
PUBLIC ChangeRegsWrapper
PUBLIC ChangeRegs
PUBLIC SaveRegsToMem

.686
.XMM
.model flat, c
extern gprval:dword
extern agprval:dword
extern stval:real10
extern astval:real10
extern xmmval:xmmword
extern axmmval:xmmword
IFDEF CONTEXT_USING_AVX
extern ymmval:ymmword
extern aymmval:ymmword
ENDIF
IFDEF CONTEXT_USING_AVX512F
extern zmmval:zmmword
extern azmmval:zmmword
extern opmaskval:qword
extern aopmaskval:qword
ENDIF
extern fpSaveArea:dword

.code

; void ChangeRegsWrapper();
; This wrapper saves and restores the registers used by ChangeRegs.
; This is done in the wrapper since we want ChangeRegs to actually
; change the register values but not to affect the application itself.
; The tool may intercept ChangeRegs and replace it with its own function.
;
; Register usage:
; eax   - used (implicitly) by xsave
; ebx   - used for testing the gpr values
; ecx   - used for holding a pointer to the fp save area (used by fxsave)
; edx   - used (implicitly) by xsave
; st0   - used (implicitly) for loading a value to the FPU stack
; st2   - used for testing the FPU values
; xmm0  - used for testing the sse (xmm) values
; ymm1  - used for testing the avx (ymm) values
; zmm5  - used for testing the avx512 (zmm) values
; k3    - used for testing the opmask register values
ChangeRegsWrapper PROC
    ; Save the necessary GPRs
    push    eax
    push    ebx
    push    ecx
    push    edx

IFDEF CONTEXT_USING_AVX512F
    ; Save the necessary mask registers
    kmovw   eax, k3
    push    eax
ENDIF

    ; Allign the fpSaveArea
    lea     ecx, fpSaveArea
    add     ecx, 40H
    and     ecx, 0ffffffc0H
    ; Save the floating-point state
IFDEF CONTEXT_USING_AVX
    push    edx
    xor     edx, edx
    mov     eax, 7
    xsave   [ecx]
ELSE
    fxsave  [ecx]
ENDIF

    ; Now call ChangeRegs - do the actual test.
    ; The tool may intercept this function and modify the register values itself.
    call    ChangeRegs

    ; Placeholder for PIN_ExecuteAt
    call    ExecuteAt

    ; Save the modified values to memory so the tool can ispect them.
    ; This is relevant only when the tool modifies the values.
    call    SaveRegsToMem

    ; Restore the floating-point state
IFDEF CONTEXT_USING_AVX
    mov     eax, 7
    xrstor  [ecx]
    pop     edx
ELSE
    fxrstor [ecx]
ENDIF

IFDEF CONTEXT_USING_AVX512F
    ; Restore the mask registers
    pop     eax
    kmovw   k3, eax
ENDIF

    ; Restore the GPRs
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret
ChangeRegsWrapper ENDP

; void ChangeRegs();
; For register usage see ChangeRegsWrapper above.
ChangeRegs PROC
    ; TEST: load the new value to ebx
    mov     ebx, gprval
    ; prepare the test value at the top of the FPU stack
    fld     real10 ptr stval
    ; TEST: load the new value to st2
    fst     st(2)
    ; TEST: load the new value to xmm0
    movdqu  xmm0, xmmword ptr xmmval
IFDEF CONTEXT_USING_AVX
    ; TEST: load the new value to ymm1
    vmovdqu ymm1, ymmword ptr ymmval
ENDIF
IFDEF CONTEXT_USING_AVX512F
    ; TEST: load the new value to zmm5
    vmovdqu32 zmm5, zmmword ptr zmmval
    ; TEST: load the new value to k3
    kmovw   k3, opmaskval
ENDIF
    ret
ChangeRegs ENDP

; void ExecuteAt();
ExecuteAt PROC
    ret
ExecuteAt ENDP

; void SaveRegsToMem();
; Save the necessary registers to memory.
; The tool will then compare the value stored in memory to the ones it expects to find.
; For register usage see ChangeRegsWrapper above.
SaveRegsToMem PROC
    ; TEST: store the new value of ebx
    mov     agprval, ebx
    ; prepare the test value at the top of the FPU stack
    fld     st(2)
    ; TEST: store the new value of st2
    fstp    real10 ptr astval
    ; TEST: store the new value of xmm0
    movdqu  xmmword ptr axmmval, xmm0
IFDEF CONTEXT_USING_AVX
    ; TEST: store the new value of ymm1
    vmovdqu ymmword ptr aymmval, ymm1
ENDIF
IFDEF CONTEXT_USING_AVX512F
    ; TEST: store the new value of zmm5
    vmovdqu32 zmmword ptr azmmval, zmm5
    ; TEST: store the new value of k3
    kmovw   aopmaskval, k3
ENDIF
    ret
SaveRegsToMem ENDP

end
