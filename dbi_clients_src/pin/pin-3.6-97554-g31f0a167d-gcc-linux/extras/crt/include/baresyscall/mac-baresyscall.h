// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_MAC_BARESYSCALL_H__
#define OS_APIS_MAC_BARESYSCALL_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(TARGET_IA32)

#include "ia32-mac/baresyscall.h"
#else
#include "intel64-mac/baresyscall.h"

#endif

#include "unix-baresyscall.h"

/*!
 * @return  The last system call's second return value.  If IsSuccess() is FALSE,
 *           this is an O/S dependent error code.
 */
ADDRINT OS_SyscallReturnValue2(OS_SYSCALLRETURN ret);

/*!
 * @param[in] type         The system call type (linux, int80 , int81 ....).
 *
 * @return  The return address is the PC of the thread before it returns
 *          the system call.
 */
void* OS_GetSyscallTrapAddress(OS_SYSCALL_TYPE type);

#ifdef __cplusplus
}
#endif

#endif // file guard
