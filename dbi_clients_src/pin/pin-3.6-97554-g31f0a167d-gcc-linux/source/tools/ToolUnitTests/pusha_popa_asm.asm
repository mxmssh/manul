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
.model flat, c
.code
DoPushA16 PROC
    push    ebp
    push    ebx
    push    edi
    push    esi

    mov     eax, [esp + 28]       ; third parameter (&inSp)
    mov     [eax], sp             ; save SP before pusha

    mov     edi, [esp + 20]       ; first parameter (&inRegs)
    mov     ax, [edi]             ; ax = inRegs->ax
    mov     cx, [edi + 2]         ; cx = inRegs->cx
    mov     dx, [edi + 4]         ; dx = inRegs->dx
    mov     bx, [edi + 6]         ; bx = inRegs->bx
                                  ; (skip sp value)
    mov     bp, [edi + 10]        ; bp = inRegs->bp
    mov     si, [edi + 12]        ; si = inRegs->si
    mov     di, [edi + 14]        ; di = inRegs->di

    pushaw

    mov     edi, [esp + (16+24)]  ; second parameter (&outRegs)
    mov     ax, [esp + 14]        ; outRegs->ax = push'd ax value
    mov     [edi], ax
    mov     ax, [esp + 12]        ; outRegs->cx = push'd cx value
    mov     [edi + 2], ax
    mov     ax, [esp + 10]        ; outRegs->dx = push'd dx value
    mov     [edi + 4], ax
    mov     ax, [esp + 8]         ; outRegs->bx = push'd bx value
    mov     [edi + 6], ax
    mov     ax, [esp + 6]         ; outRegs->sp = push'd sp value
    mov     [edi + 8], ax
    mov     ax, [esp + 4]         ; outRegs->bp = push'd bp value
    mov     [edi + 10], ax
    mov     ax, [esp + 2]         ; outRegs->si = push'd si value
    mov     [edi + 12], ax
    mov     ax, [esp]             ; outRegs->di = push'd di value
    mov     [edi + 14], ax

    lea     esp, [esp + 16]

    pop     esi
    pop     edi
    pop     ebx
    pop     ebp
	ret
DoPushA16 ENDP


.code
DoPushA32 PROC
	push	ebp
	push	ebx
	push	edi
	push	esi

    mov     eax, [esp + 28]       ; third parameter (&inSp)
    mov     [eax], esp            ; save SP before pusha

    mov     edi, [esp + 20]       ; first parameter (&inRegs)
    mov     eax, [edi]            ; eax = inRegs->eax
    mov     ecx, [edi + 4]        ; ecx = inRegs->ecx
    mov     edx, [edi + 8]        ; edx = inRegs->edx
    mov     ebx, [edi + 12]       ; ebx = inRegs->ebx
                                  ; (skip esp value)
    mov     ebp, [edi + 20]       ; ebp = inRegs->ebp
    mov     esi, [edi + 24]       ; esi = inRegs->esi
    mov     edi, [edi + 28]       ; edi = inRegs->edi

    pusha

    mov     edi, [esp + (32+24)]  ; second parameter (&outRegs)
    mov     eax, [esp + 28]       ; outRegs->eax = push'd eax value
    mov     [edi], eax
    mov     eax, [esp + 24]       ; outRegs->ecx = push'd ecx value
    mov     [edi + 4], eax
    mov     eax, [esp + 20]       ; outRegs->edx = push'd edx value
    mov     [edi + 8], eax
    mov     eax, [esp + 16]       ; outRegs->ebx = push'd ebx value
    mov     [edi + 12], eax
    mov     eax, [esp + 12]       ; outRegs->esp = push'd esp value
    mov     [edi + 16], eax
    mov     eax, [esp + 8]        ; outRegs->ebp = push'd ebp value
    mov     [edi + 20], eax
    mov     eax, [esp + 4]        ; outRegs->esi = push'd esi value
    mov     [edi + 24], eax
    mov     eax, [esp]            ; outRegs->edi = push'd edi value
    mov     [edi + 28], eax

    lea     esp, [esp + 32]

    pop     esi
    pop     edi
    pop     ebx
    pop     ebp
	ret
