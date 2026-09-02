#ifndef NILE_KERNEL_VTABLE_H_
#define NILE_KERNEL_VTABLE_H_

#include "nile/stdtypes.h"
#include "nile/kernel_config.h"

typedef void (*nile_kernel_vtable_handler_t)(void *kernel, void* arg1, uint32_t arg2, uint32_t arg3);
typedef struct nile_kernel_vtable {
	uint32_t vector_count;
	nile_kernel_vtable_handler_t vector[NILE_VTABLE_VECTOR_COUNT];
} nile_kernel_vtable;

#endif /* NILE_KERNEL_VTABLE_H_ */
