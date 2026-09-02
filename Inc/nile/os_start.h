#ifndef NILE_OS_START_H_
#define NILE_OS_START_H_

#include "nile/arch.h"
#include "nile/compiler.h"

NILE_NAKED void os_start(uint32_t kernel_sp, uint32_t task_sp);

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/os_start.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/os_start.h"
#else
#error "No OS start implementation for this architecture"
#endif
#endif /* NILE_OS_START_H_ */
