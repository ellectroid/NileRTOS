#ifndef NILE_ARCH_ARMV7M_SYSCALL_H_
#define NILE_ARCH_ARMV7M_SYSCALL_H_

#include "nile/compiler.h"

static NILE_NAKED NILE_USED uint32_t nile_syscall(uint32_t syscall_id, void *uint32_t_arg_list, uint32_t arg_cnt) {
	(void) syscall_id;
	(void) uint32_t_arg_list;
	(void) arg_cnt;
	__asm__ volatile(
			"SVC #0      \n"
			"BX  LR      \n"
	);
}

#endif /* NILE_ARCH_ARMV7M_SYSCALL_H_ */
