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
 * Launcher code common to unix platforms.
 *
 * To modify the launcher to launch a specific tool, use the auxiliary function
 * build_user_argv. This function should return an array of the tool specific arguments
 * and their count. These arguments will be added to the pin command line.
 *
 * An example of adding a tool name and its 64bit version is shown in the comment inside
 * build_user_argv.
 *
 */

#include "os_specific.h"

static char** build_user_argv(int* argc)
{
    char** argv = NULL;
    /* Usage Example:
     ====================================================
     *argc = 4; // Number of user defined arguments

     argv = (char**) malloc(sizeof(char*) * (*argc));

     argv[0] = "-t";
     argv[1] = append3("path_to_tool", "/", "toolname32");
     argv[2] = "-t64";
     argv[3] = append3("path_to_tool", "/", "toolname64");
     */

    return argv;
}

/* For testing purposes only */
#if 0
static void check_environment()
{
    char* s;
    int i;
    const char* array[] = {
            "LD_LIBRARY_PATH",
            "PIN_VM32_LD_LIBRARY_PATH",
            "PIN_VM64_LD_LIBRARY_PATH",
            "PIN_LD_RESTORE_REQUIRED",
            "PIN_APP_LD_ASSUME_KERNEL",
            "PIN_APP_LD_LIBRARY_PATH",
            "PIN_APP_LD_PRELOAD",
            0
    };
    for (i = 0; array[i] != NULL; i++)
    {
        s = getenv(array[i]);
        printf("env[%s] = %s\n", array[i], s);
    }
}

/*!
 * Prints the command line arguments.
 * @param child_argv Command line arguments array. Must be null terminated.
 */
static void print_argv_chunks(char** child_argv)
{
    char** p = child_argv;
    unsigned int i = 0;
    printf("\n");
    while (*p)
    {
        printf("argv[%d] = [%s]\n", i, *p);
        p++;
        i++;
    }
}
#endif

static void update_environment_common(char* base_path)
{
#ifdef PIN_CRT
    char buf[PATH_MAX];
    if (NULL != realpath(base_path, buf))
    {
        strcat(buf, "/extras/crt/tzdata");
        setenv("PIN_CRT_TZDATA", buf, 1);
    }
#endif
    update_environment(base_path);
}

/* Pin launcher which runs pinbin */
int main(int orig_argc, char** orig_argv)
{
    char* path_to_cmd;
    char **child_argv, **user_argv;
    int user_argc = 0;
    char* base_path;
    char* driver_name;

    driver_name = find_driver_name(orig_argv[0]);
    base_path = find_base_path(driver_name);

    update_environment_common(base_path);

    user_argv = build_user_argv(&user_argc);
    child_argv = build_child_argv(base_path, orig_argc, orig_argv, user_argc,
            user_argv);
    path_to_cmd = child_argv[0];

    /* For testing purposes */
#if 0
     check_environment();
     print_argv_chunks(child_argv);
#endif

    return execv(path_to_cmd, child_argv);
}
