#ifndef NILE_BARRIERS_H_
#define NILE_BARRIERS_H_

#include "nile/compiler.h"
#include "nile/arch.h"

/* ----------------------------- */
/* Public portable API           */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_dsb(void);
static NILE_FORCEINLINE void nile_isb(void);
static NILE_FORCEINLINE void nile_dmb(void);

static NILE_FORCEINLINE void nile_sev(void);
static NILE_FORCEINLINE void nile_wfe(void);
static NILE_FORCEINLINE void nile_wfi(void);

/* ----------------------------- */
/* Architecture-specific impl    */
/* ----------------------------- */

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/barriers.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/barriers.h"
#else
#error "No barrier implementation for this architecture"
#endif

#endif /* NILE_BARRIERS_H_ */
