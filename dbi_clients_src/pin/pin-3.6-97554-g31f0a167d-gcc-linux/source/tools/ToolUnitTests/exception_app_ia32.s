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

## unsigned int RaiseIntDivideByZeroException(unsigned int (*)(), unsigned int);

.global RaiseIntDivideByZeroException

.type RaiseIntDivideByZeroException, @function
RaiseIntDivideByZeroException:
    push %ebp
    mov %esp, %ebp
    push %ebx # save ebx
    mov 0x8(%ebp), %ebx # fptr
    mov 0xc(%ebp), %esi # except code
    push %esi
    xor %eax, %eax
    idiv %eax
    leave
    ret
    
.global CatchIntDivideByZeroException

.type CatchIntDivideByZeroException, @function
CatchIntDivideByZeroException:
    pop %eax # exc code -> %eax
    pop %ebx
    leave
    ret
    
.global UnmaskFpZeroDivide
.type UnmaskFpZeroDivide, @function
UnmaskFpZeroDivide:
  push %ebp
  mov %esp, %ebp
  call GetFCW
  mov $0x4, %edi
  not %edi
  and %edi, %eax
  push %eax
  call SetFCW
  leave
  ret
  
.global MaskFpZeroDivide
.type MaskFpZeroDivide, @function
MaskFpZeroDivide:
  push %ebp
  mov %esp, %ebp
  call GetFCW
  mov $0x4, %edi
  or %edi, %eax
  push %eax
  call SetFCW
  fnclex
  leave
  ret

.global GetFCW
.type GetFCW, @function
GetFCW:
 xor %eax, %eax
 push %eax
 fstcw (%esp)
 pop %eax
 ret


.global SetFCW
.type SetFCW, @function
SetFCW:
   push %ebp
   mov %esp, %ebp
   fnclex
   fldcw 8(%ebp)
   fnclex
   leave
   ret

.global UnmaskZeroDivideInMxcsr32
.type UnmaskZeroDivideInMxcsr32, @function
UnmaskZeroDivideInMxcsr32:
   push %ebp
   mov %esp, %ebp
   call GetMxcsr32
   mov $0x200, %edi
   not %edi
   and %eax, %edi
   push %edi
   call SetMxcsr32
   leave
   ret

.global MaskZeroDivideInMxcsr32
.type MaskZeroDivideInMxcsr32, @function
MaskZeroDivideInMxcsr32:
   push %ebp
   mov %esp, %ebp
   call GetMxcsr32
   mov $0x200, %edi
   or %eax, %edi
   push %edi
   call SetMxcsr32
   leave
   ret

.global GetMxcsr32
.type GetMxcsr32, @function
GetMxcsr32:
   push %ebp
   mov %esp, %ebp
   push %eax
   stmxcsr (%esp)
   pop %eax
   leave
   ret
  
.global SetMxcsr32
.type SetMxcsr32, @function
SetMxcsr32:
   push %ebp
   mov %esp, %ebp
   mov 0x8(%ebp), %edi
   push %edi
   ldmxcsr (%esp)
   pop %eax
   leave
   ret
