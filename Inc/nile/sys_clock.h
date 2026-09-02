#ifndef NILE_SYS_CLOCK_H_
#define NILE_SYS_CLOCK_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/arch.h"

/* ----------------------------- */
/* Public portable API           */
/* ----------------------------- */

enum{
	NILE_SYS_CLOCK_SRC_CPU = 0,
	NILE_SYS_CLOCK_SRC_EXT = 1,
};

/* Start system clock (enable tick source) */
static NILE_FORCEINLINE void nile_sys_clock_start(void);

/* Stop system clock (disable tick source) */
static NILE_FORCEINLINE void nile_sys_clock_stop(void);

/* Counter read/write */
static NILE_FORCEINLINE uint32_t nile_sys_clock_counter_read(void);
static NILE_FORCEINLINE void nile_sys_clock_counter_write(uint32_t value);

/* Overflow value read/write */
static NILE_FORCEINLINE uint32_t nile_sys_clock_counter_overflow_val_read(void);
static NILE_FORCEINLINE void nile_sys_clock_counter_overflow_val_write(uint32_t value);

/* Overflow happened? (boolean) */
static NILE_FORCEINLINE bool nile_sys_clock_counter_overflow_happened(void);

/* System clock source */
static NILE_FORCEINLINE void nile_sys_clock_src_set(uint32_t src_id);
static NILE_FORCEINLINE uint32_t nile_sys_clock_src_get();

/* ----------------------------- */
/* Architecture-specific impl    */
/* ----------------------------- */

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/sys_clock.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/sys_clock.h"
#else
#error "No system clock implementation for this architecture"
#endif

#endif /* NILE_SYS_CLOCK_H_ */
