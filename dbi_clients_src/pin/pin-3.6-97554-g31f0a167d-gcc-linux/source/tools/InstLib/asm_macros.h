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
/*
 * This header file consolidate definitions for OS X* and Linux
 * running on both ia32 and intel64 architecture.
 */

/*-------------------------------------------------------------------
 * OS specific macros:
 *-------------------------------------------------------------------
 * NAME(symbol)                - Decorate 'symbol' as a global symbol
 * DECLARE_FUNCTION(symbol)    - Declare 'symbol' as function type
 * DECLARE_FUNCTION_AS(symbol) - Declare 'symbol' as function type without decorations
 * END_FUNCTION(symbol)        - Mark the end of the function 'symbol'
 * PLT_ADDRESS(symbol)         - call target for 'symbol' to make a call to external function.
 *-------------------------------------------------------------------
 */
#ifdef TARGET_MAC
# define NAME(x) _##x
# define DECLARE_FUNCTION(fun) .global NAME(fun);
# define DECLARE_FUNCTION_AS(fun) .global fun;
# define END_FUNCTION(fun)     .global NAME(fun ## _endfunc); \
                               NAME(fun ## _endfunc):
# define PLT_ADDRESS(fun)      NAME(fun)
#else
# define NAME(x) x
# define DECLARE_FUNCTION(fun) .globl NAME(fun); \
                               .type NAME(fun),  @function
# define DECLARE_FUNCTION_AS(fun) DECLARE_FUNCTION(fun)
# define END_FUNCTION(fun)     .size NAME(fun),.-NAME(fun)
# define PLT_ADDRESS(fun)      NAME(fun) ## @plt
#endif

/*-------------------------------------------------------------------
 * Architecture specific macros:
 *-------------------------------------------------------------------
 * BEGIN_STACK_FRAME         - Expands to the instructions that build a new stack
 *                             frame for a function
 * END_STACK_FRAME           - Expands to the instructions that destroy a stack
 *                             frame just before calling "ret"
 * PARAM1                    - The first argument to a function.
 *                             Note that this macro is valid only after building a
 *                             stack frame
 * PARAM2                    - The second argument to a function.
 *                             Note that this macro is valid only after building a
 *                             stack frame
 * SCRATCH_REG*              - Taken from Calling Convention:
 *                             Scratch register are registers that can be used for temporary storage without
 *                             restrictions (no need to save before using them and restore after using them)
 * SCRATCH_REG1              - Scratch register eax/rax depending on the architecture
 * SCRATCH_REG2              - Scratch register ecx/rcx depending on the architecture
 * SCRATCH_REG2              - Scratch register edx/rdx depending on the architecture
 * RETURN_REG                - The register that holds the return value
 * GAX_REG                   - eax/rax depending on the architecture
 * GBX_REG                   - ebx/rbx depending on the architecture
 * GCX_REG                   - ecx/rcx depending on the architecture
 * GDX_REG                   - edx/rdx depending on the architecture
 * STACK_PTR                 - The stack pointer register
 * PIC_VAR(v)                - Reference memory at 'v' in PIC notation (not supported in 32 bit mode)
 * SYSCALL_PARAM1            - The first argument to a system call
 *                             Note that this macro is valid only after building a
 *                             stack frame
 * PREPARE_UNIX_SYSCALL(num) - Prepare to run a syscall numbered 'num'
 *                             Assign the syscall number (plus some required
 *                             transformation to the register that holds the
 *                             syscall number).
 * INVOKE_SYSCALL            - The instruction sequence that does the actual invocation of a syscall.
 *-------------------------------------------------------------------
 */
#if defined(TARGET_IA32)
# define BEGIN_STACK_FRAME \
             push %ebp; \
             mov %esp, %ebp
# define END_STACK_FRAME \
             mov %ebp, %esp; \
             pop %ebp
# define PARAM1 8(%ebp)
# define PARAM2 0xc(%ebp)
# define RETURN_REG %eax
# define GAX_REG %eax
# define GBX_REG %ebx
# define GCX_REG %ecx
# define CL_REG  %cl
# define GDX_REG %edx
# define STACK_PTR %esp
# define PIC_VAR(a) a
# ifdef TARGET_MAC
#  define SYSCALL_PARAM1 (%esp)
#  define PREPARE_UNIX_SYSCALL(num) mov num, %eax; \
                                    or $0x40000, %eax
#  define INVOKE_SYSCALL calll LTrap ## __LINE__; \
                         jmpl LTrapDone ## __LINE__ ; \
                         LTrap ## __LINE__: \
                         popl %edx; \
                         movl %esp, %ecx; \
                         sysenter; \
                         LTrapDone ## __LINE__:
# else
#  define PREPARE_UNIX_SYSCALL(num) mov num, %eax
#  define SYSCALL_PARAM1 %ebx
#  define INVOKE_SYSCALL int $0x80
# endif
#elif defined(TARGET_IA32E)
# define BEGIN_STACK_FRAME \
             push %rbp; \
             mov %rsp, %rbp
# define END_STACK_FRAME \
             mov %rbp, %rsp; \
             pop %rbp
# define PARAM1 %rdi
# define PARAM2 %rsi
# define RETURN_REG %rax
# define GAX_REG %rax
# define GBX_REG %rbx
# define GCX_REG %rcx
# define CL_REG  %cl
# define GDX_REG %rdx
# define STACK_PTR %rsp
# define PIC_VAR(a) a(%rip)
# define SYSCALL_PARAM1 %rdi
# define INVOKE_SYSCALL syscall
# ifdef TARGET_MAC
#  define PREPARE_UNIX_SYSCALL(num) mov num, %rax; \
                                    or $0x2000000, %rax
# else
#  define PREPARE_UNIX_SYSCALL(num) mov num, %rax
# endif
#endif

/*
 * Common
 */
#define SCRATCH_REG1 GAX_REG
#define SCRATCH_REG2 GCX_REG
#define SCRATCH_REG3 GDX_REG
