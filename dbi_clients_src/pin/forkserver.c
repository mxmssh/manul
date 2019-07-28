/*
 * forkserver for AFL.
 * The original source of file https://github.com/vanhauser-thc/afl-pin
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdint.h>
#include "afl/config.h"

#define PRINT_ERROR(string) (void)(write(2, string, strlen(string))+1)

int forkserver_initialized = 0;

void startForkServer() {
  if (forkserver_initialized != 0)
    return;
  forkserver_initialized = 1;

  uint8_t tmp[4];
  int32_t child_pid = 0;

#ifdef DEBUG
  PRINT_ERROR("starting forkserver\n");
#endif

  if (write(FORKSRV_FD + 1, tmp, 4) != 4) {
    PRINT_ERROR("Error writing fork server\n");
    _exit(1);
  }

  while (1) {
    uint32_t was_killed;
    int32_t status;
    
    if (read(FORKSRV_FD, &was_killed, 4) != 4) {
      PRINT_ERROR("Error reading fork server\n");
      _exit(1);
    }
    child_pid = fork();
    if (child_pid < 0) {
      PRINT_ERROR("Error fork\n");
      _exit(1);
    }
    if (child_pid == 0) { // child
      close(FORKSRV_FD);
      close(FORKSRV_FD + 1);
      return;
    }
    if (write(FORKSRV_FD + 1, &child_pid, 4) != 4) {
      PRINT_ERROR("Error writing fork server (2)\n");
      _exit(1);
    }
    if (waitpid(child_pid, &status, 0) < 0) {
      PRINT_ERROR("Error waiting for child\n");
      _exit(1);
    }
    if (write(FORKSRV_FD + 1, &status, 4) != 4) {
      PRINT_ERROR("Error writing fork server (3)\n");
      _exit(1);
    }
  }
}
