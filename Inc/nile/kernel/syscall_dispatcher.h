#ifndef NILE_KERNEL_SYSCALL_DISPATCHER_H_
#define NILE_KERNEL_SYSCALL_DISPATCHER_H_
#include "nile/stdtypes.h"

void syscall_handler_dispatch(uint32_t system_call_id, uint32_t* arg_list, uint32_t arg_cnt);

#endif /* NILE_KERNEL_SYSCALL_DISPATCHER_H_ */
