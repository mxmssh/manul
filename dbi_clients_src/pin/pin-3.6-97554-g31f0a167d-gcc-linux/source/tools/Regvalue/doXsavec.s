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

.data
.extern xsaveArea
.extern flags

.text

# void DoXsavec();
# This function calls xsave and stores the FP state in the given dst area.
# The caller is expected to allocate enough space for the xsave area.
# The function expects the given dst pointer to be properly aligned for the xsave instruction.
.global NAME(DoXsavec)
NAME(DoXsavec):

    lea     PIC_VAR(flags), GCX_REG
    mov     (GCX_REG), GAX_REG
    lea     PIC_VAR(xsaveArea), GCX_REG
    xor     GDX_REG, GDX_REG

    # Do xsave
    xsavec   (GCX_REG)

    ret

# void DoXsaveOpt();
# This function calls xsaveopt and stores the FP state in the given dst area.
# The caller is expected to allocate enough space for the xsaveopt area.
# The function expects the given dst pointer to be properly aligned for the xsaveopt instruction.
.global NAME(DoXsaveOpt)
NAME(DoXsaveOpt):

    lea     PIC_VAR(flags), GCX_REG
    mov     (GCX_REG), GAX_REG
    lea     PIC_VAR(xsaveArea), GCX_REG
    xor     GDX_REG, GDX_REG

    # Do xsaveopt
    xsaveopt   (GCX_REG)

    ret

# void DoXrstor();
# This function calls xrstor and restores the specified thetures from the xsave dst area.
# The function expects the given dst pointer to be properly aligned
.global NAME(DoXrstor)
NAME(DoXrstor):

    lea     PIC_VAR(flags), GCX_REG
    mov     (GCX_REG), GAX_REG
    lea     PIC_VAR(xsaveArea), GCX_REG
    xor     GDX_REG, GDX_REG

    # Do xsaveopt
    xrstor   (GCX_REG)

    ret
