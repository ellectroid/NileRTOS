#ifndef NILE_ARCH_ARMV7M_FPU_H_
#define NILE_ARCH_ARMV7M_FPU_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/barriers.h"
#include "nile/memlayout.h"

/* ----------------------------- */
/* Internal helpers              */
/* ----------------------------- */

#define SCB_CPACR_CP10_Pos      20U
#define SCB_CPACR_CP11_Pos      22U

#define SCB_CPACR_CP10_Msk      (3U << SCB_CPACR_CP10_Pos)
#define SCB_CPACR_CP11_Msk      (3U << SCB_CPACR_CP11_Pos)

static NILE_FORCEINLINE volatile uint32_t* nile_scb_cpacr(void)
{
    return &SCB->CPACR;
}

static NILE_FORCEINLINE volatile uint32_t* nile_fpu_fpccr(void)
{
    return &FPU->FPCCR;
}

/* ----------------------------- */
/* FPU access control            */
/* ----------------------------- */

/* Full access: privileged + unprivileged */
static NILE_FORCEINLINE void nile_fpu_enable_full_access(void)
{
    uint32_t val = SCB->CPACR;

    val &= ~((3U << SCB_CPACR_CP10_Pos) | (3U << SCB_CPACR_CP11_Pos));
    val |=  ((3U << SCB_CPACR_CP10_Pos) | (3U << SCB_CPACR_CP11_Pos));

    SCB->CPACR = val;

    nile_dsb();
    nile_isb();
}

/* Privileged-only access */
static NILE_FORCEINLINE void nile_fpu_enable_privileged_only(void)
{
    uint32_t val = SCB->CPACR;

    val &= ~((3U << SCB_CPACR_CP10_Pos) | (3U << SCB_CPACR_CP11_Pos));
    val |=  ((1U << SCB_CPACR_CP10_Pos) | (1U << SCB_CPACR_CP11_Pos));

    SCB->CPACR = val;

    nile_dsb();
    nile_isb();
}

/* Disable FPU entirely */
static NILE_FORCEINLINE void nile_fpu_disable(void)
{
    uint32_t val = SCB->CPACR;

    val &= ~((3U << SCB_CPACR_CP10_Pos) | (3U << SCB_CPACR_CP11_Pos));

    SCB->CPACR = val;

    nile_dsb();
    nile_isb();
}

/* ----------------------------- */
/* Stacking control              */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_fpu_enable_stacking(void)
{
    uint32_t val = FPU->FPCCR;
    val |= FPU_FPCCR_ASPEN_Msk;
    FPU->FPCCR = val;

    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_fpu_disable_stacking(void)
{
    uint32_t val = FPU->FPCCR;
    val &= ~FPU_FPCCR_ASPEN_Msk;
    FPU->FPCCR = val;

    nile_dsb();
    nile_isb();
}

/* ----------------------------- */
/* Lazy stacking                 */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_fpu_enable_lazy_stacking(void)
{
    uint32_t val = FPU->FPCCR;
    val |= (FPU_FPCCR_LSPEN_Msk | FPU_FPCCR_ASPEN_Msk);
    FPU->FPCCR = val;

    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_fpu_disable_lazy_stacking(void)
{
    uint32_t val = FPU->FPCCR;
    val &= ~FPU_FPCCR_LSPEN_Msk;
    FPU->FPCCR = val;

    nile_dsb();
    nile_isb();
}

#endif /* NILE_ARCH_ARMV7M_FPU_H_ */
