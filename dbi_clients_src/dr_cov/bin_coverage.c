/*
   Manul - DBI coverage file (winAFL port)
   -------------------------------------
   Maksim Shudrak <mshudrak@salesforce.com> <mxmssh@gmail.com>

   Copyright 2019 Salesforce.com, inc. All rights reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at:
     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
   
   Heavily based on Ivan Fratric's https://github.com/googleprojectzero/winafl
   Copyright 2019 Google Inc. All Rights Reserved.
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#define MAP_SIZE 65536

#include "dr_api.h"
#include "drmgr.h"
#include "drx.h"
#include "drreg.h"
#include "drwrap.h"
#include "drsyms.h"
#include "modules.h"
#include "utils.h"
#include "hashtable.h"
#include "drtable.h"
#include "dr_tools.h"
#include "limits.h"
//#include "config.h"
#include <string.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/shm.h>
#include <fcntl.h>
//#include <sys/stat.h>
//#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#else
#include <Windows.h>
#include <string.h>
#define strcasecmp _stricmp
#endif
static uint verbose;

#define COVERAGE_BB 0 // currently unsupported
#define COVERAGE_EDGE 1

#define NOTIFY(level, fmt, ...) do {          \
    if (verbose >= (level))                   \
        dr_fprintf(STDERR, fmt, __VA_ARGS__); \
} while (0)

#define OPTION_MAX_LENGTH MAXIMUM_PATH

typedef struct _target_module_t {
    char module_name[MAXIMUM_PATH];
    struct _target_module_t *next;
} target_module_t;

typedef struct _winafl_option_t {
    /* Use nudge to notify the process for termination so that
     * event_exit will be called.
     */
    bool nudge_kills;
    bool debug_mode;
    bool debug_manul;
    int coverage_kind;
    char logdir[MAXIMUM_PATH];
    target_module_t *target_modules;
    char fuzz_module[MAXIMUM_PATH];
    char fuzz_method[MAXIMUM_PATH];
    char ipc_obj_name[MAXIMUM_PATH];
    char shm_name[MAXIMUM_PATH];
    unsigned long fuzz_offset;
    int fuzz_iterations;
    void **func_args;
    int num_fuz_args;
    int persistence_mode;
    drwrap_callconv_t callconv;
    bool thread_coverage;
    bool no_loop;
} winafl_option_t;

typedef struct _debug_data_t {
    int pre_hanlder_called;
    int post_handler_called;
} debug_data_t;
static debug_data_t debug_data;

static winafl_option_t options;

#define NUM_THREAD_MODULE_CACHE 4

typedef struct _winafl_data_t {
    module_entry_t *cache[NUM_THREAD_MODULE_CACHE];
    file_t  log;
    unsigned char *fake_afl_area; //used for thread_coverage
    unsigned char *afl_area;
} winafl_data_t;
static winafl_data_t winafl_data;

static int winafl_tls_field;

#define ASSERT_WRAP(msg) do { \
   dr_fprintf(winafl_data.log, "%s", msg); \
   DR_ASSERT_MSG(false, msg); \
} while (0)

typedef struct _fuzz_target_t {
    reg_t xsp;            /* stack level at entry to the fuzz target */
    app_pc func_pc;
    int iteration;
} fuzz_target_t;
static fuzz_target_t fuzz_target;

static module_table_t *module_table;
static client_id_t client_id;

static volatile bool go_native;

static void
event_exit(void);

static void
event_thread_exit(void *drcontext);

/****************************************************************************
 * Nudges
 */

enum {
    NUDGE_TERMINATE_PROCESS = 1,
};

static void
event_nudge(void *drcontext, uint64 argument)
{
    int nudge_arg = (int)argument;
    int exit_arg  = (int)(argument >> 32);
    if (nudge_arg == NUDGE_TERMINATE_PROCESS) {
        static int nudge_term_count;
        /* handle multiple from both NtTerminateProcess and NtTerminateJobObject */
        uint count = dr_atomic_add32_return_sum(&nudge_term_count, 1);
        if (count == 1) {
            dr_exit_process(exit_arg);
        }
    }
    ASSERT(nudge_arg == NUDGE_TERMINATE_PROCESS, "unsupported nudge");
    ASSERT(false, "should not reach"); /* should not reach */
}

