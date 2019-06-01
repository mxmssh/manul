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
#ifndef CREATE_AND_EXIT_THREAD_UTILS_H
#define CREATE_AND_EXIT_THREAD_UTILS_H


enum RETVAL
{
    RETVAL_SUCCESS = 0,
    RETVAL_FAILURE_PIN_INIT_FAILED,
    RETVAL_FAILURE_SEMAPHORE_INIT_FAILED,
    RETVAL_FAILURE_TOOL_MAIN_RETURN,
    RETVAL_FAILURE_TEST_TIMEOUT,
    RETVAL_FAILURE_MAX_TRIALS,
    RETVAL_FAILURE_TOOL_FAILED_TO_SPAWN,
    RETVAL_FAILURE_TOOL_FAILED_TO_EXIT,
    RETVAL_FAILURE_TOO_MANY_THREADS,
    RETVAL_FAILURE_START_AFTER_FINI,
    RETVAL_FAILURE_APP_COMPLETED
};

#endif // CREATE_AND_EXIT_THREAD_UTILS_H
