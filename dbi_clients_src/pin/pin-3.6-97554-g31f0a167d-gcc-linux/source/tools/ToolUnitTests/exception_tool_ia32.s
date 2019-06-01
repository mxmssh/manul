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
.global ToolRaiseAccessInvalidAddressException
.type ToolRaiseAccessInvalidAddressException, @function
.global ToolCatchAccessInvalidAddressException
.type ToolCatchAccessInvalidAddressException, @function
.global ToolIpAccessInvalidAddressException
.type ToolIpAccessInvalidAddressException, @function

ToolRaiseAccessInvalidAddressException:
    push %ebp
    mov %esp, %ebp
    push %esi
    push %edi
    mov 0x8(%ebp), %eax # addresses array
    mov 0xc(%ebp), %esi # value
    mov (%eax), %edi
try_again:
ToolIpAccessInvalidAddressException:
    mov %esi, (%edi) # *addr = value - if addr is invalid, exception is raised
    pop %edi
    pop %esi
    leave
    ret

ToolCatchAccessInvalidAddressException:
    mov 4(%eax), %edi
    jmp try_again
    

.text
# void ToolRaiseIntDivideByZeroException(catch_ptr, exception_code)

.global ToolRaiseIntDivideByZeroException
.global ToolIpIntDivideByZeroException
.type ToolRaiseIntDivideByZeroException, @function
.global ToolCatchIntDivideByZeroException

ToolRaiseIntDivideByZeroException:
    push %ebp
    mov %esp, %ebp
    push %ebx # save ebx
    push %esi #save esi
    mov 0x8(%ebp), %ebx # fptr
    mov 0xc(%ebp), %esi # except code
    push %esi
    xor %eax, %eax
ToolIpIntDivideByZeroException:
    idiv %eax
ToolCatchIntDivideByZeroException:
    pop %eax
    pop %esi
    pop %ebx
    leave
    ret
    
    
