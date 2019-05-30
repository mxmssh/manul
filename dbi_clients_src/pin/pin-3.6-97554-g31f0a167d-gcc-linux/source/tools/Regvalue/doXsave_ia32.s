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
#include "asm_macros.h"

.data
.extern xsaveArea
.extern flags

.text

# void DoXsave();
# This function calls xsave and stores the FP state in the given dst area.
# The caller is expected to allocate enough space for the xsave area.
# The function expects the given dst pointer to be properly aligned for the xsave instruction.
DECLARE_FUNCTION_AS(DoXsave)
DoXsave:
    mov     (flags), %eax
    lea     xsaveArea, %ecx
    xor     %edx, %edx

    # Do xsave
    xsave   (%ecx)

    ret

# void DoXsaveOpt();
# This function calls xsaveopt and stores the FP state in the given dst area.
# The caller is expected to allocate enough space for the xsaveopt area.
# The function expects the given dst pointer to be properly aligned for the xsaveopt instruction.
DECLARE_FUNCTION_AS(DoXsaveOpt)
DoXsaveOpt:
    mov     (flags), %eax
    lea     xsaveArea, %ecx
    xor     %edx, %edx

    # Do xsaveopt
    xsaveopt   (%ecx)

    ret

# void DoXrstor();
# This function calls xrstor and restores the specified thetures from the xsave dst area.
# The function expects the given dst pointer to be properly aligned
DECLARE_FUNCTION_AS(DoXrstor)
DoXrstor:
    mov     (flags), %eax
    lea     xsaveArea, %ecx
    xor     %edx, %edx

    # Do xrstor
    xrstor   (%ecx)

    ret

# void DoFxsave();
# This function calls fxsave and stores the legacy FP state in the given dst area.
# The caller is expected to allocate enough space for the fxsave area.
# The function expects the given dst pointer to be properly aligned for the xsave instruction.
DECLARE_FUNCTION_AS(DoFxsave)
DoFxsave:
    lea     xsaveArea, %ecx

    # Do fxsave
    fxsave   (%ecx)

    ret

# void DoFxrstor();
# This function calls fxrstor and restores the legacy FP state fxsave dst area.
# The function expects the given dst pointer to be properly aligned
DECLARE_FUNCTION_AS(DoFxrstor)
DoFxrstor:
    lea     xsaveArea, %ecx

    # Do fxrstor
    fxrstor   (%ecx)

    ret
