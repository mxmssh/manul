// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_MAC_IA32_BARESYSCALL_H__
#define OS_APIS_MAC_IA32_BARESYSCALL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/*!
 * Set of raw return values from a system call.
 */
typedef struct /*<POD>*/
{
    ADDRINT _eax;
    ADDRINT _edx;
    ADDRINT _cf;      // cary flag after syscall
    ADDRINT _stack;   // value pointed at by SP after syscall
} OS_SYSCALLRETURN;

#ifdef __cplusplus
}
#endif

#endif // file guard
