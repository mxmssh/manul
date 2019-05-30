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
#include <asm_macros.h>

DECLARE_FUNCTION(bsr_func)
DECLARE_FUNCTION(bsf_func)

.global NAME(bsr_func)
.global NAME(bsf_func)
//.intel_syntax noprefix 

// int bsr_func(int src) - this function returns the bit index of the 
// most significant bit set. in case 'src' is zero, -1 is returned.
NAME(bsr_func):
   BEGIN_STACK_FRAME
   mov $0, RETURN_REG
   not RETURN_REG
   bsr PARAM1, RETURN_REG
   END_STACK_FRAME
   ret

// int bsr_func(int src) - this function returns the bit index of the 
// least significant bit set. in case 'src' is zero, -1 is returned.
NAME(bsf_func):
   BEGIN_STACK_FRAME
   mov $0, RETURN_REG
   not RETURN_REG
   bsf PARAM1, RETURN_REG
   END_STACK_FRAME
   ret