DoPushA32 ENDP


.code
DoPopA16 PROC
	push	ebp
	push	ebx
	push	edi
	push	esi

    lea     esp, [esp - 16]       ; reserve stack space for register block that will be pop'd

    mov     eax, [esp + (16+20)]  ; first parameter (&inRegs)
    mov     bx, [eax]             ; copy inRegs->ax to stack
    mov     [esp + 14], bx
    mov     bx, [eax + 2]         ; copy inRegs->cx to stack
    mov     [esp + 12], bx
    mov     bx, [eax + 4]         ; copy inRegs->dx to stack
    mov     [esp + 10], bx
    mov     bx, [eax + 6]         ; copy inRegs->bx to stack
    mov     [esp + 8], bx
    mov     bx, [eax + 8]         ; copy inRegs->sp to stack
    mov     [esp + 6], bx
    mov     bx, [eax + 10]        ; copy inRegs->bp to stack
    mov     [esp + 4], bx
    mov     bx, [eax + 12]        ; copy inRegs->si to stack
    mov     [esp + 2], bx
    mov     bx, [eax + 14]        ; copy inRegs->di to stack
    mov     [esp], bx

    popaw

    push    eax                   ; temporarily save pop'd AX value
    mov     eax, [esp + (4+24)]   ; second parameter (&outRegs)
    mov     [eax + 2], cx         ; outRegs->cx = pop'd cx value
    mov     [eax + 4], dx         ; outRegs->dx = pop'd dx value
    mov     [eax + 6], bx         ; outRegs->bx = pop'd bx value
                                  ; (skip pop'd sp value)
    mov     [eax + 10], bp        ; outRegs->bp = pop'd bp value
    mov     [eax + 12], si        ; outRegs->si = pop'd si value
    mov     [eax + 14], di        ; outRegs->di = pop'd di value
    pop     ebx
    mov     [eax], bx             ; outRegs->ax = pop'd ax value

    pop     esi
    pop     edi
    pop     ebx
    pop     ebp
	ret
DoPopA16 ENDP


.code
DoPopA32 PROC
	push	ebp
	push	ebx
	push	edi
	push	esi

    lea     esp, [esp - 32]       ; reserve stack space for register block that will be pop'd

    mov     eax, [esp + (32+20)]  ; first parameter (&inRegs)
    mov     ebx, [eax]            ; copy inRegs->eax to stack
    mov     [esp + 28], ebx
    mov     ebx, [eax + 4]        ; copy inRegs->ecx to stack
    mov     [esp + 24], ebx
    mov     ebx, [eax + 8]        ; copy inRegs->edx to stack
    mov     [esp + 20], ebx
    mov     ebx, [eax + 12]       ; copy inRegs->ebx to stack
    mov     [esp + 16], ebx
    mov     ebx, [eax + 16]       ; copy inRegs->esp to stack
    mov     [esp + 12], ebx
    mov     ebx, [eax + 20]       ; copy inRegs->ebp to stack
    mov     [esp + 8], ebx
    mov     ebx, [eax + 24]       ; copy inRegs->esi to stack
    mov     [esp + 4], ebx
    mov     ebx, [eax + 28]       ; copy inRegs->edi to stack
    mov     [esp], ebx

    popa

    push    eax                   ; temporarily save pop'd AX value
    mov     eax, [esp + (4+24)]   ; second parameter (&outRegs)
    mov     [eax + 4], ecx        ; outRegs->ecx = pop'd ecx value
    mov     [eax + 8], edx        ; outRegs->edx = pop'd edx value
    mov     [eax + 12], ebx       ; outRegs->ebx = pop'd ebx value
                                  ; (skip pop'd esp value)
    mov     [eax + 20], ebp       ; outRegs->ebp = pop'd ebp value
    mov     [eax + 24], esi       ; outRegs->esi = pop'd esi value
    mov     [eax + 28], edi       ; outRegs->edi = pop'd edi value
    pop     ebx
    mov     [eax], ebx            ; outRegs->eax = pop'd eax value

    pop     esi
    pop     edi
    pop     ebx
    pop     ebp
	ret
DoPopA32 ENDP

END