static bool
event_soft_kill(process_id_t pid, int exit_code)
{
    /* we pass [exit_code, NUDGE_TERMINATE_PROCESS] to target process */
    dr_config_status_t res;
    res = dr_nudge_client_ex(pid, client_id,
                             NUDGE_TERMINATE_PROCESS | (uint64)exit_code << 32,
                             0);
    if (res == DR_SUCCESS) {
        /* skip syscall since target will terminate itself */
        return true;
    }
    /* else failed b/c target not under DR control or maybe some other
     * error: let syscall go through
     */
    return false;
}


static dr_emit_flags_t
instrument_edge_coverage(void *drcontext, void *tag, instrlist_t *bb, instr_t *inst,
                      bool for_trace, bool translating, void *user_data)
{

    app_pc start_pc;
    module_entry_t **mod_entry_cache;
    module_entry_t *mod_entry;
    reg_id_t reg, reg2, reg3;
    opnd_t opnd1, opnd2;
    instr_t *new_instr;
    const char *module_name;
    uint offset;
    target_module_t *target_modules;
    bool should_instrument;
    if (!drmgr_is_first_instr(drcontext, inst))
        return DR_EMIT_DEFAULT;

    start_pc = dr_fragment_app_pc(tag);

    mod_entry_cache = winafl_data.cache;
    mod_entry = module_table_lookup(mod_entry_cache, NUM_THREAD_MODULE_CACHE, module_table, start_pc);

     if (mod_entry == NULL || mod_entry->data == NULL) return DR_EMIT_DEFAULT;

    module_name = dr_module_preferred_name(mod_entry->data);
    should_instrument = false;
    target_modules = options.target_modules;
    while(target_modules) {
        if(strcasecmp(module_name, target_modules->module_name) == 0) {
            should_instrument = true;
            if(options.debug_mode || options.debug_manul)
                dr_fprintf(winafl_data.log, "Instrumenting %s with the 'edge' mode\n", module_name);
            break;
        }
        target_modules = target_modules->next;
    }
    if(!should_instrument) return DR_EMIT_DEFAULT;

    offset = (uint)(start_pc - mod_entry->data->start);
    offset &= MAP_SIZE - 1;

    drreg_reserve_aflags(drcontext, bb, inst);
    drreg_reserve_register(drcontext, bb, inst, NULL, &reg);
    drreg_reserve_register(drcontext, bb, inst, NULL, &reg2);
    drreg_reserve_register(drcontext, bb, inst, NULL, &reg3);

    //reg2 stores AFL area, reg 3 stores previous offset

    //load the pointer to previous offset in reg3
    drmgr_insert_read_tls_field(drcontext, winafl_tls_field, bb, inst, reg3);

    //load address of shm into reg2
    if(options.thread_coverage) { /* TODO: enable and test in future */
      opnd1 = opnd_create_reg(reg2);
      opnd2 = OPND_CREATE_MEMPTR(reg3, sizeof(void *));
      new_instr = INSTR_CREATE_mov_ld(drcontext, opnd1, opnd2);
      instrlist_meta_preinsert(bb, inst, new_instr);
    } else {
      opnd1 = opnd_create_reg(reg2);
      opnd2 = OPND_CREATE_INTPTR(winafl_data.afl_area);
      new_instr = INSTR_CREATE_mov_imm(drcontext, opnd1, opnd2);
      instrlist_meta_preinsert(bb, inst, new_instr);
    }

    //load previous offset into register
    opnd1 = opnd_create_reg(reg);
    opnd2 = OPND_CREATE_MEMPTR(reg3, 0);
    new_instr = INSTR_CREATE_mov_ld(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(bb, inst, new_instr);

    //xor register with the new offset
    opnd1 = opnd_create_reg(reg);
    opnd2 = OPND_CREATE_INT32(offset);
    new_instr = INSTR_CREATE_xor(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(bb, inst, new_instr);

    //increase the counter at reg + reg2
    opnd1 = opnd_create_base_disp(reg2, reg, 1, 0, OPSZ_1);
    new_instr = INSTR_CREATE_inc(drcontext, opnd1);
    instrlist_meta_preinsert(bb, inst, new_instr);

    //store the new value
    offset = (offset >> 1)&(MAP_SIZE - 1);
    opnd1 = OPND_CREATE_MEMPTR(reg3, 0);
    opnd2 = OPND_CREATE_INT32(offset);
    new_instr = INSTR_CREATE_mov_st(drcontext, opnd1, opnd2);
    instrlist_meta_preinsert(bb, inst, new_instr);

    drreg_unreserve_register(drcontext, bb, inst, reg3);
    drreg_unreserve_register(drcontext, bb, inst, reg2);
    drreg_unreserve_register(drcontext, bb, inst, reg);
    drreg_unreserve_aflags(drcontext, bb, inst);
    if(options.debug_mode || options.debug_manul)
        dr_fprintf(winafl_data.log, "Done\n");
    return DR_EMIT_DEFAULT;
}

#ifdef _WIN32
static void
setup_shmem() {
    HANDLE map_file;
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Setting up shared memory\n");
    const char *shm_str = getenv("__AFL_SHM_ID");
    if (shm_str == NULL) ASSERT_WRAP("error accessing __AFL_SHM_ID");

    map_file = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shm_str);
    if (map_file == NULL) ASSERT_WRAP("error accessing shared memory");

    winafl_data.afl_area = (unsigned char *)MapViewOfFile(map_file, // handle to map object
        FILE_MAP_ALL_ACCESS,  // read/write permission
        0,
        0,
        MAP_SIZE);
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Done\n");
    if (winafl_data.afl_area == NULL) ASSERT_WRAP("error accessing shared memory");
}

static HANDLE pipe;

static void
setup_object() {
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Setting up the PIPE %s\n",
                   options.ipc_obj_name);
    pipe = CreateFile(
         options.ipc_obj_name,   // pipe name in and out are the same
         GENERIC_READ |  // read and write access
         GENERIC_WRITE,
         0,              // no sharing
         NULL,           // default security attributes
         OPEN_EXISTING,  // opens existing pipe
         0,              // default attributes
         NULL);          // no template file
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Done\n");
    if (pipe == INVALID_HANDLE_VALUE) ASSERT_WRAP("error connecting to pipe");
}

