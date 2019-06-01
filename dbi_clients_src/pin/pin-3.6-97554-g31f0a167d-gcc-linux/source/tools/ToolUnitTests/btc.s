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

#if (TARGET_IA32)
//
// Use a bit test and complement instruction.
// The fun here is that the memory address includes some bits from the
// bit number, and Pin used to get that wrong.
//
// int btc(char * data, UINT32 bitno)
//        
        .type btc, @function
        .global btc
btc:    
        movl     4(%esp),%ecx
        movl     8(%esp),%edx    
        btc      %edx, (%ecx)
        mov      $0, %eax
        jnc      1f
        mov      $1, %eax
1:      
// Ensure that nothing has broken edx, with memory operand rewriting
// that might happen, since Pin has to mask the value down...
        cmp     8(%esp),%edx
        je      2f
        or      $2,%eax         // Flag a problem!
2:      
        ret

// int btr(char * data, UINT32 bitno)
//        
        .type btr, @function
        .global btr
btr:    
        movl     4(%esp),%ecx
        movl     8(%esp),%eax
        // To give btr a more complicated address mode, we offset by
        // -4 and then add +4 back in. This tests a slightly different code path in Pin.
        lea      -4(%ecx),%ecx 
        btr      %eax, 4(%ecx)
        mov      $0, %eax
        jnc      1f
        mov      $1, %eax
1:      
        ret

// int bts(char * data, UINT32 bitno)
//        
        .type bts, @function
        .global bts
bts:    
        movl     4(%esp),%ecx
        movl     8(%esp),%eax
        // Try the w suffixed version as well.
        btsw     %ax, (%ecx)
        mov      $0, %eax
        jnc      1f
        mov      $1, %eax
1:      
        ret

// int bt(char * data, UINT32 bitno)
//        
        .type bt , @function
        .global bt 
bt :    
        movl     4(%esp),%ecx
        movl     8(%esp),%eax
        bt       %eax, (%ecx)
        mov      $0, %eax
        jnc      1f
        mov      $1, %eax
1:      
        ret

#else
// 64 bit code.
//
// Use a bit test and complement instruction.
//
// int btc(char * data, UINT32 bitno)
//        
        .type btc, @function
        .global btc
btc:
        mov     %rsi, %rcx
        btc      %rsi, (%rdi)
        mov      $0, %rax
        jnc      1f
        mov      $1, %rax
1:
        cmp     %rcx,%rsi
        je      2f
        or      $2,%rax
2:      
        ret

// int btr(char * data, UINT32 bitno)
//        
        .type btr, @function
        .global btr
btr:    
        // To give btr a more complicated address mode, we offset by
        // -4 and then add +4 back in. This tests a slightly different code path in Pin.
        lea      -4(%rdi),%rdi 
        btr      %rsi, 4(%rdi)
        mov      $0, %rax
        jnc      1f
        mov      $1, %rax
1:      
        ret

// int bts(char * data, UINT32 bitno)
//        
        .type bts, @function
        .global bts
bts:
        mov     %rsi,%rax
        // Try the w suffixed version as well.
        btsw    %ax, (%rdi)
        mov     $0, %rax
        jnc     1f
        mov     $1, %rax
1:      
        ret

// int bt(char * data, UINT32 bitno)
//        
        .type bt , @function
        .global bt 
bt :    
        btl      %esi, (%rdi)
        mov      $0, %rax
        jnc      1f
        mov      $1, %rax
1:      
        ret
#endif
        