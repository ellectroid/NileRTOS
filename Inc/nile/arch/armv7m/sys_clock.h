#ifndef NILE_ARCH_ARMV7M_SYS_CLOCK_H_
#define NILE_ARCH_ARMV7M_SYS_CLOCK_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/memlayout.h"


/* ARMv7-M system clock implementation */

/* ARMv7-M system clock implementation */

static NILE_FORCEINLINE void nile_sys_clock_start(void)
{
    /* Enable SysTick counter (do NOT enable interrupt here) */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

static NILE_FORCEINLINE void nile_sys_clock_stop(void)
{
    /* Disable SysTick counter */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

static NILE_FORCEINLINE uint32_t nile_sys_clock_counter_read(void)
{
    /* Read current counter value */
    return SysTick->VAL & SysTick_VAL_CURRENT_Msk;
}

static NILE_FORCEINLINE void nile_sys_clock_counter_write(uint32_t value)
{
    /* Write current counter value */
    SysTick->VAL = (value & SysTick_VAL_CURRENT_Msk);
}

static NILE_FORCEINLINE uint32_t nile_sys_clock_counter_overflow_val_read(void)
{
    /* Read reload value (overflow threshold) */
    return SysTick->LOAD & SysTick_LOAD_RELOAD_Msk;
}

static NILE_FORCEINLINE void nile_sys_clock_counter_overflow_val_write(uint32_t value)
{
    /* Write reload value (overflow threshold) */
    SysTick->LOAD = (value & SysTick_LOAD_RELOAD_Msk);
}

static NILE_FORCEINLINE bool nile_sys_clock_counter_overflow_happened(void)
{
    /* COUNTFLAG is 1 when VAL wrapped from 0 to LOAD */
    return (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U;
}

static NILE_FORCEINLINE void nile_sys_clock_src_set(uint32_t src_id)
{
    switch (src_id)
    {
        case NILE_SYS_CLOCK_SRC_CPU:
            /* Use CPU clock (AHB) */
            SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
            break;

        case NILE_SYS_CLOCK_SRC_EXT:
            /* Use external reference clock */
            SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;
            break;

        default:
            /* Unknown source — ignore */
            break;
    }
}

static NILE_FORCEINLINE uint32_t nile_sys_clock_src_get(void)
{
    uint32_t ret = NILE_SYS_CLOCK_SRC_CPU; /* default */

    switch (SysTick->CTRL & SysTick_CTRL_CLKSOURCE_Msk)
    {
        case SysTick_CTRL_CLKSOURCE_Msk:
            ret = NILE_SYS_CLOCK_SRC_CPU;
            break;

        default:
            ret = NILE_SYS_CLOCK_SRC_EXT;
            break;
    }

    return ret;
}

#endif /* NILE_ARCH_ARMV7M_SYS_CLOCK_H_ */