static char
ReadCommandFromObject()
{
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Reading from PIPE\n");
    DWORD num_read;
    char result;
    ReadFile(pipe, &result, 1, &num_read, NULL);
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Done, result: %c\n", result);
    return result;
}

static void
WriteCommandToObject(char cmd)
{
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Writing %c in PIPE\n", cmd);
    DWORD num_written;
    WriteFile(pipe, &cmd, 1, &num_written, NULL);
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Done\n");
}

static bool
onexception(void *drcontext, dr_exception_t *excpt) {
    DWORD exception_code = excpt->record->ExceptionCode;

    if(options.debug_mode)
        dr_fprintf(winafl_data.log, "Exception caught: %x\n", exception_code);

    if((exception_code == EXCEPTION_ACCESS_VIOLATION) ||
        (exception_code == EXCEPTION_ILLEGAL_INSTRUCTION) ||
        (exception_code == EXCEPTION_PRIV_INSTRUCTION) ||
        (exception_code == EXCEPTION_INT_DIVIDE_BY_ZERO) ||
        (exception_code == STATUS_HEAP_CORRUPTION) ||
        (exception_code == EXCEPTION_STACK_OVERFLOW) ||
        (exception_code == STATUS_STACK_BUFFER_OVERRUN) ||
        (exception_code == STATUS_FATAL_APP_EXIT)) {
        if(options.debug_mode) {
            dr_fprintf(winafl_data.log, "crashed\n");
        } else {
            WriteCommandToObject('C');
        }
        dr_exit_process(1);
    }
    return true;
}

#else

int socket_fd;

static void
setup_object() {
    struct sockaddr_un addr;

    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Setting up socket %s\n", options.ipc_obj_name);

    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) ASSERT_WRAP("Call to socket(AF_UNIX, ...) failed");

    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Connecting to socket %d\n", socket_fd);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, options.ipc_obj_name, sizeof(addr.sun_path)-1);
    if (connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) ASSERT_WRAP("error connecting to socket");

    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Done");
}

static void
close_socket() {
    close(socket_fd);
}

static void
setup_shmem() {
   if (options.debug_manul)
       dr_fprintf(winafl_data.log, "Setting up shared memory\n");
   const char *shm_str = getenv("__AFL_SHM_ID");
   if (shm_str == NULL) ASSERT_WRAP("error accessing AFL_SHM_ENV");
   int shm_id = atoi(shm_str);
   if (shm_id < 0) ASSERT_WRAP("error converting AFL_SHM_ENV");
   winafl_data.afl_area = (unsigned char *)shmat(shm_id, 0, 0);
   if (options.debug_manul)
       dr_fprintf(winafl_data.log, "Done\n");
   if (winafl_data.afl_area == (void *)-1) ASSERT_WRAP("error accessing shared memory");
}

static char
ReadCommandFromObject() {
    char cmd[1];
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Reading from UDS %d \n", socket_fd);
    read(socket_fd, cmd, 1);
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Result %c \n", cmd[0]);
    return cmd[0];
}

