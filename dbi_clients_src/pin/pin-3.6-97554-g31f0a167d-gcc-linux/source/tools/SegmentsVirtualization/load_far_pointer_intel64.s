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
# struct FarPointer16
# {
#     unsigned short _farPtr;
#     unsigned short _segVal;
#     unsigned int   _pad;
# };
#
# struct FarPointer32
# {
#     unsigned int _farPtr;
#     unsigned int _segVal;
#     unsigned int   _pad;
# };
# struct FarPointer64
# {
#     unsigned long _farPtr;
# 	  unsigned long _segVal;
# 	  unsigned long _pad;
# };
# 

# unsigned short SetGs16(const FarPointer16 *fp, unsigned long upperBits);
# unsigned int   SetGs32(const FarPointer32 *fp, unsigned long upperBits);
# unsigned long  SetGs64(const FarPointer64 *fp, unsigned long upperBits);


.global SetGs16
.type SetGs16, @function


SetGs16:
   mov %rsi, %rax
   lgs (%rdi), %ax
   ret

.global SetGs32
.type SetGs32, @function


SetGs32:
   mov %rsi, %rax
   lgs (%rdi), %eax
   ret

.global SetGs64
.type SetGs64, @function


SetGs64:
   mov %rsi, %rax
   lgs (%rdi), %rax
   ret

.global GetGsVal
.type GetGsVal, @function

GetGsVal:
   mov $0xffffffff88888888, %rax
   mov %gs, %eax
   ret

# void MoveMem16ToGs(unsigned int  *val32);
# void MoveMem64ToGs(unsigned long *val64);
# 
# void MoveGsToMem16(unsigned int  *val32);
# void MoveGsToMem64(unsigned long *val64);


.global MoveMem16ToGs
.type MoveMem16ToGs, @function

MoveMem16ToGs:
    movw (%rdi), %gs
    ret
    
.global MoveMem64ToGs
.type MoveMem64ToGs, @function

MoveMem64ToGs:
    mov (%rdi), %gs    
    ret
    
.global MoveGsToMem16
.type MoveGsToMem16, @function

MoveGsToMem16:
    movw %gs, (%rdi)
    ret
    
.global MoveGsToMem64
.type MoveGsToMem64, @function

MoveGsToMem64:
    mov %gs, (%rdi)  
    ret
