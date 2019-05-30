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
#include "os_specific.h"
#include "os-apis/host.h"

static int IsHostArch64Bit()
{
    OS_RETURN_CODE os_ret;
    OS_HOST_CPU_ARCH_TYPE host_arch;
    static int host_is_64bit = 1;
    static int checked = 0;

    if (1 == checked) return host_is_64bit;
    checked = 1;

    os_ret = OS_GetHostCPUArch( &host_arch );
    if (os_ret.generic_err == OS_RETURN_CODE_NO_ERROR)
    {
        if (OS_HOST_CPU_ARCH_TYPE_IA32 == host_arch)
        {
            host_is_64bit = 0;
        }
        if (host_arch == OS_HOST_CPU_ARCH_TYPE_INVALID)
        {
            fprintf(stderr, "OS_GetHostCPUArch returned OS_HOST_CPU_ARCH_TYPE_INVALID. Assuming 64 bit.\n");
        }
    }
    else
    {
        fprintf(stderr, "OS_GetHostCPUArch returned with error. Assuming 64 bit.\ngeneric_err=%d, os_specific_err=%d \n",
                        (int)os_ret.generic_err, os_ret.os_specific_err);
    }
    return host_is_64bit;
}


void update_environment(char* base_path)
{
    /*
     Set the following environment variables:
     1) PIN_VM32_LD_LIBRARY_PATH and PIN_VM64_LD_LIBRARY_PATH
     2) PIN_LD_RESTORE_REQUIRED (set it to "t")
     3) PIN_APP_LD_LIBRARY_PATH (copy of LD_LIBRARY_PATH (if set))
     4) PIN_APP_LD_ASSUME_KERNEL (copy of LD_ASSUME_KERNEL (if set))
     4a) PIN_APP_LD_BIND_NOW
     4b) PIN_APP_LD_PRELOAD
     5) unset LD_ASSUME_KERNEL
     6) PIN_INJECTOR32_LD_LIBRARY_PATH and PIN_INJECTOR64_LD_LIBRARY_PATH
     7) Set LD_LIBRARY_PATH to the proper PIN_INJECTOR*_LD_LIBRARY_PATH based on the host architecture.

     On Unix systems, we run pinbin instead of pin.
     */

    int r;
    const int overwrite = 1;
    char* pin_32_ld_library_path = 0;
    char* pin_64_ld_library_path = 0;
    char* injector_32_ld_library_path = 0;
    char* injector_64_ld_library_path = 0;
    const char* pin_runtime_dir = "runtime";
    const char* lib_ext_dir = "lib-ext";
    const char* pincrt_lib_dir = "pincrt";
    const char* extras_dir = "extras";
    const char* lib_dir = "lib";
    char* ld_library_path = 0;
    char* ld_assume_kernel = 0;
    char* base_path32 = 0;
    char* base_path64 = 0;
    char* extras_path = 0;
    char* xed32 = 0;
    char* xed64 = 0;
    char* xed_runtime_libs32 = 0;
    char* xed_runtime_libs64 = 0;
    char* pin_runtime_libs32 = 0;
    char* pin_runtime_libs64 = 0;
    char* pincrt_libs32 = 0;
    char* pincrt_libs64 = 0;
    char* ext_libs32 = 0;
    char* ext_libs64 = 0;
    char* incoming_ld_preload = 0;
    char* incoming_ld_bind_now = 0;

    base_path32 = append3(base_path, "/", "ia32");
    base_path64 = append3(base_path, "/", "intel64");

    extras_path = append3(base_path, "/", extras_dir);
    xed32 = append3(extras_path, "/", "xed-ia32");
    xed64 = append3(extras_path, "/", "xed-intel64");
    xed_runtime_libs32 = append3(xed32, "/", lib_dir);
    xed_runtime_libs64 = append3(xed64, "/", lib_dir);

    /* make pin_libs - required for pin/vm */
    pin_runtime_libs32 = append3(base_path32, "/", pin_runtime_dir);
    pin_runtime_libs64 = append3(base_path64, "/", pin_runtime_dir);

    ext_libs32 = append3(base_path32, "/", lib_ext_dir);
    ext_libs64 = append3(base_path64, "/", lib_ext_dir);

    pincrt_libs32 = append3(pin_runtime_libs32, "/", pincrt_lib_dir);
    pincrt_libs64 = append3(pin_runtime_libs64, "/", pincrt_lib_dir);

    /* Set Pin Vm library paths */
    pin_32_ld_library_path = getenv("PIN_VM32_LD_LIBRARY_PATH");
    pin_64_ld_library_path = getenv("PIN_VM64_LD_LIBRARY_PATH");

    pin_32_ld_library_path = append3(xed_runtime_libs32, ":", pin_32_ld_library_path);
    pin_64_ld_library_path = append3(xed_runtime_libs64, ":", pin_64_ld_library_path);
    pin_32_ld_library_path = append3(ext_libs32, ":", pin_32_ld_library_path);
    pin_64_ld_library_path = append3(ext_libs64, ":", pin_64_ld_library_path);
    pin_32_ld_library_path = append3(pincrt_libs32, ":", pin_32_ld_library_path);
    pin_64_ld_library_path = append3(pincrt_libs64, ":", pin_64_ld_library_path);

    r = setenv("PIN_VM32_LD_LIBRARY_PATH", pin_32_ld_library_path, overwrite);
    check_retval(r, "setenv PIN_VM32_LD_LIBRARY_PATH");
    r = setenv("PIN_VM64_LD_LIBRARY_PATH", pin_64_ld_library_path, overwrite);
    check_retval(r, "setenv PIN_VM64_LD_LIBRARY_PATH");

    /* Set Pin injector library paths */
    injector_32_ld_library_path = getenv("PIN_INJECTOR32_LD_LIBRARY_PATH");
    injector_64_ld_library_path = getenv("PIN_INJECTOR64_LD_LIBRARY_PATH");

    injector_32_ld_library_path = append3(xed_runtime_libs32, ":", injector_32_ld_library_path);
    injector_64_ld_library_path = append3(xed_runtime_libs64, ":", injector_64_ld_library_path);
    injector_32_ld_library_path = append3(pincrt_libs32, ":", injector_32_ld_library_path);
    injector_64_ld_library_path = append3(pincrt_libs64, ":", injector_64_ld_library_path);

    r = setenv("PIN_INJECTOR32_LD_LIBRARY_PATH", injector_32_ld_library_path, overwrite);
    check_retval(r, "setenv PIN_INJECTOR32_LD_LIBRARY_PATH");
    r = setenv("PIN_INJECTOR64_LD_LIBRARY_PATH", injector_64_ld_library_path, overwrite);
    check_retval(r, "setenv PIN_INJECTOR64_LD_LIBRARY_PATH_64");

    /* This variable tells the injector to restore environment variables after pin is injected. */
    r = setenv("PIN_LD_RESTORE_REQUIRED", "t", overwrite);
    check_retval(r, "setenv PIN_LD_RESTORE_REQUIRED");

    /*
     * Backup the LD_LIBRARY_PATH, since pin uses a different one while launching. It will be restored
     * when the app is loaded to memory.
     */
    ld_library_path = getenv("LD_LIBRARY_PATH");
    if (ld_library_path)
    {
        r = setenv("PIN_APP_LD_LIBRARY_PATH", ld_library_path, overwrite);
        check_retval(r, "setenv PIN_APP_LD_LIBRARY_PATH");
    }

    /* Overwrite LD_LIBRARY_PATH with the libraries required for pin to run. */
    r = setenv("LD_LIBRARY_PATH", (IsHostArch64Bit()) ? injector_64_ld_library_path : injector_32_ld_library_path, overwrite);
    check_retval(r, "setenv LD_LIBRARY_PATH");

    /*
     * If the LD_BIND_NOW, LD_ASSUME_KERNEL and LD_PRELOAD variables were defined they should pass as
     * is to the app. Since pin's injector doesn't need it, we save them now and restore it after
     * pin is injected into the process.
     */
    ld_assume_kernel = getenv("LD_ASSUME_KERNEL");
    if (ld_assume_kernel)
    {
        r = setenv("PIN_APP_LD_ASSUME_KERNEL", ld_assume_kernel, overwrite);
        check_retval(r, "setenv PIN_APP_LD_ASSUME_KERNEL");
        unsetenv("LD_ASSUME_KERNEL");
    }

    incoming_ld_bind_now = getenv("LD_BIND_NOW");
    if (incoming_ld_bind_now)
    {
        r = setenv("PIN_APP_LD_BIND_NOW", incoming_ld_bind_now, overwrite);
        check_retval(r, "setenv PIN_APP_LD_BIND_NOW");
        unsetenv("LD_BIND_NOW");
    }

    incoming_ld_preload = getenv("LD_PRELOAD");
    if (incoming_ld_preload)
    {
        r = setenv("PIN_APP_LD_PRELOAD", incoming_ld_preload, overwrite);
        check_retval(r, "setenv PIN_APP_LD_PRELOAD");
        unsetenv("LD_PRELOAD");
    }
}

