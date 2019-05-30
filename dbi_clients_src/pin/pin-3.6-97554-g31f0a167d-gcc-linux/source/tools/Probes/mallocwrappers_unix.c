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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

typedef typeof(malloc) * MallocType;
typedef typeof(free) * FreeType;

MallocType mallocFun;
FreeType   freeFun;


#ifndef STATIC_LINK

/* The maximal 32-bit decimal unsigned integer (4294967295) takes 10 bytes. */
#define max32BitDecLen 10

/* The maximal 64-bit hexadecimal unsigned integer (0xffffffffffffffff) takes 18 bytes. */
#define max64BitHexLen 18
#define max64BitHexNoPrefixLen 16


/*
 * A simple conversion function from a 32-bit unsigned integer to a string with no dynamic allocation of memory.
 * The function stores the string in the supplied buffer which is expected to be at least max32BitDecLen bytes
 * long. No \0 terminator is stored.
 *
 * The function returns the number of written digits.
 */
unsigned int SimpleDecStr(unsigned int num, char* buf)
{
    static const char conversionMap[10] =
    {
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9'
    };
    char temp[max32BitDecLen] = { 0 };
    unsigned int src = 0;
    unsigned int dst = 0;

    if (0 == num)
    {
        buf[0] = '0';
        return 1;
    }

    while (0 != num)
    {
        assert(src < max32BitDecLen);
        temp[src] = conversionMap[num%10];
        num /= 10;
        ++src;
    }

    while (0 < src)
    {
        --src;
        buf[dst] = temp[src];
        ++dst;
    }

    return dst;
}


/*
 * A simple conversion function from an unsigned integer (of pointer size) to a hexadecimal string representation
 * with no dynamic allocation of memory. The function stores the string in the supplied buffer which is expected
 * to be at least max64BitHexLen bytes long. No \0 terminator is stored.
 *
 * The function returns the number of written characters.
 */
unsigned int SimpleHexStr(uintptr_t num, char* buf)
{
    static const char conversionMap[16] =
    {
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        'a',
        'b',
        'c',
        'd',
        'e',
        'f'
    };
    char temp[max64BitHexNoPrefixLen] = { 0 };
    unsigned int src = 0;
    unsigned int dst = 2;
    buf[0] = '0';
    buf[1] = 'x';

    if (0 == num)
    {
        buf[2] = '0';
        return 3;
    }

    while (0 != num)
    {
        assert(src < max64BitHexNoPrefixLen);
        temp[src] = conversionMap[num%16];
        num >>= 4; /* num /= 16 */
        ++src;
    }

    while (0 < src)
    {
        --src;
        buf[dst] = temp[src];
        ++dst;
    }

    return dst;
}
#endif // not STATIC_LINK


void * mallocWrapper(int size)
{
    void * res = (*mallocFun)(size);

#ifdef STATIC_LINK
    fprintf(stderr,"malloc(%d) = %p\n", size, res);
#else
    static const char* startStr = "malloc(";
    static const char* middleStr = ") = ";
    static const char* endStr = "\n";
    static const unsigned int stringTermLen = 1; /* '\0' */

    const unsigned int startLen = strlen(startStr);
    const unsigned int middleLen = strlen(middleStr);
    const unsigned int endLen = strlen(endStr);
    const unsigned int msgSize = startLen + middleLen + endLen + max32BitDecLen + max64BitHexLen + stringTermLen;

    char msg[msgSize];
    unsigned int i = 0;

    /* Build the message as seen in the fprintf above. */
    strncpy(&(msg[i]), startStr, startLen);
    i += startLen;
    i += SimpleDecStr(size, &(msg[i]));
    strncpy(&(msg[i]), middleStr, middleLen);
    i += middleLen;
    i += SimpleHexStr((uintptr_t)res, &(msg[i]));
    strncpy(&(msg[i]), endStr, endLen);
    i += endLen;
    msg[i++] = '\0';
    
    write(STDERR_FILENO, msg, i);
#endif // not STATIC_LINK
    return res;
}

void freeWrapper(void *p)
{
#ifndef STATIC_LINK
    static const char* startStr = "free(";
    static const char* endStr = ")\n";
    static const unsigned int stringTermLen = 1; /* '\0' */

    const unsigned int startLen = strlen(startStr);
    const unsigned int endLen = strlen(endStr);
    const unsigned int msgSize = startLen + endLen + max64BitHexLen + stringTermLen;

    char msg[msgSize];
    unsigned int i = 0;

    /* Build the message as seen in the fprintf below. */
    strncpy(&(msg[i]), startStr, startLen);
    i += startLen;
    i += SimpleHexStr((uintptr_t)p, &(msg[i]));
    strncpy(&(msg[i]), endStr, endLen);
    i += endLen;
    msg[i++] = '\0';
#endif // not STATIC_LINK

    (*freeFun)(p);
    
#ifdef STATIC_LINK
    fprintf(stderr,"free(%p)\n",p);
#else
    write(STDERR_FILENO, msg, i);
#endif // STATIC_LINK
}

void SetOriginalFptr(MallocType m, FreeType f)
{
    mallocFun = m;
    freeFun = f;
}
