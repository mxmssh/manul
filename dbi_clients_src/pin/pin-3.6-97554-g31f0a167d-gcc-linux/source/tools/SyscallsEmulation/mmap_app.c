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
#ifdef NDEBUG
# undef NDEBUG
#endif
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

void createFileWithRandomData(const char* filename, size_t desiredFileLength)
{
    size_t totalWritten = 0;
    int pgSize = getpagesize();
    char* pageBuf = malloc(pgSize);
    int outFd;
    outFd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);
    assert(outFd >= 0);
    while (totalWritten < desiredFileLength)
    {
        int i;
        int* p = (int*)pageBuf;
        for (i = 0; i < pgSize; i += sizeof(int), p++)
        {
            *p = random();
        }
        size_t thisTimeWriten = 0;
        while (thisTimeWriten < pgSize)
        {
            ssize_t writeCount = write(outFd, pageBuf + thisTimeWriten, pgSize - thisTimeWriten);
            assert(writeCount > 0 || (writeCount < 0 && errno == EINTR));
            if (writeCount > 0)
            {
                thisTimeWriten += writeCount;
            }
        }
        totalWritten += pgSize;
    }
    close(outFd);
    free(pageBuf);
}

void* readOnePage(int fd)
{
    size_t totalRead = 0;
    char* buf = malloc(getpagesize());
    while (totalRead < getpagesize())
    {
        ssize_t readCount = read(fd, buf + totalRead, getpagesize() - totalRead);
        assert(readCount > 0 || (readCount < 0 && errno == EINTR));
        if (readCount > 0)
        {
            totalRead += readCount;
        }
    }
    return (void*)buf;
}

int main(int argc, const char* argv[])
{
    const char* filename = argv[1];
    int fd, fd2;
    void *addr, *buf;
    if (argc != 2)
    {
        fprintf(stderr, "Usage %s <file-to-generate>\n", argv[0]);
        return 1;
    }

    createFileWithRandomData(filename, getpagesize() * 5);
    fd = open(filename, O_RDONLY);
    assert(fd >= 0);
    fd2 = dup(fd);

    addr = mmap(NULL, getpagesize(), PROT_READ, MAP_FILE | MAP_PRIVATE , fd2, 3 * getpagesize());
    assert(NULL != addr);

    lseek(fd, 3 * getpagesize(), SEEK_SET);
    buf = readOnePage(fd);

    assert(memcmp(buf, addr, getpagesize()) == 0);

    free(buf);

    return 0;
}