char* find_driver_name(char* argv0)
{
    int chars;
    const char* proc_link = 0;
    char base_path[PATH_MAX];

    /* The path of the current running executable (self) under the procfs. */
    proc_link = "/proc/self/exe";

    if (access(proc_link, F_OK) != 0)
    {
        /* no /proc... */
        assert(strlen(argv0) < PATH_MAX);
        strcpy(base_path, argv0);
    }
    else
    {
        chars = readlink(proc_link, base_path, PATH_MAX);
        if (chars == -1)
        {
            perror("readlink:");
            exit(1);
        }
        base_path[chars] = 0; /* null terminate the string */
    }

    return strdup(base_path);
}

char** build_child_argv(char* base_path, int argc, char** argv, int user_argc,
        char** user_argv)
{
    char** child_argv = (char**) malloc(sizeof(char*) * (argc + user_argc + 4));
    int var = 0, user_arg = 0, child_argv_ind = 0;

    /*
        This is just to make sure both binaries actually exist. The program will exit if any of these calls fail.
      */
    find_driver_name(append3(base_path, "/", "ia32/bin/pinbin"));
    find_driver_name(append3(base_path, "/", "intel64/bin/pinbin"));

    /*
       Set the default pinbin to that of the architecture of the host.
       The reason is that a 64bit machine may not have 32bit glibc installed,
       in which case pin will fail to run if the 32bit pinbin
       version is used.
       If host is 64bit then start with 64bit pinbin. pinbin may later switch to the 32bit pinbin using the -p32 parameter.
       If host is 32bit then start with 32bit pinbin. pinbin may later switch to the 64bit pinbin using the -p64 parameter.
    */
    if (IsHostArch64Bit())
    {
        child_argv[child_argv_ind++] = append3(base_path, "/", "intel64/bin/pinbin");
        child_argv[child_argv_ind++] = "-p32";
        child_argv[child_argv_ind++] = append3(base_path, "/", "ia32/bin/pinbin");
    }
    else
    {
        child_argv[child_argv_ind++] = append3(base_path, "/", "ia32/bin/pinbin");
        child_argv[child_argv_ind++] = "-p64";
        child_argv[child_argv_ind++] = append3(base_path, "/", "intel64/bin/pinbin");
    }

    /* Add the user arguments */
    for (user_arg = 0; user_arg < user_argc; ++user_arg)
    {
        child_argv[child_argv_ind++] = user_argv[user_arg];
    }

    /* Copy original command line parameters. */
    for (var = 1; var < argc; ++var)
    {
        child_argv[child_argv_ind++] = argv[var];
    }

    /* Null terminate the array. */
    child_argv[child_argv_ind++] = NULL;

    /* Clean the user arguments memory */
    if (user_argv)
    {
        free(user_argv);
    }

    return child_argv;
}

