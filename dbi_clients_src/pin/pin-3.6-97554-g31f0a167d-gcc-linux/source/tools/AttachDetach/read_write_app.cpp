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
/*! @file
 *  Test detaching - reattach Pin on Linux
 *  The application tests Pin correctness in interrupted system calls 
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" int AppShouldExit()
{
    return 0;
}


int main()
{
    char filename[PATH_MAX];
    sprintf(filename, "rw_test.%d.txt", getpid());
    
    while (!AppShouldExit())
    {
        FILE *out = fopen(filename, "w+");
        if (NULL == out)
        {
            int err = errno;
            printf("Failed to open output file %d\n", err);
            return -1;
        }

        size_t longStrSize = sizeof("aaaaabbbbccccddddd_")*100;
        char* orgBuf = (char*)malloc(longStrSize+2);
        orgBuf[0] ='\0';

        for(int i=0; i<100; i++)
        {
            sprintf(orgBuf, "%saaaaabbbbccccddddd_", orgBuf);
        }
        char *buf = (char*)malloc(longStrSize+2);

        for(int i=0; i<10; i++)
        {
            if (fseek(out, 0, SEEK_SET) != 0)
            {
                printf("fseek failed\n");
                return -1;
            }
            size_t writtenBytes = fwrite(orgBuf, 1, longStrSize, out);
            if (writtenBytes != longStrSize)
            {
                printf("Write operation ended with %d bytes; the full size is %d\n", (int)writtenBytes, (int)longStrSize);
                return -1;
            }
            if (fseek(out, 0, SEEK_SET) != 0)
            {
                printf("fseek failed\n");
                return -1;
            }
            size_t readBytes = fread(buf, 1, longStrSize, out);
            if (readBytes != longStrSize)
            {
                printf("Read operation ended with %d bytes; the full size is %d\n", (int)readBytes, (int)longStrSize);
                return -1;
            }
            if (strcmp(orgBuf, buf))
            {
                printf("The strings are different after RW operation\n");
                return -1;
            }
        }
        fclose(out);      
        free(orgBuf);
        free(buf);
    }
    return 0;
}
