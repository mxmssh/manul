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
# On linux 64 
# % gcc -nostdlib -o reptest reptest.S
# % pinbin -- reptest
#	
        .data
one:
	.string	"IAMHEREE"
	strlen = . - one
two:	
	.string	"IWASHERE"

        .text
.globl _start

_start:
	fnop
	cld
        xor     %rbx, %rbx                      # %rbx holds test number (used as exit code on failure)
        
# Test different string comparison
        inc     %rbx
	lea	one(%rip), %rsi
	lea	two(%rip), %rdi
	mov     $strlen,%rcx
	repe cmpsb
        cmp     $(strlen-2),%rcx                # Should fail at second byte
        jne     2f

# Test same string comparison
        inc     %rbx
	lea	one(%rip), %rsi
	lea	one(%rip), %rdi
	mov     $strlen,%rcx
	repe cmpsb 
        test    %rcx,%rcx                       # Should run full length
        jne     2f

# Test same string comparison, but with no count...
        inc     %rbx
	lea	one(%rip), %rsi
	lea	one(%rip), %rdi
        xor     %rcx,%rcx
	repe cmpsb 
        test    %rcx,%rcx                       # Should still be zero
        jne     2f

# Test scasd
        inc     %rbx
	mov	one(%rip), %rax
	lea	two(%rip), %rdi
	scasw
	mov     %rax,%rcx
	
# Test same string comparison, but with no count...
        inc     %rbx
        lea     one(%rip), %rsi
        lea     one(%rip), %rdi
        xor     %rcx,%rcx
        repe cmpsb
        test    %rcx,%rcx                       # Should still be zero
        jne     2f

# Test scasd
        inc     %rbx
        mov     one(%rip), %rax
        lea     two(%rip), %rdi
        scasw
        mov     %rax,%rcx

# Test a couple of zero sized operations
#
        inc     %rbx
        xor     %rcx,%rcx
        mov     %rcx,%rsi
        mov     %rcx,%rdi
        rep movsq

        inc     %rbx
        rep scasq

        inc     %rbx
        rep stosq

# and exit

        mov     $0,%rbx         # first argument: exit code
2:
        mov     $1,%rax         # system call number (sys_exit)
#if (__FreeBSD__)
        mov     %rbx,%rdi
        syscall
#elif (__linux__)
        int     $0x80
#else
# error Unknown target OS        
#endif       
        fnop
        
