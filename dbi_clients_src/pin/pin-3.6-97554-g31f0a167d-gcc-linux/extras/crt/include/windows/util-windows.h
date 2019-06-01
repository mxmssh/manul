// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_UITL_WINDOWS_H_
#define OS_APIS_UITL_WINDOWS_H_

#include "os-apis.h"
#include "types.h"
#include "baresyscall/baresyscall.h"
#include "win_syscalls.h"

void OS_SetSysCallTable(SYSCALL_NUMBER_T *input);

void OS_SetIfItIsWow64();

#endif // file guard
