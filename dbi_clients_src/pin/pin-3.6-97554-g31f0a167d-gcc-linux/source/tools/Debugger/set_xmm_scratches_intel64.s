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




.text
.globl SetXmmScratchesFun 
SetXmmScratchesFun: 
    movdqu   (%rdi), %xmm0
    movdqu   16(%rdi), %xmm1
    movdqu   32(%rdi), %xmm2
    movdqu   48(%rdi), %xmm3
    movdqu   64(%rdi), %xmm4
    movdqu   80(%rdi), %xmm5
    movdqu   96(%rdi), %xmm6
    movdqu   112(%rdi), %xmm7
    movdqu   128(%rdi), %xmm8
    movdqu   144(%rdi), %xmm9
    movdqu   160(%rdi), %xmm10
    movdqu   176(%rdi), %xmm11
    movdqu   192(%rdi), %xmm12
    movdqu   208(%rdi), %xmm13
    movdqu   224(%rdi), %xmm14
    movdqu   240(%rdi), %xmm15
    
    ret

