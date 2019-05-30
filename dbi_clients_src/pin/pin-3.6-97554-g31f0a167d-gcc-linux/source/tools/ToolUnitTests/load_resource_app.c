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
#include <windows.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // Modify field in TEB that is temporary used by OS loader to point at image name
    // when binary image is mapped for execution.
    WCHAR *str = L"garbage";
    struct _TEB *pt = NtCurrentTeb();
    PVOID *aup = &((PNT_TIB)pt)->ArbitraryUserPointer;
    *aup = (PVOID)str;

    {
        // LOAD_LIBRARY_AS_IMAGE_RESOURCE flag orders to map DLL as image but not for execution.
        // OS loader doesn't temporary modify ArbitraryUserPointer TEB field in this case,
        // so value set by the application remains visible.
        HMODULE hMod = LoadLibraryEx("mswsock.dll", NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
        DWORD err = GetLastError();
        printf("base = %x, error = %x\n", hMod, err);
        return (hMod == 0);
    }
}
