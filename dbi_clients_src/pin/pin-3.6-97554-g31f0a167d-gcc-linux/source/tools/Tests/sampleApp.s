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
	.text
# RTN of size 200000 tickled a bug in symbol handling	
.globl big
	.type	big, @function
big:
	.space 200000
	
.globl main
	.type	main, @function
main:
	call xlat
	mov ecx, 16
loop1:
	nop
	loop  loop1

	mov ecx, 16
	mov eax, 0
	cmp eax,0
loop2:
	nop
	loope  loop2

	mov ecx, 16
	mov eax, 1
	cmp eax, 0	
loop3:	
	nop
	loopne  loop3

	mov eax, 0
	ret

xlat:
#ifdef TARGET_IA32
	movb [table+0],0
	movb [table+1],1
	movb [table+2],2
	movb [table+256+0],7
	movb [table+256+1],8
	movb [table+256+2],9
	lea ebx,[table]
#else
	movb [rip+table+0],0
	movb [rip+table+1],1
	movb [rip+table+2],2
	movb [rip+table+256+0],7
	movb [rip+table+256+1],8
	movb [rip+table+256+2],9
	lea rbx,[rip+table]
#endif
	mov eax,256+1
	xlat
	ret
	
.data
table:
	.space 512
	
	
	
		
