#ifndef NILE_MEMPROT_H_
#define NILE_MEMPROT_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/arch.h"

/* ----------------------------- */
/* Portable permission model     */
/* ----------------------------- */

#define NILE_MEMPROT_PERM_NONE        0U
#define NILE_MEMPROT_PERM_RW_PRIV     1U
#define NILE_MEMPROT_PERM_RW_FULL     2U
#define NILE_MEMPROT_PERM_RO_PRIV     3U
#define NILE_MEMPROT_PERM_RO_FULL     4U

/* ----------------------------- */
/* Portable memory type model    */
/* ----------------------------- */

#define NILE_MEMPROT_MEM_STRONGLY_ORDERED  0U
#define NILE_MEMPROT_MEM_DEVICE            1U
#define NILE_MEMPROT_MEM_NORMAL            2U
#define NILE_MEMPROT_MEM_NORMAL_NOCACHE    3U

/* ----------------------------- */
/* Public portable API           */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_memprot_enable(void);
static NILE_FORCEINLINE void nile_memprot_disable(void);

static NILE_FORCEINLINE void nile_memprot_enable_background(void);
static NILE_FORCEINLINE void nile_memprot_disable_background(void);

static NILE_FORCEINLINE void nile_memprot_region_enable(uint32_t region);
static NILE_FORCEINLINE void nile_memprot_region_disable(uint32_t region);

static NILE_FORCEINLINE void nile_memprot_region_configure(
    uint32_t region,
    uintptr_t base,
    size_t size,
    uint32_t perms,
    uint32_t type,
    bool executable
);

/* ----------------------------- */
/* Architecture-specific impl    */
/* ----------------------------- */

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/memprot.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/memprot.h"
#else
#error "No memory protection implementation for this architecture"
#endif

#endif /* NILE_MEMPROT_H_ */
