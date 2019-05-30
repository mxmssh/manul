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
 *  pin tool combined from multi-DLLs (main_dll, dynamic_secondary_dll, static_secondary_dll). 
 *  This is the "main DLL", use PIN API only in this DLL
 *  usage of PIN API in dynamic_secondary_dll and static_secondary_dll is not allowed
 *  (see README for more inforamtion)
 *
 *  NOTE: New Pin image loader does not (yet) support dynamic loading of Pin DLLs.
 *        Code related to dynamic_secondary_dll was suppressed.
 *        Look at Mantis 3280 for updates.
 *        #define DYN_LOAD will enable validation of dynamic loading feature in the test.
 */

#include <iostream>
#include <fstream>

#include <link.h>
#include <dlfcn.h>

#include "pin.H"

using namespace std;

KNOB<BOOL> KnobEnumerate(KNOB_MODE_WRITEONCE, "pintool",
    "enumerate", "0", "Enumerate modules loaded by Pin");

/* ===================================================================== */
/* Global Variables and Declerations */
/* ===================================================================== */

PIN_LOCK pinLock;

typedef VOID (* BEFORE_BBL)(ADDRINT ip);
typedef int (* INIT_F)(bool enumerate);
typedef VOID (* FINI_F)();

#if defined(DYN_LOAD)
// Functions pointers for dynamic_secondary_dll
BEFORE_BBL pBeforeBBL2;
INIT_F pInit2;
FINI_F pFini2;
#endif

// Dll imports for static_secondary_dll
extern "C" __declspec( dllimport ) VOID BeforeBBL1(ADDRINT ip);
extern "C" __declspec( dllimport ) VOID Init1();
extern "C" __declspec( dllimport ) VOID Fini1();

/* ===================================================================== */

// This function is called before every basic block
VOID PIN_FAST_ANALYSIS_CALL BeforeBBL(ADDRINT ip) 
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    BeforeBBL1(ip);
#if defined(DYN_LOAD)
    pBeforeBBL2(ip);
#endif
    PIN_ReleaseLock(&pinLock);
}

/* ===================================================================== */

// Pin calls this function every time a new trace is encountered
VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to BeforeBBL before every bbl, passing the ip address.
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)BeforeBBL, IARG_FAST_ANALYSIS_CALL, 
                       IARG_INST_PTR, IARG_END);
    }
}

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    BeforeBBL1(0);
#if defined(DYN_LOAD)
    pBeforeBBL2(0);
#endif
    PIN_ReleaseLock(&pinLock);
}

VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{
    PIN_GetLock(&pinLock, PIN_GetTid());
    BeforeBBL1(0);
#if defined(DYN_LOAD)
    pBeforeBBL2(0);
#endif
    PIN_ReleaseLock(&pinLock);
}

/* ===================================================================== */

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    Fini1();
#if defined(DYN_LOAD)
    pFini2();
#endif
}

// This function gets info of an image loaded by Pin loader.
// Invoked by dl_iterate_phdr()
int dl_iterate_callback(struct dl_phdr_info * info, size_t size, VOID * data)
{
    cerr << info->dlpi_name << " " << hex << info->dlpi_addr << " " << info->dlpi_phdr->p_memsz << endl;
    // Increment module counter.
    ++(*reinterpret_cast<int *>(data));
    return 0;
}

/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    PIN_InitLock(&pinLock);

    // Register Trace() to be called to instrument traces
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini() to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Call Static secondary dll Init1()
    Init1();

    int nModules;

#if defined(DYN_LOAD)
    // Dynamic secondary dll - load library, initialize function pointers
    // and call Init2()
    VOID * module = dlopen("dynamic_secondary_dll.dll", RTLD_NOW);
    if (module == NULL)
    {
        cerr << "Failed to load dynamic_secondary_dll.dll" << endl;
        exit(1);
    }
    pInit2 = reinterpret_cast<INIT_F>(dlsym(module, "Init2"));
    pBeforeBBL2 = reinterpret_cast<BEFORE_BBL>(dlsym(module, "BeforeBBL2"));
    pFini2 = reinterpret_cast<FINI_F>(dlsym(module, "Fini2"));
    if (pInit2 == NULL || pBeforeBBL2 == NULL || pFini2 == NULL)
    {
        cerr << "Failed to find proc addresses in dynamic_secondary_dll.dll" << endl;
        exit(1);
    }

    nModules = pInit2(KnobEnumerate);
#endif

    int nModulesMain = 0;
    // Enumerate DLLs currently loaded by Pin loader.
    dl_iterate_phdr(dl_iterate_callback, &nModulesMain);

    if (KnobEnumerate && ((nModulesMain <= 0) || (nModulesMain != nModules)))
    {
        // Failure. Module enumeration results in main and dynamic Dlls don't match.
        PIN_ExitApplication(1);
    }

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
