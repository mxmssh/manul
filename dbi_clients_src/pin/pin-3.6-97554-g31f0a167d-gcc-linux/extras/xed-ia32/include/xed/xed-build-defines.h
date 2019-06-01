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
#if !defined(XED_BUILD_DEFINES_H)
#  define XED_BUILD_DEFINES_H

#  if !defined(XED_AMD_ENABLED)
#    define XED_AMD_ENABLED
#  endif
#  if !defined(XED_AVX)
#    define XED_AVX
#  endif
#  if !defined(XED_CET)
#    define XED_CET
#  endif
#  if !defined(XED_DECODER)
#    define XED_DECODER
#  endif
#  if !defined(XED_DLL)
#    define XED_DLL
#  endif
#  if !defined(XED_ENCODER)
#    define XED_ENCODER
#  endif
#  if !defined(XED_GIT_VERSION)
#    define XED_GIT_VERSION "7.54.0-44-g6aebf27"
#  endif
#  if !defined(XED_MPX)
#    define XED_MPX
#  endif
#  if !defined(XED_SUPPORTS_AVX512)
#    define XED_SUPPORTS_AVX512
#  endif
#  if !defined(XED_SUPPORTS_LZCNT_TZCNT)
#    define XED_SUPPORTS_LZCNT_TZCNT
#  endif
#  if !defined(XED_SUPPORTS_SHA)
#    define XED_SUPPORTS_SHA
#  endif
#endif
