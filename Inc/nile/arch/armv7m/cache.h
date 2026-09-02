#ifndef NILE_ARCH_ARMV7M_CACHE_H_
#define NILE_ARCH_ARMV7M_CACHE_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/barriers.h"
#include "nile/memlayout.h"

/* ----------------------------- */
/* Helpers                       */
/* ----------------------------- */

static NILE_FORCEINLINE volatile uint32_t* nile_scb_ccr(void)
{
    return &SCB->CCR;
}

static NILE_FORCEINLINE volatile uint32_t* nile_scb_iciallu(void)
{
    return &SCB->ICIALLU;
}

/* ----------------------------- */
/* I-CACHE MAINTENANCE           */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_cache_invalidate_icache_all(void)
{
    SCB->ICIALLU = 0U;
    nile_dsb();
    nile_isb();
}

/* ----------------------------- */
/* D-CACHE MAINTENANCE           */
/* ----------------------------- */

/* Select D-cache in CSSELR */
static NILE_FORCEINLINE void nile_cache_select_dcache(void)
{
    SCB->CSSELR = 0U; /* Level 0, D-cache */
    nile_dsb();
}

/* Read CCSIDR */
static NILE_FORCEINLINE uint32_t nile_cache_read_ccsidr(void)
{
    return SCB->CCSIDR;
}

/* Full D-cache clean+invalidate */
static NILE_FORCEINLINE void nile_cache_clean_invalidate_dcache_all(void)
{
    nile_cache_select_dcache();
    uint32_t ccsidr = nile_cache_read_ccsidr();

    uint32_t sets = ((ccsidr >> 13) & 0x7FFF);
    uint32_t ways = ((ccsidr >> 3) & 0x3FF);

    for (uint32_t way = 0; way <= ways; way++)
    {
        for (uint32_t set = 0; set <= sets; set++)
        {
            uint32_t sw = (way << 30) | (set << 5);
            SCB->DCCISW = sw;
        }
    }

    nile_dsb();
    nile_isb();
}

/* Full D-cache clean */
static NILE_FORCEINLINE void nile_cache_clean_dcache_all(void)
{
    nile_cache_select_dcache();
    uint32_t ccsidr = nile_cache_read_ccsidr();

    uint32_t sets = ((ccsidr >> 13) & 0x7FFF);
    uint32_t ways = ((ccsidr >> 3) & 0x3FF);

    for (uint32_t way = 0; way <= ways; way++)
    {
        for (uint32_t set = 0; set <= sets; set++)
        {
            uint32_t sw = (way << 30) | (set << 5);
            SCB->DCCSW = sw;
        }
    }

    nile_dsb();
    nile_isb();
}

/* Full D-cache invalidate */
static NILE_FORCEINLINE void nile_cache_invalidate_dcache_all(void)
{
    nile_cache_select_dcache();
    uint32_t ccsidr = nile_cache_read_ccsidr();

    uint32_t sets = ((ccsidr >> 13) & 0x7FFF);
    uint32_t ways = ((ccsidr >> 3) & 0x3FF);

    for (uint32_t way = 0; way <= ways; way++)
    {
        for (uint32_t set = 0; set <= sets; set++)
        {
            uint32_t sw = (way << 30) | (set << 5);
            SCB->DCISW = sw;
        }
    }

    nile_dsb();
    nile_isb();
}

/* ----------------------------- */
/* Per-address operations        */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_cache_clean_dcache_addr(uintptr_t addr)
{
    SCB->DCCMVAC = (uint32_t)addr;
    nile_dsb();
}

static NILE_FORCEINLINE void nile_cache_invalidate_dcache_addr(uintptr_t addr)
{
    SCB->DCIMVAC = (uint32_t)addr;
    nile_dsb();
}

static NILE_FORCEINLINE void nile_cache_clean_invalidate_dcache_addr(uintptr_t addr)
{
    SCB->DCCIMVAC = (uint32_t)addr;
    nile_dsb();
}

/* ----------------------------- */
/* Cache enable/disable          */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_cache_enable_icache(void)
{
    nile_cache_invalidate_icache_all();
    volatile uint32_t *ccr = nile_scb_ccr();
    *ccr |= SCB_CCR_IC_Msk;
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_cache_disable_icache(void)
{
    volatile uint32_t *ccr = nile_scb_ccr();
    *ccr &= ~SCB_CCR_IC_Msk;
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_cache_enable_dcache(void)
{
    nile_cache_clean_invalidate_dcache_all();
    volatile uint32_t *ccr = nile_scb_ccr();
    *ccr |= SCB_CCR_DC_Msk;
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_cache_disable_dcache(void)
{
    volatile uint32_t *ccr = nile_scb_ccr();
    *ccr &= ~SCB_CCR_DC_Msk;
    nile_dsb();
    nile_isb();
}

/* ----------------------------- */
/* Branch prediction             */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_branch_prediction_enable(void)
{
    volatile uint32_t *ccr = nile_scb_ccr();
    *ccr |= SCB_CCR_BP_Msk;
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_branch_prediction_disable(void)
{
    volatile uint32_t *ccr = nile_scb_ccr();
    *ccr &= ~SCB_CCR_BP_Msk;
    nile_dsb();
    nile_isb();
}

#endif /* NILE_ARCH_ARMV7M_CACHE_H_ */
