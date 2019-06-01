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
# contains important code patterns
# doesn't actually do anything
# Used to make sure that the probe is done only when allowed.

#very short func, not valid for probe
.global too_short
.type too_short, function
too_short:
    ret
    .size       too_short, .-too_short

.type hidden_no_nops, function
hidden_no_nops:
    nop
    push %ebx
    pop %ebx
    push %ebx
    pop %ebx
    push %ebx
    pop %ebx
    ret
    .size hidden_no_nops, .-hidden_no_nops

#very short func followed by padding nops, valid for probe
.global too_short_with_nops
.type too_short_with_nops, function
too_short_with_nops:
    ret
    .size too_short_with_nops, .-too_short_with_nops

.type hidden_nops, function
hidden_nops:
    nop
    nop
    nop
    nop
    nop
    nop
    ret
    .size hidden_nops, .-hidden_nops

    
.global call_hidden_function
.type call_hidden_function, function
call_hidden_function:
    push %ebx
    call hidden_no_nops
    call hidden_nops
    pop %ebx
    ret
    .size call_hidden_function, .-call_hidden_function

.global call_function
.type call_function, function

call_function:
    push %ebx
    call too_short
    call too_short_with_nops
    pop %ebx
    ret
    .size call_function, .-call_function

