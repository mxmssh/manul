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
 * Application launcher used for testing Pin in attach mode (as opposed to launch mode)
 * It creates a child process (which is the application we want Pin to attach to) and write its process ID
 * to the output which is later used by Pin to attach to it.
 * The difference between this launcher and w_app_launcher.cpp is that this launcher waits for child process to
 * notify it is ready to be attached by Pin (using semaphore). Intended for tests that the attach point the
 * in time is important for them.
 */
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <assert.h>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// launch given application, write application's PID to stdout

int main(int argc, char * argv[], char * envp[])
{
    string cmdLine = "";
    // Build command line
    for(int i = 1; i < argc; i++)
    {
        cmdLine += string(argv[i]);
        if(i != argc - 1)
        {
            cmdLine +=  string(" ");
        }
    }

    // Invoke Application
    STARTUPINFO         si;
    PROCESS_INFORMATION  pi;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    memset(&pi, 0, sizeof(pi));

    // Create child process suspended in order to get its process ID first (see below why)
    //
    if (!CreateProcess(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
    {
        cout <<  "0" << endl;
        cerr <<  "Failed to create " << cmdLine << endl;
        exit(-1);
    }


    //
    // Wait for child process to notify it is ready to be attached by Pin by using the below semaphore.
    //

    // Create a unique semaphore handle name using the child process ID
	ostringstream stream;
	stream << pi.dwProcessId;
    string semaphoreHandleName = "semaphore_handle_" + stream.str();

    HANDLE ChildReadySemaphore;
    ChildReadySemaphore = CreateSemaphore(
        NULL,   // default security attributes
        0,      // initial count
        1,      // maximum count
        semaphoreHandleName.c_str());  // unnamed semaphore

    // Now that the semaphore handle exist we can resume the child process which will
    // use this handle (by using the unique name)
    ResumeThread(pi.hThread);

	cerr << "  Launcher: waiting on " << semaphoreHandleName << endl;
	DWORD waitResult = WaitForSingleObject(
            ChildReadySemaphore, // handle to semaphore
            30000);              // wait 30 seconds

	if (waitResult == WAIT_TIMEOUT)
    {
	    cerr << "Error: Child process didn't notify it is ready to be attached by Pin for more than 30 seconds. ";
        return 1;
    }
	assert(waitResult == WAIT_OBJECT_0);

	cerr << "  Launcher: "<< semaphoreHandleName << " was relesed by App" << endl;

	// Now we can proceed to attach Pin to child process by writing its process ID (will be used by Pin)
    char digitBuffer[64];
    cout << itoa(pi.dwProcessId, digitBuffer, 10);

	CloseHandle(ChildReadySemaphore);

    return 0;
}

