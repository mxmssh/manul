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
# 
#  struct FarPointer
#  {
#      unsigned int _farPtr;
#      unsigned int _segVal;
#  };

#  int SetGs(const FarPointer *fp);


.global SetGs
.type SetGs, @function


SetGs:
   push %ebp
   mov %esp, %ebp
   mov 0x8(%ebp), %eax
   lgs (%eax), %eax
   leave
   ret

#  int SetFs(const FarPointer *fp);

.global SetFs
.type SetFs, @function

SetFs:
   push %ebp
   mov %esp, %ebp
   mov 0x8(%ebp), %eax
   lfs (%eax), %eax
   leave
   ret
   
#  unsigned int GetGsBase();
#  unsigned int GetFsBase();

.global GetGsBase
.type GetGsBase, @function


GetGsBase:
   mov %gs:0x0, %eax
   ret
   
.global GetFsBase
.type GetFsBase, @function

GetFsBase:
   mov %fs:0x0, %eax
   ret
   

