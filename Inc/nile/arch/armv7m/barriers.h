#ifndef NILE_ARCH_ARMV7M_BARRIERS_H_
#define NILE_ARCH_ARMV7M_BARRIERS_H_

#include "nile/compiler.h"

/* Data Synchronization Barrier */
static NILE_FORCEINLINE void nile_dsb(void)
{
    __asm__ volatile ("dsb 0xF" ::: "memory");
}

/* Instruction Synchronization Barrier */
static NILE_FORCEINLINE void nile_isb(void)
{
    __asm__ volatile ("isb 0xF" ::: "memory");
}

/* Data Memory Barrier */
static NILE_FORCEINLINE void nile_dmb(void)
{
    __asm__ volatile ("dmb 0xF" ::: "memory");
}

/* Send Event */
static NILE_FORCEINLINE void nile_sev(void)
{
    __asm__ volatile ("sev" ::: "memory");
}

/* Wait For Event */
static NILE_FORCEINLINE void nile_wfe(void)
{
    __asm__ volatile ("wfe" ::: "memory");
}

/* Wait For Interrupt */
static NILE_FORCEINLINE void nile_wfi(void)
{
    __asm__ volatile ("wfi" ::: "memory");
}

#endif
