// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_WINDOWS_INTEL64_BARESYSCALL_H__
#define OS_APIS_WINDOWS_INTEL64_BARESYSCALL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define REG_SIZE                      HEX(8)
// 8 callee-saved registers
#define CALLEE_SAVED_REG              HEX(8)
// System call arguments stack offset
// (shadow stack (0x20) + return address (0x8))
#define SYSCALL_ARG_STACK_OFFSET      HEX(28)

#ifdef __cplusplus
}
#endif

#endif // file guard
