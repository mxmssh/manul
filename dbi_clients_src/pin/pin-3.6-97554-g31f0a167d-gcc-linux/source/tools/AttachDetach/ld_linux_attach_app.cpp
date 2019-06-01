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
 *
 * A test for attach to single threaded application.
 * The application launches Pin and waits until Pin attaches
 */

#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <assert.h>

using namespace std;

typedef bool (*FUNPTR)();

/* Pin doesn't kill the process if failed to attach, exit on SIGALRM */
void ExitOnAlarm(int sig)
{
    fprintf(stderr, "Pin is not attached, exit on SIGALRM\n");
    exit(0);
}

extern "C" bool PinAttached()
{
    return false;
}

/*
 * Expected command line: <this exe> [-th_num NUM] -pin $PIN -pinarg <pin args > -t tool <tool args>
 */

void ParseCommandLine(int argc, char *argv[], list < string>* pinArgs)//, list < string>* pinBinaryArgs)
{
    string pinBinary;
    string beforePinArgs;
    for (int i=1; i<argc; i++)
    {
        string arg = string(argv[i]);
        if (arg == "-pin")
        {
            pinBinary = argv[++i];
            if ((i + 1) < argc)
            {
                arg = string(argv[i + 1]);
                if ("-pinarg" != arg)
                {
                    beforePinArgs = argv[++i];
                }
            }
        }
        else if (arg == "-pinarg")
        {
            //Add -pinarg to point start of pin args.
            pinArgs->push_back(arg);
            for (int parg = ++i; parg < argc; parg++)
            {
                pinArgs->push_back(string(argv[parg]));
                ++i;
            }
        }
    }
    if (!beforePinArgs.empty())
    {
        pinArgs->push_front(beforePinArgs);
    }
    assert(!pinBinary.empty());
    pinArgs->push_front(pinBinary);
}

void StartPin(list <string>* pinArgs)
{
    pid_t appPid = getpid();
    pid_t child = fork();
    if (child > 0)
    {
        return;
    }
    else if (0 == child)
    {
        // start Pin from child
        char **inArgv = new char*[pinArgs->size()+10];

        // Pin binary in the first
        list <string>::iterator pinArgIt = pinArgs->begin();
        string pinBinary = *pinArgIt;
        pinArgIt++;

        // build pin arguments:
        // pin -pid appPid [pinarg]
        unsigned int idx = 0;
        inArgv[idx++] = (char *)pinBinary.c_str();
        if ("-pinarg" != *pinArgIt)
        {
            inArgv[idx++] = (char *)(pinArgIt++)->c_str();
        }
        pinArgIt++;
        inArgv[idx++] = (char*)"-pid";
        inArgv[idx] = (char *)malloc(10);
        sprintf(inArgv[idx++], "%d", appPid);
    
        for (; pinArgIt != pinArgs->end(); pinArgIt++)
        {
            inArgv[idx++]= (char *)pinArgIt->c_str();
        }
        inArgv[idx] = 0;

        execvp(inArgv[0], inArgv);
        fprintf(stderr, "ERROR: execv %s failed\n", inArgv[0]);
        exit(1);
    }
    else
    {
        fprintf(stderr, "ERROR: fork failed\n");
        exit(1);
    }
}

int main(int argc, char * argv[])
{
    list <string> pinArgs;
    volatile FUNPTR pinAttached = PinAttached;

    ParseCommandLine(argc, argv, &pinArgs);
    StartPin(&pinArgs);

    /* Exit in 20 sec */
    signal(SIGALRM, ExitOnAlarm);    
    alarm(20);

    while (!pinAttached())
    {
        sched_yield();
        sleep(2);
    }

    return 0;
}
