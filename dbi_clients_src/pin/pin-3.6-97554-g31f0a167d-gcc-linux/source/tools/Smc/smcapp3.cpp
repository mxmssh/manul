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
 *  An example of SMC application. 
 */
#include "smc_util.h"
#include "../Utils/sys_memory.h"
#include <stdio.h>

/*!
 * Exit with the specified error message
 */
static void Abort(string msg)
{
    fprintf (stderr, "%s\n", msg.c_str());
    exit(1);
}


/*!
 * The main procedure of the application.
 */
int main(int argc, char *argv[])
{
    fprintf (stderr, "SMC in the image of the application\n");

    // buffer to move foo routines into and execute
    static char staticBuffer1[PI_FUNC::MAX_SIZE];
    static char staticBuffer2[PI_FUNC::MAX_SIZE];
    static char staticBuffer3[PI_FUNC::MAX_SIZE];
    // Set read-write-execute protection for the buffer 
    size_t pageSize = GetPageSize();
    char * firstPage1 = (char *)(((size_t)staticBuffer1) & ~(pageSize - 1));
    char * endPage1 = (char *)(((size_t)staticBuffer1 + sizeof(staticBuffer1) + pageSize - 1) & ~(pageSize - 1));
    char *firstPage2 = (char *)(((size_t)staticBuffer2) & ~(pageSize - 1));
    char * endPage2 = (char *)(((size_t)staticBuffer2 + sizeof(staticBuffer2) + pageSize - 1) & ~(pageSize - 1));
    char *firstPage3 = (char *)(((size_t)staticBuffer3) & ~(pageSize - 1));
    char * endPage3 = (char *)(((size_t)staticBuffer3 + sizeof(staticBuffer3) + pageSize - 1) & ~(pageSize - 1));
    if (!MemProtect(firstPage1, endPage1 - firstPage1, MEM_READ_WRITE_EXEC)) {Abort("MemProtect failed");}
    if (!MemProtect(firstPage2, endPage2 - firstPage2, MEM_READ_WRITE_EXEC)) {Abort("MemProtect failed");}
    if (!MemProtect(firstPage3, endPage3 - firstPage3, MEM_READ_WRITE_EXEC)) {Abort("MemProtect failed");}
    int printed = 0;
    FOO_FUNC fooFunc;
    FILE *fp = fopen ("smcapp3_a.out", "w");
    fprintf (fp, "%p\n", staticBuffer1);
    fclose(fp);
    fooFunc.Copy(staticBuffer1).Execute().AssertStatus();

    fp = fopen ("smcapp3_b.out", "w");
    fprintf (fp, "%p\n", staticBuffer2);
    fclose(fp);
    fooFunc.Copy(staticBuffer2).Execute().AssertStatus();

    fooFunc.Copy(staticBuffer3).Execute().AssertStatus();

           
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
