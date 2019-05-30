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
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define NUM_OF_THREADS 5
#define CHILD_TIMEOUT 30 //30 seconds
#define TIMEOUT 600      //10 minutes

static bool application_error = false;

extern "C" int AppShouldExit()
{
    if (application_error) return 1;
    return 0;
}

void RunApp()
{
    pid_t child = fork();
    if (child < 0) // fork failed - no child process is created
    {
        fprintf(stderr, "APPLICATION ERROR: fork failed\n");
        application_error = true;
        return;
    }
    
    if (child == 0)
    {
        // I am the child - exit immediately
        write(1, ".", 1);
        _exit(0);
    }
    else
    {        
        // I am the parent - wait for the child with timeout
        int status = 0, res = 0;
        int timer = CHILD_TIMEOUT;
        do {
            res = waitpid(child, &status, WNOHANG); 
            // waitpid will return: 
            //   0 : case child is alive but not finished. 
            //   child : case child finished and status is available. 
            //   -1 : case child is dead and no status is available (possibly died from kill -9)
            sleep(1);
        } while (res == 0 && timer-- > 0 );
                
        if (res == child)
        {
            if (! WIFEXITED(status))
            {
                fprintf(stderr, 
                        "APPLICATION ERROR: The child process %d did not terminate normally. status = 0x%x\n",
                        res, status);
                application_error = true;
            }
            else if (WEXITSTATUS(status) != 0)
            {
                fprintf(stderr, 
                "APPLICATION ERROR: The child process failed with status 0x%x\n", 
                    WEXITSTATUS(status));
                application_error = true;
            }
        } 
        else if (res == -1 )
        {
            //Child is dead (might be killed with kill -9), and status is not available
            fprintf(stderr, 
            "APPLICATION ERROR: child process exited unexpectedly, status is not available. \n");
            application_error = true;
        }
        else if ((res == 0) && (timer < 0))
        {
            // Something is wrong. The child process should have terminated by now and is most likely deadlocked. 
            // If we don't kill it then it will remain in the system.
            fprintf(stderr, 
                    "APPLICATION ERROR: Child process did not terminate after %d seconds. Killing child process. \n", 
                    CHILD_TIMEOUT);
            kill(child, SIGKILL);
            application_error = true;
        }
        else
        {
            fprintf(stderr, 
            "APPLICATION ERROR: Unknown error; waitpid returned with 0x%x ; child process exited with status 0x%x \n",
            res, status);
            application_error = true;
        }
    }
}

void *ThreadFunc(void* arg)
{
    while (! AppShouldExit())
    {
        RunApp();
        sleep(1);
    }
    int* exit_code = new int;
    *exit_code = 1;
    pthread_exit((void*)exit_code);
}


static void TimeoutHandler(int a)
{
    printf("Application is running more than %d minutes. It might be hanged. Killing it\n", TIMEOUT/60);
    kill(0, SIGTERM); // Kill the entire process group
}

int main(int argc, char **argv)
{
    pthread_t thHandle[NUM_OF_THREADS];

    struct sigaction sigact_timeout;
    sigact_timeout.sa_handler = TimeoutHandler;
    sigact_timeout.sa_flags = 0;
    sigfillset(&sigact_timeout.sa_mask);
                            
    if (sigaction(SIGALRM, &sigact_timeout, 0) == -1)
    {
        fprintf(stderr, "Unable to set up timeout handler errno=%d.\n", errno);
        return 1;
    }

    alarm(TIMEOUT);

    for (unsigned int i=0; i < NUM_OF_THREADS; ++i)
    {
        if (pthread_create(&thHandle[i], NULL, ThreadFunc, NULL) != 0)
        {
            fprintf(stderr, "pthread_create returned with an error, errno=%d.\n", errno);
            kill(0, SIGTERM); // Kill the entire process group
        }
    }

    while (! AppShouldExit())
    {
        RunApp();
        sleep(1);
    }

    // If we are here it means there was an application error. 
    // We are not interested in the exit status of the threads - there is failure anyway.
    // Allow the threads to exit gracefully. 
    // In case of a deadlock TimeoutHandler will be called and will kill the process group brute force.
    for (unsigned int i=0; i < NUM_OF_THREADS; ++i) pthread_join(thHandle[i], NULL);

    if (application_error) kill(0, SIGTERM); // Kill the entire process group
    return 0;
}
