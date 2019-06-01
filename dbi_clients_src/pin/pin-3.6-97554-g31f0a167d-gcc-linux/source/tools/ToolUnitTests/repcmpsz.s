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
/*! @file
 *  This file contains the assembly source of Pin unit test repcmpsz_tool
 */

        .data
one:
	.string	"IAMHEREE"
	strlen = . - one
two:	
	.string	"IWASHERE"

        .text
.globl _start

#ifdef TARGET_LINUX
.type _start, @function
#endif

_start:
	fnop
	cld
        xor     %ebx, %ebx                      # %ebx holds test number (used as exit code on failure)
        
# Test different string comparison
        inc     %ebx
	lea	one, %esi
	lea	two, %edi
	mov     $strlen,%ecx
	repe cmpsb
        cmp     $(strlen-2),%ecx                # Should fail at second byte
        jne     2f

# Test same string comparison
        inc     %ebx
	lea	one, %esi
	lea	one, %edi
	mov     $strlen,%ecx
	repe cmpsb 
        test    %ecx,%ecx                       # Should run full length
        jne     2f

# Test same string comparison, but with no count...
        inc     %ebx
	lea	one, %esi
	lea	one, %edi
        xor     %ecx,%ecx
	repe cmpsb 
        test    %ecx,%ecx                       # Should still be zero
        jne     2f

# Test scasd
        inc     %ebx
	movl	one, %eax
	lea	two, %edi
	scasw
	mov     %eax,%ecx

# and exit

	movl	$0,%ebx		# Linux first argument: exit code
2:  
	push    %ebx		# OS X expects args on the stack
	push    %ebx
	movl	$0x1,%eax	# system call number (sys_exit)
	int	$0x80		# call kernel
	fnop

