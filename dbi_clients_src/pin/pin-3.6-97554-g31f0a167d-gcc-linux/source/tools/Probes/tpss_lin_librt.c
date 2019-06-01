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
/*
 * This tool mimics the behavior of TPSS on Linux by adding probes to various librt functions.
 * However, in this tool these probes are merely empty wrappers that call the original functions.
 * The objective of the test is to verify that probe generation and insertion don't cause Pin
 * to crash.
 *
 * This file is part of the tpss_lin_librt tool and compiles against the native
 * libc of the machine/compiler in order to extact data types definition from it's headers.
 */

#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

typedef struct timespec * STRUCT_TIMESPEC_PTR;

typedef char * CHAR_PTR;

typedef unsigned * UNSIGNED_PTR;

extern void printFunctionCalled(const char* funcName);

/* ===================================================================== */
/* Function signatures - these functions will be probed                  */
/* ===================================================================== */

int (*fptrclock_nanosleep)(clockid_t __clock_id, int __flags, const STRUCT_TIMESPEC_PTR __rqtp, STRUCT_TIMESPEC_PTR __rmtp);

int (*fptrmq_close)(mqd_t __mqdes);

mqd_t (*fptrmq_open)(const CHAR_PTR __name, int __oflag);

ssize_t (*fptrmq_receive)(mqd_t __mqdes, CHAR_PTR __msg_ptr, size_t __msg_len, UNSIGNED_PTR __msg_prio);

ssize_t (*fptrmq_timedreceive)(mqd_t __mqdes, CHAR_PTR __msg_ptr, size_t __msg_len, UNSIGNED_PTR __msg_prio,  const STRUCT_TIMESPEC_PTR __abs_timeout);

int (*fptrmq_send)(mqd_t __mqdes, const CHAR_PTR __msg_ptr, size_t __msg_len, unsigned __msg_prio);

int (*fptrmq_timedsend)(mqd_t __mqdes, const CHAR_PTR __msg_ptr, size_t __msg_len, unsigned __msg_prio, const STRUCT_TIMESPEC_PTR __abs_timeout);

/* ===================================================================== */
/* Probes - implementation of the wrapper functions                      */
/* ===================================================================== */

int myclock_nanosleep(clockid_t __clock_id, int __flags, const STRUCT_TIMESPEC_PTR __rqtp, STRUCT_TIMESPEC_PTR __rmtp)
{
   printFunctionCalled("myclock_nanosleep");
   int res = fptrclock_nanosleep(__clock_id,  __flags, __rqtp, __rmtp);

   return res;
}

int mymq_close(mqd_t __mqdes)
{
   printFunctionCalled("mymq_close");
   int res = fptrmq_close(__mqdes);

   return res;
}

mqd_t mymq_open(const CHAR_PTR __name, int __oflag)
{
   printFunctionCalled("mymq_open");
   mqd_t res = fptrmq_open(__name, __oflag);

   return res;
}

ssize_t mymq_receive(mqd_t __mqdes, CHAR_PTR __msg_ptr, size_t __msg_len, UNSIGNED_PTR __msg_prio)
{
   printFunctionCalled("mymq_receive");
   ssize_t res = fptrmq_receive( __mqdes,  __msg_ptr,  __msg_len,  __msg_prio);

   return res;
}

ssize_t mymq_timedreceive(mqd_t __mqdes, CHAR_PTR __msg_ptr, size_t __msg_len, UNSIGNED_PTR __msg_prio,  const STRUCT_TIMESPEC_PTR __abs_timeout)
{
   printFunctionCalled("mymq_timedreceive");
   ssize_t res = fptrmq_timedreceive( __mqdes, __msg_ptr, __msg_len, __msg_prio, __abs_timeout);

   return res;
}

int mymq_send(mqd_t __mqdes, const CHAR_PTR __msg_ptr, size_t __msg_len, unsigned __msg_prio)
{
   printFunctionCalled("mymq_send");
   int res = fptrmq_send(__mqdes, __msg_ptr,  __msg_len,  __msg_prio);

   return res;
}

int mymq_timedsend(mqd_t __mqdes, const CHAR_PTR __msg_ptr, size_t __msg_len, unsigned __msg_prio, const STRUCT_TIMESPEC_PTR __abs_timeout)
{
   printFunctionCalled("mymq_timedsend");
   int res = fptrmq_timedsend(__mqdes, __msg_ptr, __msg_len, __msg_prio, __abs_timeout);

   return res;
}

