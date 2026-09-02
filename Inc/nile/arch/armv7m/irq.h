#ifndef NILE_ARCH_ARMV7M_IRQ_H_
#define NILE_ARCH_ARMV7M_IRQ_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/barriers.h"
#include "nile/irq_priority.h"
#include "nile/memlayout.h"

/* Priority shift for external IRQs */
#ifndef NILE_MCU_IRQ_EXC_PRIO_SHIFT
#define NILE_MCU_IRQ_EXC_PRIO_SHIFT  (8 - NILE_MCU_IRQ_EXC_PRIO_BITS)
#endif

/* ----------------------------- */
/* Helpers                       */
/* ----------------------------- */

static NILE_FORCEINLINE volatile uint32_t* nile_nvic_iser(uint32_t index)
{
    return &NVIC->ISER[index];
}

static NILE_FORCEINLINE volatile uint32_t* nile_nvic_icer(uint32_t index)
{
    return &NVIC->ICER[index];
}

static NILE_FORCEINLINE volatile uint32_t* nile_nvic_ispr(uint32_t index)
{
    return &NVIC->ISPR[index];
}

static NILE_FORCEINLINE volatile uint32_t* nile_nvic_icpr(uint32_t index)
{
    return &NVIC->ICPR[index];
}

static NILE_FORCEINLINE volatile uint32_t* nile_nvic_iabr(uint32_t index)
{
    return &NVIC->IABR[index];
}

static NILE_FORCEINLINE volatile uint8_t* nile_nvic_ipr(uint32_t index)
{
    return &NVIC->IP[index];
}

static NILE_FORCEINLINE volatile uint32_t* nile_nvic_stir(void)
{
    return &NVIC->STIR;
}

/* ----------------------------- */
/* Global interrupt control      */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_irq_global_enable(void)
{
    __asm__ volatile ("cpsie i" ::: "memory");
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_irq_global_disable(void)
{
    __asm__ volatile ("cpsid i" ::: "memory");
    nile_dsb();
    nile_isb();
}

/* ----------------------------- */
/* Per-interrupt control         */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_irq_enable(nile_irq_id_t id)
{
    if (id < 0) {
        return;
    }

    uint32_t irq   = (uint32_t)id;
    uint32_t index = irq / 32U;
    uint32_t bit   = irq % 32U;

    NVIC->ISER[index] = (1U << bit);
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_irq_disable(nile_irq_id_t id)
{
    if (id < 0) {
        return;
    }

    uint32_t irq   = (uint32_t)id;
    uint32_t index = irq / 32U;
    uint32_t bit   = irq % 32U;

    NVIC->ICER[index] = (1U << bit);
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_irq_set_priority(nile_irq_id_t id, uint32_t priority)
{
    if (id < 0) {
        return;
    }

    uint32_t irq = (uint32_t)id;
    NVIC->IP[irq] = (uint8_t)(priority << NILE_MCU_IRQ_EXC_PRIO_SHIFT);
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE uint32_t nile_irq_get_priority(nile_irq_id_t id)
{
    if (id < 0) {
        return 0U;
    }

    uint32_t irq = (uint32_t)id;
    return (uint32_t)(NVIC->IP[irq] >> NILE_MCU_IRQ_EXC_PRIO_SHIFT);
}

static NILE_FORCEINLINE void nile_irq_set_pending(nile_irq_id_t id)
{
    if (id < 0) {
        return;
    }

    uint32_t irq   = (uint32_t)id;
    uint32_t index = irq / 32U;
    uint32_t bit   = irq % 32U;

    NVIC->ISPR[index] = (1U << bit);
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE void nile_irq_clear_pending(nile_irq_id_t id)
{
    if (id < 0) {
        return;
    }

    uint32_t irq   = (uint32_t)id;
    uint32_t index = irq / 32U;
    uint32_t bit   = irq % 32U;

    NVIC->ICPR[index] = (1U << bit);
    nile_dsb();
    nile_isb();
}

static NILE_FORCEINLINE bool nile_irq_is_active(nile_irq_id_t id)
{
    if (id < 0) {
        return false;
    }

    uint32_t irq   = (uint32_t)id;
    uint32_t index = irq / 32U;
    uint32_t bit   = irq % 32U;

    uint32_t val = NVIC->IABR[index];
    return ((val & (1U << bit)) != 0U);
}

/* Software-triggered interrupt */
static NILE_FORCEINLINE void nile_irq_trigger(nile_irq_id_t id)
{
    if (id < 0) {
        return;
    }

    uint32_t irq = (uint32_t)id & 0x1FFU;
    NVIC->STIR = irq;
    nile_dsb();
    nile_isb();
}

/* Current active IRQ: returns external IRQ number or -1 if none */
static NILE_FORCEINLINE nile_irq_id_t nile_irq_current(void)
{
    uint32_t icsr = SCB->ICSR;
    uint32_t vect = icsr & SCB_ICSR_VECTACTIVE_Msk;

    if (vect >= 16U) {
        return (nile_irq_id_t)(vect - 16U);
    }

    return (nile_irq_id_t)-1;
}

#endif /* NILE_ARCH_ARMV7M_IRQ_H_ */
