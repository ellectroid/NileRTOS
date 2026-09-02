#ifndef NILE_KERNEL_H_
#define NILE_KERNEL_H_

#include "nile/kernel_config.h"

#include "nile/kernel/info.h"
#include "nile/kernel/status.h"
#include "nile/kernel/scheduler.h"
#include "nile/kernel/syscall_vtable.h"
#include "nile/kernel/vtable.h"
#include "nile/kernel/io_dev.h"
#include "nile/kernel/kheap.h"

typedef struct nile_kernel{
	nile_kernel_info info; //basic kernel info (size, version)
	nile_kernel_system_status status; //status flags, optional logging info
	nile_kernel_scheduler scheduler; //scheduling params
	nile_kernel_vtable vtable; //supplementary function pointers
	nile_kernel_syscall_vtable syscall_vtable; //system call vectors
	nile_kernel_io_char_dev io_char_dev[NILE_KERNEL_IO_CHAR_DEV_COUNT]; //io char device control
	nile_kernel_io_block_dev io_block_dev[NILE_KERNEL_IO_BLOCK_DEV_COUNT]; //io block device control
	nile_kernel_kheap heap; //kernel heap
}nile_kernel;

uint32_t nile_kernel_init();
uint32_t nile_kernel_init_devices();
NILE_NORETURN void nile_kernel_start();

#endif /* NILE_KERNEL_H_ */