static void
WriteCommandToObject(char cmd) {
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Writing %c in UDS %d\n", cmd, socket_fd);
    write(socket_fd, &cmd, 1);
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "Done writing\n");
}

#endif

static void
pre_loop_start_handler(void *wrapcxt, INOUT void **user_data)
{
    void *drcontext = drwrap_get_drcontext(wrapcxt);

    if (!options.debug_mode) {
        if (options.debug_manul)
            dr_fprintf(winafl_data.log, "In pre_loop_start_handler\n");
        //let server know we finished a cycle, redundant on first cycle.
        WriteCommandToObject('K');
        if (fuzz_target.iteration == options.fuzz_iterations)
            dr_exit_process(0);

        fuzz_target.iteration++;

        //let server know we are starting a new cycle
        WriteCommandToObject('P');

        //wait for server acknowledgement for cycle start
        char command = ReadCommandFromObject();

        if (command != 'F') {
            if (command == 'Q')
                dr_exit_process(0);
            else {
                dr_fprintf(winafl_data.log, "unrecognized command received over pipe");
                dr_exit_process(1);
            }
        }
    }
    else {
        debug_data.pre_hanlder_called++;
        dr_fprintf(winafl_data.log, "In pre_loop_start_handler: %d\n",
                debug_data.pre_hanlder_called);
    }

    memset(winafl_data.afl_area, 0, MAP_SIZE);

    if (options.coverage_kind == COVERAGE_EDGE || options.thread_coverage) {
        void **thread_data = (void **)drmgr_get_tls_field(drcontext, winafl_tls_field);
        thread_data[0] = 0;
        thread_data[1] = winafl_data.afl_area;
    }
}

static void
pre_fuzz_handler(void *wrapcxt, INOUT void **user_data)
{
    char command = 0;
    int i;
    void *drcontext;

    app_pc target_to_fuzz = drwrap_get_func(wrapcxt);
    dr_mcontext_t *mc = drwrap_get_mcontext_ex(wrapcxt, DR_MC_ALL);
    drcontext = drwrap_get_drcontext(wrapcxt);

    fuzz_target.xsp = mc->xsp;
    fuzz_target.func_pc = target_to_fuzz;

    if(!options.debug_mode) {
        if (options.debug_manul)
            dr_fprintf(winafl_data.log, "In pre_fuzz_handler\n");
        WriteCommandToObject('P');
        command = ReadCommandFromObject();

        if(command != 'F') {
            if(command == 'Q')
                dr_exit_process(0);
            else {
                dr_fprintf(winafl_data.log, "unrecognized command received over pipe");
                dr_exit_process(1);
            }
        }

    } else {
        debug_data.pre_hanlder_called++;
        dr_fprintf(winafl_data.log, "In pre_fuzz_handler\n");
    }

    //save or restore arguments
    if (!options.no_loop) {
        if (fuzz_target.iteration == 0) {
            for (i = 0; i < options.num_fuz_args; i++)
                options.func_args[i] = drwrap_get_arg(wrapcxt, i);
        } else {
            for (i = 0; i < options.num_fuz_args; i++)
                drwrap_set_arg(wrapcxt, i, options.func_args[i]);
        }
    }

    memset(winafl_data.afl_area, 0, MAP_SIZE);

    if(options.coverage_kind == COVERAGE_EDGE || options.thread_coverage) {
        void **thread_data = (void **)drmgr_get_tls_field(drcontext, winafl_tls_field);
        thread_data[0] = 0;
        thread_data[1] = winafl_data.afl_area;
    }
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "done pre_fuzz_handler\n");
}

static void
post_fuzz_handler(void *wrapcxt, void *user_data)
{
    dr_mcontext_t *mc;
    mc = drwrap_get_mcontext(wrapcxt);

    if(!options.debug_mode) {
        if (options.debug_manul)
            dr_fprintf(winafl_data.log, "In post_fuzz_handler\n");
        WriteCommandToObject('K');
    } else {
        debug_data.post_handler_called++;
        dr_fprintf(winafl_data.log, "In post_fuzz_handler\n");
    }

    /* We don't need to reload context in case of network-based fuzzing. */
    if (options.no_loop)
        return;

    fuzz_target.iteration++;
    if(fuzz_target.iteration == options.fuzz_iterations) {
        if (options.debug_manul)
            dr_fprintf(winafl_data.log, "Target iteration exceeds limit, exiting the target\n");
        WriteCommandToObject('Q');
        dr_exit_process(0);
    }

    mc->xsp = fuzz_target.xsp;
    mc->pc = fuzz_target.func_pc;
    drwrap_redirect_execution(wrapcxt);
    if (options.debug_manul)
        dr_fprintf(winafl_data.log, "done post_fuzz_handler\n");
}

