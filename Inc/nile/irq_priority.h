#ifndef NILE_IRQ_PRIORITY_H_
#define NILE_IRQ_PRIORITY_H_
#include "nile/compiler.h"
#include "nile/mcu.h"

#ifndef NILE_REQUIRED
#define NILE_REQUIRED 0x87654321U
#endif

/* Sentinel meaning "must be overridden by MCU-specific backend" */
#define NILE_MCU_IRQ_EXC_PRIO_LVL_CNT NILE_REQUIRED
#define NILE_MCU_IRQ_EXC_PRIO_BITS    NILE_REQUIRED

#if defined(NILE_MCU_STM32F746)
#include "nile/mcu/stm32f746/irq_priority.h"
#endif

/* Validation: ensure backend overrode the sentinel */
#if NILE_MCU_IRQ_EXC_PRIO_LVL_CNT == NILE_REQUIRED
#error "NILE_MCU_IRQ_EXC_PRIO_LVL_CNT must be defined by MCU-specific config"
#endif

#if NILE_MCU_IRQ_EXC_PRIO_BITS == NILE_REQUIRED
#error "NILE_MCU_IRQ_EXC_PRIO_BITS must be defined by MCU-specific config"
#endif

#if (NILE_MCU_IRQ_EXC_PRIO_LVL_CNT != 16 && NILE_MCU_IRQ_EXC_PRIO_LVL_CNT != 8)
#error "Unsupported priority level count"
#endif

enum NILE_IRQ_PRIORITY{
    NILE_IRQ_PRIO_LVL_SYS_CTX_SWITCH = 0,
    NILE_IRQ_PRIO_LVL_SYS_TICK_SRC   = 1,
    NILE_IRQ_PRIO_LVL_SYS_SYSCALL    = 2,

    NILE_IRQ_PRIO_LVL_USR_MIN        = 3,
    NILE_IRQ_PRIO_LVL_USR_0          = 3,
    NILE_IRQ_PRIO_LVL_USR_1,
    NILE_IRQ_PRIO_LVL_USR_2,
    NILE_IRQ_PRIO_LVL_USR_3,
#if (NILE_MCU_IRQ_EXC_PRIO_LVL_CNT == 16)
    NILE_IRQ_PRIO_LVL_USR_4,
    NILE_IRQ_PRIO_LVL_USR_5,
    NILE_IRQ_PRIO_LVL_USR_6,
    NILE_IRQ_PRIO_LVL_USR_7,
    NILE_IRQ_PRIO_LVL_USR_8,
    NILE_IRQ_PRIO_LVL_USR_9,
    NILE_IRQ_PRIO_LVL_USR_10,
	NILE_IRQ_PRIO_LVL_USR_11,
#endif
	NILE_IRQ_PRIO_LVL_SYS_EXC,
    NILE_IRQ_PRIO_LVL_USR_MAX = NILE_IRQ_PRIO_LVL_SYS_EXC - 1,
};

static NILE_FORCEINLINE uint32_t nile_irq_get_raw_priority_value(uint32_t nile_irq_priority);
static NILE_FORCEINLINE uint32_t nile_irq_get_nile_priority_value(uint32_t raw_irq_priority);

/* Clean up sentinel to avoid conflicts */
#undef NILE_REQUIRED

#endif /* NILE_IRQ_PRIORITY_H_ */
