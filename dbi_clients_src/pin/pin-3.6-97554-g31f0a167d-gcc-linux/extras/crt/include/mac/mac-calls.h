// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_MAC_CALLS_H__
#define OS_APIS_MAC_CALLS_H__

#include "os-apis.h"
#include <mach/mach.h>

#include "my_syscall_sw.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UC_FLAVOR           30
#define UC_RESET_ALT_STACK  0x80000000
#define UC_SET_ALT_STACK    0x40000000


#ifdef TARGET_IA32
# define SYSCALL_CONSTRUCT_UNIX_INT80(syscall_number) (syscall_number|0x04000000)
#else
# define SYSCALL_CONSTRUCT_UNIX_INT80(syscall_number) SYSCALL_CONSTRUCT_UNIX(syscall_number)
#endif


#define msgh_request_port   msgh_remote_port
#define msgh_reply_port     msgh_local_port

extern NDR_record_t MY_NDR_record;

int MacSysCtl(int *name, u_int namelen, void *oldp, size_t *oldlenp,
    void *newp, size_t newlen);

mach_port_t OS_mach_alloc_reply_port();
kern_return_t OS_mach_port_deallocate ( ipc_space_t task, mach_port_name_t name);

/**
 * @brief Deallocate/destroy the given port (@name) (we actually deallocate/destroy what is called a task's right)
 */
kern_return_t OS_mach_port_mod_refs(ipc_space_t task, mach_port_name_t name, mach_port_right_t right, mach_port_delta_t delta);

mach_msg_return_t OS_mach_msg(mach_msg_header_t *msg, mach_msg_option_t option, mach_msg_size_t send_size, mach_msg_size_t rcv_size,
        mach_port_t rcv_name, mach_msg_timeout_t timeout, mach_port_t notify);
mach_msg_return_t OS_mach_msg_send(mach_msg_header_t *msg);
mach_msg_return_t OS_mach_msg_receive(mach_msg_header_t *msg);
mach_port_t task_self_trap();
int OS_proc_info(int callnum, int pid, int flavor, uint64_t arg, void * buffer, int buffersize);
int OS_proc_pidinfo(int pid, int flavor, uint64_t arg,  void *buffer, int buffersize);

#ifdef __cplusplus
}
#endif

#endif // OS_APIS_MAC_CALLS_H__