static void
event_module_unload(void *drcontext, const module_data_t *info)
{
    module_table_unload(module_table, info);
}

static void
event_module_load(void *drcontext, const module_data_t *info, bool loaded)
{
    const char *module_name = dr_module_preferred_name(info);
    app_pc to_wrap = 0;

    if(options.debug_mode || options.debug_manul)
        dr_fprintf(winafl_data.log, "Module loaded, %s\n", module_name);

    if(options.fuzz_module[0]) {
        if (options.debug_mode || options.debug_manul)
            dr_fprintf(winafl_data.log, "Comparing %s %s\n", module_name, options.fuzz_module);
        if (strcasecmp(module_name, options.fuzz_module) == 0) {
            if(options.debug_mode || options.debug_manul)
                dr_fprintf(winafl_data.log, "Found target module %s\n", options.fuzz_module);
            if(options.fuzz_offset) {
                to_wrap = info->start + options.fuzz_offset;
            } else {
                //first try exported symbols
                if (options.debug_mode || options.debug_manul)
                    dr_fprintf(winafl_data.log, "Wrapping %s\n", options.fuzz_method);
                to_wrap = (app_pc)dr_get_proc_address(info->handle, options.fuzz_method);
                if(!to_wrap) {
                    //if that fails, try with the symbol access library
#ifdef USE_DRSYMS
                    drsym_init(0);
                    drsym_lookup_symbol(info->full_path, options.fuzz_method, (size_t *)(&to_wrap), 0);
                    drsym_exit();
#endif
                    ASSERT_WRAP("Can't find specified method in fuzz_module");
                    to_wrap += (size_t)info->start;
                }
            }
            if (options.persistence_mode == 1) // # func wrap mode
                if (!drwrap_wrap_ex(to_wrap, pre_fuzz_handler, post_fuzz_handler,
                                    NULL, options.callconv))
                    dr_fprintf(winafl_data.log, "Warning! Failed to wrap target function\n");
            if (options.persistence_mode == 2) // # in_app
                if (!drwrap_wrap_ex(to_wrap, pre_loop_start_handler,
                                    NULL, NULL, options.callconv))
                    dr_fprintf(winafl_data.log, "Warning! Failed to wrap target function\n");
        }
    }

    module_table_load(module_table, info);
}

static void
event_exit(void)
{
    if(options.debug_mode || options.debug_manul) {
        //dump_winafl_data();
        dr_close_file(winafl_data.log);
    }

    /* destroy module table */
    module_table_destroy(module_table);

    drx_exit();
    drmgr_exit();
}

static void
event_init()
{
    char buf[MAXIMUM_PATH];

    module_table = module_table_create();

    memset(winafl_data.cache, 0, sizeof(winafl_data.cache));

    if(options.debug_mode || options.debug_manul) {
        winafl_data.log =
            drx_open_unique_appid_file(options.logdir, dr_get_process_id(),
                                   "afl", "proc.log",
                                   DR_FILE_ALLOW_LARGE,
                                   buf, BUFFER_SIZE_ELEMENTS(buf));
        if (winafl_data.log != INVALID_FILE) {
            dr_log(NULL, LOG_ALL, 1, "winafl: log file is %s\n", buf);
            NOTIFY(1, "<created log file %s>\n", buf);
        }
    }

    fuzz_target.iteration = 0;
}

static void event_thread_init(void *drcontext)
{
  void **thread_data;

  thread_data = (void **)dr_thread_alloc(drcontext, 2 * sizeof(void *));
  thread_data[0] = 0;
  thread_data[1] = 0;

  drmgr_set_tls_field(drcontext, winafl_tls_field, thread_data);
}

static void event_thread_exit(void *drcontext)
{
  void *data = drmgr_get_tls_field(drcontext, winafl_tls_field);
  dr_thread_free(drcontext, data, 2 * sizeof(void *));
}

