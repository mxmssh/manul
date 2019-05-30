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
#ifndef _ATFORK_CALLBACKS_H_
#define _ATFORK_CALLBACKS_H_

#include <iostream>
#include <errno.h>
#include <sys/shm.h>

const key_t shm_key = 1234;

typedef struct fork_callbacks_s {
    int atfork_before;
    int atfork_after_child;
    int atfork_after_parent;
    int pin_before_fork;
    int pin_after_fork_child;
    int pin_after_fork_parent;
} fork_callbacks;

inline fork_callbacks* get_shared_mem(bool create)
{
    int shmid = shmget(shm_key, sizeof(fork_callbacks), create ? IPC_CREAT | 0666 : 0666);
    if (shmid < 0)
    {
        cerr << "shmget failed with errno " << errno << endl;
        return NULL;
    }

    void* shm = shmat(shmid, NULL, 0);
    if (shm == (void*) -1) {
        cerr << "shmat failed with errno " << errno << endl;
        return NULL;
    }

    return (fork_callbacks*)shm;
}

inline bool remove_shared_object(bool silent = false)
{
    int shmid = shmget(shm_key, sizeof(fork_callbacks), 0666);
    if (shmid < 0)
    {
        if (!silent)
            cerr << "shmget failed with errno " << errno << endl;
        return false;
    }

    void* shm = shmat(shmid, NULL, 0);
    if (shm == (void*) -1) {
        if (!silent)
            cerr << "shmat failed with errno " << errno << endl;
        return false;
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        if (!silent)
            cerr << "shmctl failed with errno " << errno << endl;
        return false;
    }

    if (shmdt(shm) == -1)
    {
        if (!silent)
            cerr << "shmdt failed with errno " << errno << endl;
        return false;
    }

    return true;

}

inline fork_callbacks* create_shared_object()
{
    fork_callbacks* ret = NULL;
    remove_shared_object( true );
    ret = get_shared_mem( true );
    if (ret == NULL)
    {
        remove_shared_object( true );
    }
    return ret;
}

inline fork_callbacks* get_shared_object()
{
    fork_callbacks* ret = NULL;
    ret = get_shared_mem( false );
    if (ret == NULL)
    {
        remove_shared_object( true );
    }
    return ret;
}

#endif
