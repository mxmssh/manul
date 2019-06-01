/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 * CopyWithXmm(char *dest, const char *src, size_t size)
 *
 *  Copy a memory block using XMM registers.  Assumes that
 *  'dest' and 'src' are 16-byte aligned and 'size' is a
 *  multiple of 16 bytes.
 */
.text
	.align 4
#if defined(TARGET_MAC)
.globl _CopyWithXmm
_CopyWithXmm:
#else
.globl CopyWithXmm
CopyWithXmm:
#endif
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx

	movl	8(%ebp), %esi
	movl	12(%ebp), %ebx
	movl	16(%ebp), %ecx
    shr     $0x5, %ecx

.L1:
    movdqa  (%ebx), %xmm0
    movdqa  0x10(%ebx), %xmm1
    movdqa  %xmm0, (%esi)
    movdqa  %xmm1, 0x10(%esi)
    lea     0x20(%ebx), %ebx
    lea     0x20(%esi), %esi
    dec     %ecx
    jne     .L1

	popl	%ebx
	popl	%esi
	popl	%ebp
	ret


/*
 * CopyWithXmmDelay(char *dest, const char *src, size_t size)
 *
 *  This is exactly the same as CopyWithXmm() except that has
 *  an internal delay that makes it more likely for a signal
 *  to interrupt the copy during a critical point.
 */
.text
	.align 4
#if defined(TARGET_MAC)
.globl _CopyWithXmmDelay
_CopyWithXmmDelay:
#else
.globl CopyWithXmmDelay
CopyWithXmmDelay:
#endif
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx

	movl	8(%ebp), %esi
	movl	12(%ebp), %ebx
	movl	16(%ebp), %ecx
    shr     $0x5, %ecx

.L2:
    movdqa  (%ebx), %xmm0
    movdqa  0x10(%ebx), %xmm1

    /*
     * Delay for a while.  The values in the XMM registers are live
     * at this point, so this delay loop opens up a timing window that
     * makes it more likely for a signal to interrupt the copy when
     * the XMM registers are being used.
     */
    mov     $0x1000, %eax
.L3:
    dec     %eax
    jne     .L3

    movdqa  %xmm0, (%esi)
    movdqa  %xmm1, 0x10(%esi)
    lea     0x20(%ebx), %ebx
    lea     0x20(%esi), %esi
    dec     %ecx
    jne     .L2

	popl	%ebx
	popl	%esi
	popl	%ebp
	ret


/*
 * CopyWithXmmFault(char *dest, const char *src, size_t size)
 *
 *  This is exactly the same as CopyWithXmm() except that there is
 *  an illegal memory access to address 0x0 at a critical point.
 *  The caller is expected to set up a SIGSEGV handler to catch the
 *  illegal access.  The handler must change the value of %eax to
 *  a valid memory location.
 */
.text
	.align 4
#if defined(TARGET_MAC)
.globl _CopyWithXmmFault
_CopyWithXmmFault:
#else
.globl CopyWithXmmFault
CopyWithXmmFault:
#endif
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx

	movl	8(%ebp), %esi
	movl	12(%ebp), %ebx
	movl	16(%ebp), %ecx
    shr     $0x5, %ecx

.L4:
    movdqa  (%ebx), %xmm0
    movdqa  0x10(%ebx), %xmm1

    /*
     * This causes a SIGSEGV.  The caller must fix %eax in a handler.
     */
    movl    $0x0, %eax
    movl    (%eax), %eax

    movdqa  %xmm0, (%esi)
    movdqa  %xmm1, 0x10(%esi)
    lea     0x20(%ebx), %ebx
    lea     0x20(%esi), %esi
    dec     %ecx
    jne     .L4

	popl	%ebx
	popl	%esi
	popl	%ebp
	ret
