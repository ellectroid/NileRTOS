#ifndef NILE_MCU_IRQ_PRIORITY_H_
#define NILE_MCU_IRQ_PRIORITY_H_

#include "nile/compiler.h"
#include "nile/stdtypes.h"

#undef NILE_MCU_IRQ_EXC_PRIO_LVL_CNT
//#define NILE_MCU_IRQ_EXC_PRIO_LVL_CNT (8)
#define NILE_MCU_IRQ_EXC_PRIO_LVL_CNT (16)

#undef NILE_MCU_IRQ_EXC_PRIO_BITS
#define NILE_MCU_IRQ_EXC_PRIO_BITS    4

static NILE_FORCEINLINE uint32_t nile_irq_get_raw_priority_value(uint32_t nile_irq_priority)
{
    return NILE_MCU_IRQ_EXC_PRIO_LVL_CNT - 1 - nile_irq_priority;
}

static NILE_FORCEINLINE uint32_t nile_irq_get_nile_priority_value(uint32_t raw_irq_priority)
{
    return NILE_MCU_IRQ_EXC_PRIO_LVL_CNT - 1 - raw_irq_priority;
}

#endif /* NILE_MCU_IRQ_PRIORITY_H_ */