static void
options_init(client_id_t id, int argc, const char *argv[])
{
    int i;
    const char *token;
    target_module_t *target_modules;
    /* default values */
    options.nudge_kills = true;
    options.debug_mode = false;
    options.debug_manul = false;
    options.target_modules = NULL;
    options.coverage_kind = COVERAGE_EDGE;
    options.persistence_mode = 0;
    options.fuzz_offset = 0;
    options.fuzz_iterations = 5000;
    options.no_loop = false;
    options.func_args = NULL;
    options.num_fuz_args = 0;
    options.thread_coverage = false;
    options.callconv = DRWRAP_CALLCONV_DEFAULT;

    dr_snprintf(options.logdir, BUFFER_SIZE_ELEMENTS(options.logdir), ".");

    for (i = 1/*skip client*/; i < argc; i++) {
        token = argv[i];
        if (strcmp(token, "-no_nudge_kills") == 0)
            options.nudge_kills = false;
        else if (strcmp(token, "-debug") == 0)
            options.debug_mode = true;
        else if (strcmp(token, "-debug_manul") == 0)
            options.debug_manul = true;
        else if (strcmp(token, "-thread_coverage") == 0)
            options.thread_coverage = true;
        else if (strcmp(token, "-logdir") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing logdir path");
            strncpy(options.logdir, argv[++i], BUFFER_SIZE_ELEMENTS(options.logdir));
        }
        else if (strcmp(token, "-ipc_obj_name") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing pipe path IN name");
            strcat(options.ipc_obj_name, argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-target_method") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing method");
            strncpy(options.fuzz_method, argv[++i], BUFFER_SIZE_ELEMENTS(options.fuzz_method));
        }
        else if (strcmp(token, "-target_offset") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing offset");
            options.fuzz_offset = strtoul(argv[++i], NULL, 0);
        }
        else if (strcmp(token, "-fuzz_iterations") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing number of iterations");
            options.fuzz_iterations = atoi(argv[++i]);
        }
        else if (strcmp(token, "-persistence_mode") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing persistence mode");
            options.persistence_mode = atoi(argv[++i]);
        }
        else if (strcmp(token, "-coverage_module") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing module");
            target_modules = options.target_modules;
            options.target_modules = (target_module_t *)dr_global_alloc(sizeof(target_module_t));
            options.target_modules->next = target_modules;
            strncpy(options.target_modules->module_name, argv[++i], BUFFER_SIZE_ELEMENTS(options.target_modules->module_name));
            if (options.debug_mode)
                dr_fprintf(winafl_data.log, "coverage module: %s\n", options.target_modules->module_name);
        }
        else if (strcmp(token, "-verbose") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing -verbose number");
            token = argv[++i];
            if (dr_sscanf(token, "%u", &verbose) != 1) {
                USAGE_CHECK(false, "invalid -verbose number");
            }
        }
        else if (strcmp(token, "-target_module") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing module");
            strncpy(options.fuzz_module, argv[++i], BUFFER_SIZE_ELEMENTS(options.fuzz_module));
        }
        else {
            NOTIFY(0, "UNRECOGNIZED OPTION: \"%s\"\n", token);
            USAGE_CHECK(false, "invalid option");
        }
    }
    if (options.persistence_mode == 2)
        options.no_loop = true;
    /* TODO: check that options are correctly set */
}

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    drreg_options_t ops = {sizeof(ops), 2 /*max slots needed: aflags*/, false};

    dr_set_client_name("Manul DBI (winAFL port) library", "");

    drmgr_init();
    drx_init();
    drreg_init(&ops);
    drwrap_init();

    options_init(id, argc, argv);
    
    event_init();

    dr_register_exit_event(event_exit);
#ifdef _WIN32
    drmgr_register_exception_event(onexception);
#endif
    drmgr_register_bb_instrumentation_event(NULL, instrument_edge_coverage, NULL);

    drmgr_register_module_load_event(event_module_load);
    drmgr_register_module_unload_event(event_module_unload);
    dr_register_nudge_event(event_nudge, id);

    client_id = id;

    if (options.nudge_kills)
        drx_register_soft_kills(event_soft_kill);

    /* we have to grab shared memory from the fuzzer (alread installed) */
    if (!options.debug_mode) {
        setup_shmem();
        /* we have to establish connection via PIPEs in persistence mode */
        if (options.persistence_mode > 0)
            setup_object();
    } else {
        winafl_data.afl_area = (unsigned char *)dr_global_alloc(MAP_SIZE);
    }

    winafl_tls_field = drmgr_register_tls_field();
    if(winafl_tls_field == -1) ASSERT_WRAP("error reserving TLS field");
    
    drmgr_register_thread_init_event(event_thread_init);
    drmgr_register_thread_exit_event(event_thread_exit);

}
