#ifndef NILE_CACHE_H_
#define NILE_CACHE_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/arch.h"

/* ----------------------------- */
/* Public portable API           */
/* ----------------------------- */

/* I-cache */
static NILE_FORCEINLINE void nile_cache_invalidate_icache_all(void);
static NILE_FORCEINLINE void nile_cache_enable_icache(void);
static NILE_FORCEINLINE void nile_cache_disable_icache(void);

/* D-cache: full operations */
static NILE_FORCEINLINE void nile_cache_clean_invalidate_dcache_all(void);
static NILE_FORCEINLINE void nile_cache_clean_dcache_all(void);
static NILE_FORCEINLINE void nile_cache_invalidate_dcache_all(void);

/* D-cache: per-address operations */
static NILE_FORCEINLINE void nile_cache_clean_dcache_addr(uintptr_t addr);
static NILE_FORCEINLINE void nile_cache_invalidate_dcache_addr(uintptr_t addr);
static NILE_FORCEINLINE void nile_cache_clean_invalidate_dcache_addr(uintptr_t addr);

/* Branch prediction */
static NILE_FORCEINLINE void nile_branch_prediction_enable(void);
static NILE_FORCEINLINE void nile_branch_prediction_disable(void);

/* ----------------------------- */
/* Architecture-specific impl    */
/* ----------------------------- */

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/cache.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/cache.h"
#else
#error "No cache implementation for this architecture"
#endif

#endif /* NILE_CACHE_H_ */
