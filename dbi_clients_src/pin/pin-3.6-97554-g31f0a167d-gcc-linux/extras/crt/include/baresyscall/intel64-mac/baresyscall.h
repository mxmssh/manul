// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_MAC_INTEL64_BARESYSCALL_H__
#define OS_APIS_MAC_INTEL64_BARESYSCALL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/*!
 * Set of raw return values from a system call.
 */
typedef struct /*<POD>*/
{
    ADDRINT _rax;
    ADDRINT _rdx;
    ADDRINT _cf;
} OS_SYSCALLRETURN;

#ifdef __cplusplus
}
#endif

#endif // file guard
