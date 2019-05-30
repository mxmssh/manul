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
.globl main
.type main, function
main:	
	push %ecx
	push %esi
	push %edi
	
	fnop

        # copy instring to outstring, counting up
	mov $1, %edx
	cld
	movl $2, %ecx
	mov $instring, %esi
	mov $outstring, %edi
	rep movsb
	
	cld
	mov $2, %edx
	movl $2, %ecx
	mov $instring, %esi
	mov $outstring, %edi
	rep movsw
	
	cld
	mov $3, %edx
	movl $2, %ecx
	mov $instring, %esi
	mov $outstring, %edi
	rep movsl
	
	# copy instring to outstring, counting down
	mov $5, %edx
	std
	movl $2, %ecx
	mov $instring+16-1, %esi
	mov $outstring+16-1, %edi
	rep movsb
	
	mov $6, %edx
	std
	movl $2, %ecx
	mov $instring+16-2, %esi
	mov $outstring+16-2, %edi
	rep movsw
	
	mov $7, %edx
	std
	movl $2, %ecx
	mov $instring+16-4, %esi
	mov $outstring+16-4, %edi
	rep movsl
	
	
	# store 'ab' 3 times, moving forward
	mov $9, %edx
	cld
	movl $0x6261, %eax
	movl $3, %ecx
	mov  $outstring, %edi
	rep stosw
	
	# store 'cd' 3 times, moving backwards
	mov $10, %edx
	std
	movl $0x6463, %eax
	movl $3, %ecx
	mov  $outstring + 16 - 2, %edi
	rep stosw
	
	# load 2 times, moving backwards
	mov $11, %edx
	std
	movl $2, %ecx
	mov  $instring + 16 - 4, %esi
	rep lodsl
	
	# Find 'ab' in instring
	mov $12, %edx
	cld
	movl $0x6261, %eax
	movl $16, %ecx
	mov  $instring, %edi
	repne scasw

	# Find first bytes not '01' in instring
	mov $13, %edx
	cld
	movl $0x3130, %eax
	movl $16, %ecx
	mov  $instring, %edi
	repe scasw

	# Find first mismatch in instring1 and instring2
	mov $14, %edx
	cld
	movl $16, %ecx
	mov  $instring, %esi
	mov  $instring2, %edi
	repe cmpsb

	# Find first match in instring1 and instring2
	mov $15, %edx
	cld
	movl $16, %ecx
	mov  $instring, %esi
	mov  $instring2, %edi
	repne cmpsb

	# A test with a zero count
	mov $16, %edx
	cld
	movl $0, %ecx
	mov  $instring, %esi
	mov  $instring2, %edi
	repne cmpsb

	mov $0, %eax
	
	pop %edi
	pop %esi
	pop %ecx
	ret
	
.data
instring:		
.ascii 	"0123456789abcdef"
instring2:
.ascii 	"0123456x89abcdef"
outstring:	
.ascii 	"0123456789abcdef"
