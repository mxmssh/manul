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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#include <iostream>
using namespace std;

#include "atfork_callbacks.h"

/***********************************************
*
*   This app prints a number for each of the pthreads callbacks.
*   The pintool (atfork_callbacks.cpp) also prints numbers for the callbacks from PIN.
*
*   The before-fork callbacks are atfork_prepare() and pintool_on_fork_before()
*   atfork_prepare() should precede pintool_on_fork_before()
*
*   The after-fork callbacks for PARENT are atfork_parent() and pintool_on_fork_after_in_parent()
*   pintool_on_fork_after_in_parent() should precede after-fork()
*
*   The after-fork callbacks for CHILD are atfork_child() and pintool_on_fork_after_in_child()
*   pintool_on_fork_after_in_child() should precede atfork_child()
*
************************************************/

void atfork_prepare()
{
    fork_callbacks* callbacks = get_shared_object();
    assert(callbacks);

    if (callbacks->pin_before_fork != 0)
    {
        cerr << "pintool_on_fork_before() was called before atfork_prepare()" << endl;
        exit(1);
    }
    callbacks->atfork_before = 1;
}

void atfork_parent()
{
    fork_callbacks* callbacks = get_shared_object();
    assert(callbacks);

    if (callbacks->pin_after_fork_parent != 1)
    {
        cerr << "atfork_parent() was called before pintool_on_fork_after_in_parent()" << endl;
        exit(1);
    }
    callbacks->atfork_after_parent = 1;
}

void atfork_child()
{
    fork_callbacks* callbacks = get_shared_object();
    assert(callbacks);

    if (callbacks->pin_after_fork_child != 1)
    {
        cerr << "atfork_child() was called before pintool_on_fork_after_in_child()" << endl;
        exit(1);
    }
    callbacks->atfork_after_child = 1;
}

int main()
{
    fork_callbacks* callbacks = create_shared_object();
    assert(callbacks);
    callbacks->atfork_before = 0;
    callbacks->atfork_after_child = 0;
    callbacks->atfork_after_parent = 0;
    callbacks->pin_before_fork = 0;
    callbacks->pin_after_fork_child = 0;
    callbacks->pin_after_fork_parent = 0;

    pthread_atfork(atfork_prepare, atfork_parent, atfork_child);

    pid_t child = fork();

    if (child != 0)
    {
        assert(child > 0);

        int status = 0;
        pid_t client =  wait(&status);
        assert(client == child);

        callbacks = get_shared_object();
        assert(callbacks);

        fork_callbacks callbacks_copy = *callbacks;

        remove_shared_object();

        assert(callbacks_copy.atfork_before);
        assert(callbacks_copy.atfork_after_child);
        assert(callbacks_copy.atfork_after_parent);
        assert(callbacks_copy.pin_before_fork);
        assert(callbacks_copy.pin_after_fork_child);
        assert(callbacks_copy.pin_after_fork_parent);

    }

    return 0;
}
