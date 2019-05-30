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
#else
#include <Windows.h>
#include <string.h>
#define strcasecmp _stricmp
#endif
static uint verbose;

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
	int persistence_mode;
    int coverage_kind;
    char logdir[MAXIMUM_PATH];
    target_module_t *target_modules;
    char fuzz_module[MAXIMUM_PATH];
    char fuzz_method[MAXIMUM_PATH];
    char pipe_name[MAXIMUM_PATH];
    char shm_name[MAXIMUM_PATH];
    unsigned long fuzz_offset;
    int fuzz_iterations;
    void **func_args;
    int num_fuz_args;
    drwrap_callconv_t callconv;
    bool thread_coverage;
    bool no_loop;
} winafl_option_t;
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
    mod_entry = module_table_lookup(mod_entry_cache,
                                                NUM_THREAD_MODULE_CACHE,
                                                module_table, start_pc);

     if (mod_entry == NULL || mod_entry->data == NULL) return DR_EMIT_DEFAULT;

    module_name = dr_module_preferred_name(mod_entry->data);

    should_instrument = false;
    target_modules = options.target_modules;
    while(target_modules) {
        if(strcasecmp(module_name, target_modules->module_name) == 0) {
            should_instrument = true;
            if(options.debug_mode)
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
    if(options.thread_coverage) {
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

    return DR_EMIT_DEFAULT;
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

    if (module_name == NULL) {
        // In case exe_name is not defined, we will fall back on the preferred name.
        module_name = dr_module_preferred_name(info);
    }

    if(options.debug_mode)
        dr_fprintf(winafl_data.log, "Module loaded, %s\n", module_name);

    module_table_load(module_table, info);
}

static void
event_exit(void)
{
    if(options.debug_mode) {
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

    if(options.debug_mode) {
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

#ifdef _WIN32
static void
setup_shmem() {
    HANDLE map_file;

    const char *shm_str = getenv("__AFL_SHM_ID");
    if (shm_str == NULL) ASSERT_WRAP("error accessing __AFL_SHM_ID");

    map_file = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shm_str);
    if (map_file == NULL) ASSERT_WRAP("error accesing shared memory");

    winafl_data.afl_area = (unsigned char *)MapViewOfFile(map_file, // handle to map object
        FILE_MAP_ALL_ACCESS,  // read/write permission
        0,
        0,
        MAP_SIZE);

    if (winafl_data.afl_area == NULL) ASSERT_WRAP("error accesing shared memory");
}
#else
static void
setup_shmem() {
   const char *shm_str = getenv("__AFL_SHM_ID");
   if (shm_str == NULL) ASSERT_WRAP("error accessing AFL_SHM_ENV");
   int shm_id = atoi(shm_str);
   if (shm_id < 0) ASSERT_WRAP("error converting AFL_SHM_ENV");
   winafl_data.afl_area = (unsigned char *)shmat(shm_id, 0, 0);

   if (winafl_data.afl_area == (void *)-1) ASSERT_WRAP("error accessing shared memory");
}
#endif

static void
options_init(client_id_t id, int argc, const char *argv[])
{
    int i;
    const char *token;
    target_module_t *target_modules;
    /* default values */
    options.nudge_kills = true;
    options.debug_mode = false;
    options.target_modules = NULL;
    options.fuzz_offset = 0;
    options.fuzz_iterations = 1000;
    options.no_loop = false;
    options.func_args = NULL;
    options.num_fuz_args = 0;
    options.callconv = DRWRAP_CALLCONV_DEFAULT;
    dr_snprintf(options.logdir, BUFFER_SIZE_ELEMENTS(options.logdir), ".");

    for (i = 1/*skip client*/; i < argc; i++) {
        token = argv[i];
        if (strcmp(token, "-no_nudge_kills") == 0)
            options.nudge_kills = false;
        else if (strcmp(token, "-debug") == 0)
            options.debug_mode = true;
        else if (strcmp(token, "-logdir") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing logdir path");
            strncpy(options.logdir, argv[++i], BUFFER_SIZE_ELEMENTS(options.logdir));
        }
        else if (strcmp(token, "-fuzzer_id") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing fuzzer id");
            strcpy(options.pipe_name, "\\\\.\\pipe\\afl_pipe_");
            strcpy(options.shm_name, "afl_shm_");
            strcat(options.pipe_name, argv[i+1]);
            strcat(options.shm_name, argv[i+1]);
            i++;
        }
        else if (strcmp(token, "-coverage_module") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing module");
            target_modules = options.target_modules;
            options.target_modules = (target_module_t *)dr_global_alloc(sizeof(target_module_t));
            options.target_modules->next = target_modules;
            strncpy(options.target_modules->module_name, argv[++i], BUFFER_SIZE_ELEMENTS(options.target_modules->module_name));
            dr_fprintf(winafl_data.log, "target module: %s\n", options.target_modules->module_name);
        }
        else if (strcmp(token, "-verbose") == 0) {
            USAGE_CHECK((i + 1) < argc, "missing -verbose number");
            token = argv[++i];
            if (dr_sscanf(token, "%u", &verbose) != 1) {
                USAGE_CHECK(false, "invalid -verbose number");
            }
        }
        else {
            NOTIFY(0, "UNRECOGNIZED OPTION: \"%s\"\n", token);
            USAGE_CHECK(false, "invalid option");
        }
    }
}

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    drreg_options_t ops = {sizeof(ops), 2 /*max slots needed: aflags*/, false};

    dr_set_client_name("AFL", "");

    drmgr_init();
    drx_init();
    drreg_init(&ops);
    drwrap_init();

    options_init(id, argc, argv);
    
    event_init();

    dr_register_exit_event(event_exit);

    drmgr_register_bb_instrumentation_event(NULL, instrument_edge_coverage, NULL);

    drmgr_register_module_load_event(event_module_load);
    drmgr_register_module_unload_event(event_module_unload);
    dr_register_nudge_event(event_nudge, id);

    client_id = id;

    if (options.nudge_kills)
        drx_register_soft_kills(event_soft_kill);

    /* we have to grab shared memory from AFL (alread installed) */
    setup_shmem();

    winafl_tls_field = drmgr_register_tls_field();
    if(winafl_tls_field == -1) DR_ASSERT_MSG(false, "error reserving TLS field");
    
    drmgr_register_thread_init_event(event_thread_init);
    drmgr_register_thread_exit_event(event_thread_exit);


}
