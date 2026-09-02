#ifndef NILE_FPU_H_
#define NILE_FPU_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/arch.h"

/* ----------------------------- */
/* Public portable API           */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_fpu_enable_full_access(void);
static NILE_FORCEINLINE void nile_fpu_enable_privileged_only(void);
static NILE_FORCEINLINE void nile_fpu_disable(void);

static NILE_FORCEINLINE void nile_fpu_enable_stacking(void);
static NILE_FORCEINLINE void nile_fpu_disable_stacking(void);
static NILE_FORCEINLINE void nile_fpu_enable_lazy_stacking(void);
static NILE_FORCEINLINE void nile_fpu_disable_lazy_stacking(void);

/* ----------------------------- */
/* Architecture-specific impl    */
/* ----------------------------- */

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/fpu.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/fpu.h"
#else
#error "No FPU implementation for this architecture"
#endif

#endif /* NILE_FPU_H_ */
