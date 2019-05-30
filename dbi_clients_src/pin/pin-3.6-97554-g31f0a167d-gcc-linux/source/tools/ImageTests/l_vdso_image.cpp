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
 *  This tool check that vdso image can be instrumented (see code comments for more details).
 *  The tool should be used with the l_vdso_image_app application.
 */

#include <iostream>
#include <fstream>
#include <linux/unistd.h>
#include "pin.H"


// Global variables

// A knob for defining the file with list of loaded images
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "l_vdso_image.log", "log file for the tool");

ofstream outFile; // The tool's output file for printing the loaded images.
BOOL beforeTimeOfDayCalled = false;
BOOL unload_vsdo = false;
BOOL vdsoUsed = false; // True if one of the functions below are fetched
ADDRINT vdsoGetTimeOfDayAddress = 0; // Address of __vdso_gettimeofday function
ADDRINT kernelVsyscallAddress = 0; // Address of __kernel_vsyscall function

/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */

VOID timeOfDayBefore()
{
    outFile << "Before __vdso_gettimeofday" << endl;
    beforeTimeOfDayCalled = true;
}

VOID kernelVSyscallRtnBefore(ADDRINT l_eax)
{
    if (l_eax == __NR_gettimeofday)
    {
        outFile << "Before __kernel_vsyscall with EAX equal to __NR_gettimeofday" << endl;
        beforeTimeOfDayCalled = true;
    }
}

VOID Trace(TRACE trace, VOID *v)
{
    ADDRINT traceAddress = TRACE_Address(trace);
    ADDRINT traceLastAddress = traceAddress + TRACE_Size(trace) - 1;

    if (((vdsoGetTimeOfDayAddress >= traceAddress) &&  (vdsoGetTimeOfDayAddress <= traceLastAddress))
        || ((kernelVsyscallAddress >= traceAddress) &&  (kernelVsyscallAddress <= traceLastAddress)))

    {
        outFile << "vdso used" << endl;
        vdsoUsed = true;
    }
}

/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */

static VOID ImageLoad(IMG img, VOID *v)
{
    if (!IMG_IsVDSO(img))
    {
        return;
    }
    outFile << IMG_Name(img) << endl;


    RTN getTimeOfDayRtn = RTN_FindByName(img, "__vdso_gettimeofday");
    RTN kernelVSyscallRtn = RTN_FindByName(img, "__kernel_vsyscall");
    if (!RTN_Valid(getTimeOfDayRtn) && !RTN_Valid(kernelVSyscallRtn))
    {
        cerr << "TOOL ERROR: Unable to find the __vdso_gettimeofday or __kernel_vsyscall functions in the application."
                << endl;
        PIN_ExitProcess(1);
    }

    // Different Linux OS's (as well as 32/64 bit) have different ways of calling VDSO gettimeofday service
    // (no easy switch). So just catching any of them will satisfy the test.
    if (RTN_Valid(getTimeOfDayRtn))
    {
        vdsoGetTimeOfDayAddress = RTN_Address(getTimeOfDayRtn);

        // Instrumenting __vdso_gettimeofday() will satisfy the test
        RTN_Open(getTimeOfDayRtn);
        RTN_InsertCall(getTimeOfDayRtn, IPOINT_BEFORE, (AFUNPTR)timeOfDayBefore, IARG_END);
        RTN_Close(getTimeOfDayRtn);
    }
    if (RTN_Valid(kernelVSyscallRtn))
    {
        kernelVsyscallAddress = RTN_Address(kernelVSyscallRtn);

        // Instrumenting __kernel_vsyscall() with EAX equal to __NR_gettimeofday will satisfy the test
        RTN_Open(kernelVSyscallRtn);
        RTN_InsertCall(kernelVSyscallRtn, IPOINT_BEFORE, (AFUNPTR)kernelVSyscallRtnBefore,
                IARG_REG_VALUE, REG_EAX,
                IARG_END);
        RTN_Close(kernelVSyscallRtn);
    }

}

static VOID ImageUnload(IMG img, VOID *v)
{
    if (IMG_IsVDSO(img))
    {
        unload_vsdo = true;
    }
}


static VOID Fini(INT32 code, VOID *v)
{
    ASSERT(unload_vsdo,
            "Error, VDSO wasn't unloaded");
    ASSERT(!vdsoUsed || beforeTimeOfDayCalled,
                "Error, VDSO gettimeofday service was not instrumented "
                "(__vdso_gettimeofday() or __kernel_vsyscall with __NR_gettimeofday)");
    // sanity check: A situation where VDSO was not used but the analysis was called shouldn't happen
    ASSERTX( !(!vdsoUsed && beforeTimeOfDayCalled) );
    outFile.close();
}


int main( INT32 argc, CHAR *argv[] )
{
    // Initialization.
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // Open the tool's output file for printing the loaded images.
    outFile.open(KnobOutputFile.Value().c_str());
    if(!outFile.is_open() || outFile.fail())
    {
        cerr << "TOOL ERROR: Unable to open the output file." << endl;
        PIN_ExitProcess(1);
    }

    IMG_AddInstrumentFunction(ImageLoad, 0);
    IMG_AddUnloadFunction(ImageUnload, 0);
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start the program.
    PIN_StartProgram(); // never returns

    return 1; // return error value
}
