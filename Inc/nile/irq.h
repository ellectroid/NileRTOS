#ifndef NILE_IRQ_H_
#define NILE_IRQ_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/arch.h"

/* Universal interrupt ID type */
typedef int32_t nile_irq_id_t;

/* ----------------------------- */
/* Public portable API           */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_irq_global_enable(void);
static NILE_FORCEINLINE void nile_irq_global_disable(void);

static NILE_FORCEINLINE void nile_irq_enable(nile_irq_id_t raw_id);
static NILE_FORCEINLINE void nile_irq_disable(nile_irq_id_t raw_id);

static NILE_FORCEINLINE void nile_irq_set_priority(nile_irq_id_t raw_id, uint32_t raw_priority);
static NILE_FORCEINLINE uint32_t nile_irq_get_priority(nile_irq_id_t raw_id);

static NILE_FORCEINLINE void nile_irq_set_pending(nile_irq_id_t raw_id);
static NILE_FORCEINLINE void nile_irq_clear_pending(nile_irq_id_t raw_id);

static NILE_FORCEINLINE bool nile_irq_is_active(nile_irq_id_t raw_id);

static NILE_FORCEINLINE void nile_irq_trigger(nile_irq_id_t raw_id);

static NILE_FORCEINLINE nile_irq_id_t nile_irq_current(void);

/* ----------------------------- */
/* Architecture-specific impl    */
/* ----------------------------- */

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/irq.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/irq.h"
#else
#error "No interrupt implementation for this architecture"
#endif

#endif /* NILE_IRQ_H_ */
