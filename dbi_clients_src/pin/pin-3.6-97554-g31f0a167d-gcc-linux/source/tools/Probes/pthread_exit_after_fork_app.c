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
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>
#include <pthread.h>  
#include <assert.h>
#include <errno.h>
#include <sys/wait.h>

void *functionC(void *p0);  

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;  


int main()
{
    int res;
    pthread_t thread1;
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed\n");
        return 1;
    }
    else if (pid != 0)
    {
        // parent
        int stat;
        pid_t waitpid_ret;
        while (0 > (waitpid_ret = waitpid(pid, &stat, 0)) && errno == EINTR);
        if (0 > waitpid_ret)
        {
            perror("waitpid failed\n");
            return 2;
        }
        if (WIFEXITED(stat))
        {
            // process exited
            return WEXITSTATUS(stat);
        }
        // process terminated by a signal
        return WTERMSIG(stat) + 128;
    }
  
    int param;
    if ( (res = pthread_create( &thread1, NULL, &functionC, &param)) )  
    {
        printf("Thread creation failed: %d\n", res);
        return 1;
    }
    
    pthread_join( thread1, NULL);
    /*   pthread_join( thread2, NULL);*/
    return 0;
}

void *functionC(void *p0)
{
    pthread_exit(p0);
    printf("ERROR: This code comes after pthread_exit. It should not be executed!\n");
    assert(0);
}
