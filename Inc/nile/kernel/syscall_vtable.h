#ifndef NILE_KERNEL_SYSCALL_VTABLE_H_
#define NILE_KERNEL_SYSCALL_VTABLE_H_

#include "nile/stdtypes.h"
#include "nile/kernel_config.h"

typedef uint32_t (*nile_kernel_syscall_handler_t)(uint32_t* arg_list, uint32_t arg_cnt);

typedef struct nile_kernel_syscall_vtable {
    uint32_t vector_count;
    nile_kernel_syscall_handler_t vector[NILE_SYSCALL_VECTOR_COUNT];
} nile_kernel_syscall_vtable;

#endif /* NILE_KERNEL_SYSCALL_VTABLE_H_ */
