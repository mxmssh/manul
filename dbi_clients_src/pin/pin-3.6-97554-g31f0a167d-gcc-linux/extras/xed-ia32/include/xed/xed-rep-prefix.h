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
/// @file xed-immed.h
/// 

#ifndef XED_REP_PREFIX_H
# define XED_REP_PREFIX_H

#include "xed-types.h"
#include "xed-common-defs.h"
#include "xed-iclass-enum.h"
/// @name REP-like prefix removal and addition
//@{

/// @ingroup DEC Take an instruction with a REP/REPE/REPNE prefix and
/// return the corresponding xed_iclass_enum_t without that prefix. The
/// return value differs from the other functions in this group: If the
/// input iclass does not have REP/REPNE/REPE prefix, the function returns
/// the original instruction.
XED_DLL_EXPORT xed_iclass_enum_t xed_rep_remove(xed_iclass_enum_t x);

/// @ingroup DEC Take an #xed_iclass_enum_t value without a REPE prefix and
/// return the corresponding #xed_iclass_enum_t with a REPE prefix. If the
/// input instruction cannot have have a REPE prefix, this function returns
/// XED_ICLASS_INVALID.
XED_DLL_EXPORT xed_iclass_enum_t xed_repe_map(xed_iclass_enum_t iclass);

/// @ingroup DEC Take an #xed_iclass_enum_t value without a REPNE prefix
/// and return the corresponding #xed_iclass_enum_t with a REPNE prefix. If
/// the input instruction cannot have a REPNE prefix, this function returns
/// XED_ICLASS_INVALID.
XED_DLL_EXPORT xed_iclass_enum_t xed_repne_map(xed_iclass_enum_t iclass);

/// @ingroup DEC Take an #xed_iclass_enum_t value without a REP prefix and
/// return the corresponding #xed_iclass_enum_t with a REP prefix. If the
/// input instruction cannot have a REP prefix, this function returns
/// XED_ICLASS_INVALID.
XED_DLL_EXPORT xed_iclass_enum_t xed_rep_map(xed_iclass_enum_t iclass);

/// @ingroup DEC Take an #xed_iclass_enum_t value for an instruction with a
/// REP/REPNE/REPE prefix and return the corresponding #xed_iclass_enum_t
/// without that prefix. If the input instruction does not have a
/// REP/REPNE/REPE prefix, this function returns XED_ICLASS_INVALID.
XED_DLL_EXPORT xed_iclass_enum_t xed_norep_map(xed_iclass_enum_t iclass);
//@}
#endif
