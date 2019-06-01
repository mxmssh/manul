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
#ifdef TARGET_MAC
.global _Fld1_a
_Fld1_a:
#else
.global Fld1_a
.type Fld1_a,  @function
Fld1_a:
#endif
    fld1
	fld1
	fld1
    ret


#ifdef TARGET_MAC
.global _Fld1_b
_Fld1_b:
#else
.global Fld1_b
.type Fld1_b,  @function
Fld1_b:
#endif
    fld1
	fld1
	fld1
    ret


#ifdef TARGET_MAC
.global _FldzToTop3_a
_FldzToTop3_a:
#else
.global FldzToTop3_a
.type FldzToTop3_a,  @function
FldzToTop3_a:
#endif
    fstp %st(0)
	fstp %st(0)
	fstp %st(0)
	fldz
	fldz
	fldz
    ret

#ifdef TARGET_MAC
.global _mmx_save
_mmx_save:
#else
.global mmx_save
.type mmx_save,  @function
mmx_save:
#endif
  fxsave (%rdi)
  RET


#ifdef TARGET_MAC
.global _mmx_restore
_mmx_restore:
#else
.global mmx_restore
.type mmx_restore,  @function
mmx_restore:
#endif
  fxrstor (%rdi)
  RET



