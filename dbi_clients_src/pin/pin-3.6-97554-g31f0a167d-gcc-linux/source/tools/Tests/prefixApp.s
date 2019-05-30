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
.intel_syntax noprefix
.globl main
.type	main, function
main:
	call test1
	call test2
	mov eax, 0
	ret

.globl test1
.type   test1, function
test1:
	xor eax, eax
	cmp eax, 0
	je bye
	# the following code will never executed

	# address size & segment prefixes
	addr16 mov fs:5, eax

	# rep prefix
	mov esi, mystr1
	mov edi, mystr2
	rep movsb

.globl test2
.type   test2, function
test2:
	xor eax, eax
	cmp eax, 0
	je bye
	# the following code will never executed

	# lock prefix
	lock add var, ecx

	# operand size & repne prefixes
	mov eax, 8
	mov edi, mystr3
	repne scasw

bye:
	ret

.data
var: .long 10
mystr1: .ascii "hello1"
mystr2: .ascii "hello2"
mystr3: .ascii "hello3"
