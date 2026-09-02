#ifndef NILE_ARCH_ARMV7M_MEMPROT_H_
#define NILE_ARCH_ARMV7M_MEMPROT_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/barriers.h"
#include "nile/memlayout.h"

/* ----------------------------- */
/* Helpers to access registers   */
/* ----------------------------- */

static NILE_FORCEINLINE volatile uint32_t* nile_mpu_ctrl(void)
{
    return &MPU->CTRL;
}

static NILE_FORCEINLINE volatile uint32_t* nile_mpu_rnr(void)
{
    return &MPU->RNR;
}

static NILE_FORCEINLINE volatile uint32_t* nile_mpu_rbar(void)
{
    return &MPU->RBAR;
}

static NILE_FORCEINLINE volatile uint32_t* nile_mpu_rasr(void)
{
    return &MPU->RASR;
}

/* ----------------------------- */
/* Region select                 */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_memprot_region_select(uint32_t region)
{
    MPU->RNR = region;
}

/* ----------------------------- */
/* Encode size                   */
/* ----------------------------- */

static NILE_FORCEINLINE uint32_t nile_memprot_armv7m_encode_size(size_t size)
{
    uint32_t s = (uint32_t)size;
    uint32_t log2 = 0U;

    while (s > 1U) {
        s >>= 1U;
        log2++;
    }

    /* SIZE field = (log2(size) - 1) << MPU_RASR_SIZE_Pos */
    return ((log2 - 1U) << MPU_RASR_SIZE_Pos);
}

/* ----------------------------- */
/* Encode permissions            */
/* ----------------------------- */

static NILE_FORCEINLINE uint32_t nile_memprot_armv7m_encode_perms(uint32_t perms)
{
    uint32_t ap;

    switch (perms) {
    case NILE_MEMPROT_PERM_RW_PRIV:
        ap = 0x1U;
        break;
    case NILE_MEMPROT_PERM_RW_FULL:
        ap = 0x3U;
        break;
    case NILE_MEMPROT_PERM_RO_PRIV:
        ap = 0x5U;
        break;
    case NILE_MEMPROT_PERM_RO_FULL:
        ap = 0x6U;
        break;
    case NILE_MEMPROT_PERM_NONE:
    default:
        ap = 0x0U;
        break;
    }

    return (ap << MPU_RASR_AP_Pos);
}

/* ----------------------------- */
/* Encode memory type            */
/* ----------------------------- */

static NILE_FORCEINLINE uint32_t nile_memprot_armv7m_encode_memtype(uint32_t type)
{
    uint32_t tex = 0U, c = 0U, b = 0U, s = 0U;

    switch (type) {
    case NILE_MEMPROT_MEM_STRONGLY_ORDERED:
        tex = 0U; c = 0U; b = 0U; s = 1U;
        break;
    case NILE_MEMPROT_MEM_DEVICE:
        tex = 0U; c = 0U; b = 1U; s = 1U;
        break;
    case NILE_MEMPROT_MEM_NORMAL:
        tex = 0U; c = 1U; b = 1U; s = 0U;
        break;
    case NILE_MEMPROT_MEM_NORMAL_NOCACHE:
        tex = 1U; c = 0U; b = 0U; s = 0U;
        break;
    default:
        tex = 0U; c = 0U; b = 0U; s = 0U;
        break;
    }

    uint32_t rasr = 0U;
    rasr |= (tex << MPU_RASR_TEX_Pos);
    rasr |= (s   << MPU_RASR_S_Pos);
    rasr |= (c   << MPU_RASR_C_Pos);
    rasr |= (b   << MPU_RASR_B_Pos);

    return rasr;
}

/* ----------------------------- */
/* Global control                */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_memprot_enable(void)
{
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk;
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_memprot_disable(void)
{
    MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_memprot_enable_background(void)
{
    MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk;
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_memprot_disable_background(void)
{
    MPU->CTRL &= ~MPU_CTRL_PRIVDEFENA_Msk;
    nile_dsb();
    nile_isb();
}

/* ----------------------------- */
/* Region lifecycle              */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_memprot_region_enable(uint32_t region)
{
    MPU->RNR = region;
    MPU->RASR |= MPU_RASR_ENABLE_Msk;
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_memprot_region_disable(uint32_t region)
{
    MPU->RNR = region;
    MPU->RASR &= ~MPU_RASR_ENABLE_Msk;
    nile_dsb();
    nile_isb();
}

/* ----------------------------- */
/* Region configuration          */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_memprot_region_configure(
    uint32_t region,
    uintptr_t base,
    size_t size,
    uint32_t perms,
    uint32_t type,
    bool executable
)
{
    MPU->RNR = region;

    uint32_t rasr = 0U;

    rasr |= nile_memprot_armv7m_encode_size(size);
    rasr |= nile_memprot_armv7m_encode_perms(perms);
    rasr |= nile_memprot_armv7m_encode_memtype(type);

    if (!executable) {
        rasr |= MPU_RASR_XN_Msk;
    }

    rasr |= MPU_RASR_ENABLE_Msk;

    MPU->RBAR = (uint32_t)base;
    MPU->RASR = rasr;

    nile_dsb();
    nile_isb();
}

#endif /* NILE_ARCH_ARMV7M_MEMPROT_H_ */
