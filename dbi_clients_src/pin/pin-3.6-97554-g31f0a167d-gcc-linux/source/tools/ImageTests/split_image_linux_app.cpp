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
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <limits.h>
#include <link.h>

// The executable filename of this program
char progname[4096];

using namespace std;

/*
 * Iterate over all images known to the loader and print their memory regions
 */
int dl_iterate(struct dl_phdr_info *info, size_t size, void *data)
{
    const char* realname = info->dlpi_name;
    vector<pair<void*, void*> > vecSegments;
    if (strstr(realname, "linux-gate.so") == realname || strstr(realname, "linux-vdso.so") == realname)
    {
        // Don't count VDSO, PIN doesn't repont it intentionally
        return 0;
    }
    for (int j = 0; j < info->dlpi_phnum; j++)
    {
        if (info->dlpi_phdr[j].p_type == PT_LOAD)
        {
            void* start = (void *) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);
            void* end = (void*)((char*)start + info->dlpi_phdr[j].p_memsz - 1);
            vecSegments.push_back(pair<void*, void*>(start, end));
            if (*realname == 0 && (void*)dl_iterate >= start && (void*)dl_iterate < end)
            {
                realname = progname;
            }
        }
    }
    if (*realname != 0)
    {
        for (vector<pair<void*, void*> >::iterator it = vecSegments.begin(); it != vecSegments.end(); it++)
        {
            printf("%s, %p-%p\n", realname, it->first, it->second);
        }
    }
    return 0;
}

int main(int argc, const char* argv[], char** envp)
{
    if (argc != 2)
    {
        printf("Usage: %s <path to lib>\n", argv[0]);
        return 1;
    }
    realpath(argv[0], progname);
    void* hDll = dlopen(argv[1], RTLD_LAZY | RTLD_GLOBAL);
    if (NULL == hDll)
    {
        printf("Failed to open %s - %s\n", argv[1], dlerror());
        return 1;
    }
    int (*return2)() = (int (*)())dlsym(hDll, "return2");
    if (NULL == return2)
    {
        printf("Failed to locate 'return2' - %s\n", dlerror());
        return 1;
    }

    if (return2() != 2)
    {
        printf("Bad value returned from 'return2'\n");
        return 1;
    }
    dl_iterate_phdr(dl_iterate, NULL);
    return 0;
}
