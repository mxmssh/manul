/*
 The original source of file https://github.com/vanhauser-thc/afl-pin
*/

#include "pin.H"
#include <asm/unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <sys/types.h>
#include <sys/shm.h>
#include "afl/config.h"

#if PIN_PRODUCT_VERSION_MAJOR < 3
 #if PIN_PRODUCT_VERSION_MINOR < 6
  #warn "WARNING: you should use pintool >= 3.6!"
 #endif
#endif

KNOB < string > KnobLibs(KNOB_MODE_WRITEONCE, "pintool", "libs", "", "also report basic bocks of dynamic libraries");
//#define DEBUG
//#define DEBUG_PRINT

typedef uint16_t map_t;
static map_t prev_id;
#ifndef DEBUG
static uint8_t *trace_bits = NULL;
#else
static uint8_t trace_bits[65535] = {0};
#endif

static int libs = 0;
static vector<string> libs_to_instrument;

/* ===================================================================== */
/* Usage                                                                 */
/* ===================================================================== */

INT32 Usage() {
  cout << "afl-pin v 0.1 " << endl;
  cout << "=====================================================================" << endl;
  cout << " -libs         also instrument basic blocks of certain dynamic libraries" << endl;
  return -1;
}

static VOID PIN_FAST_ANALYSIS_CALL bbreport(ADDRINT addr) {
  map_t id = (map_t)(((uintptr_t)addr) >> 1);
#ifdef DEBUG_PRINT
  cerr << "BB: 0x" << hex << addr << " and id 0x" << (prev_id ^ id) << endl;;
#endif

  trace_bits[prev_id ^ id]++;
  prev_id = id >> 1;
}


static VOID Trace(TRACE trace, VOID *v)
{
    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        bool instrument = FALSE;
        PIN_LockClient();
        IMG img = IMG_FindByAddress(BBL_Address(bbl));
        if (!IMG_Valid(img)) {
          continue;
          }
        string module_name = IMG_Name(img);
        PIN_UnlockClient();
        transform(module_name.begin(), module_name.end(), module_name.begin(), ::tolower);
        if (IMG_IsMainExecutable(img))
            instrument = TRUE;
        else if (libs) {
          for (auto const& libs: libs_to_instrument) {
              if (module_name.find(libs) != string::npos) {
                  instrument = TRUE;
                  break;
              }
          }
        }

        if (!instrument)
          continue;
        ADDRINT image_base = IMG_LowAddress(img);
#ifdef DEBUG_PRINT
        cerr << "instrumenting " << module_name << image_base << endl;
#endif

        BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)bbreport, IARG_FAST_ANALYSIS_CALL,
                       IARG_ADDRINT, BBL_Address(bbl) - image_base, IARG_END);
    }
}

VOID fini(INT32 code, VOID * v) {
#ifdef DEBUG
  cerr << "DEBUG: END OF PROGRAM" << endl;
#endif
  return;
}

/* ===================================================================== */
/* MAIN function                                                         */
/* ===================================================================== */

int main(int argc, char *argv[]) {
#ifndef DEBUG
  char *shmenv;
  int shm_id;
#endif

  if (PIN_Init(argc, argv))
    return Usage();
  PIN_SetSyntaxIntel();

#ifndef DEBUG
  if ((shmenv = getenv(SHM_ENV_VAR)) == NULL) {
    fprintf(stderr, "Error: AFL environment variable " SHM_ENV_VAR " not set\n");
    exit(-1);
  }
  if ((shm_id = atoi(shmenv)) < 0) {
    fprintf(stderr, "Error: invalid " SHM_ENV_VAR " contents\n");
    exit(-1);
  }
  if ((trace_bits = (u8 *) shmat(shm_id, NULL, 0)) == (void*) -1 || trace_bits == NULL) {
    fprintf(stderr, "Error: " SHM_ENV_VAR " attach failed\n");
    exit(-1);
  }
  /* TODO: support fork server
  if (fcntl(FORKSRV_FD, F_GETFL) == -1 || fcntl(FORKSRV_FD + 1, F_GETFL) == -1) {
    fprintf(stderr, "Error: AFL fork server file descriptors are not open\n");
    exit(-1);
  }*/
#endif

  /* Open config and read string, TODO: check if exist */
  string config_path = KnobLibs.Value();
  //string content;
  if (!config_path.empty()) {

    std::ifstream t(config_path.c_str());
    if(t.fail()) {
      cerr << "Failed to open config specified" << endl;
      return -1;
    }

    libs = 1;
    string s;
    while (getline(t, s, ',')) {
      s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
      libs_to_instrument.push_back(s);
    }
  }

  PIN_AddFiniFunction(fini, 0);
  TRACE_AddInstrumentFunction(Trace, 0);


  PIN_StartProgram();
  return 0;
}
