#ifndef NILE_ARCH_ARMV7M_EXCEPTION_H_
#define NILE_ARCH_ARMV7M_EXCEPTION_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/barriers.h"

#include "nile/irq.h"
#include "nile/irq_priority.h"

#ifndef NILE_MCU_IRQ_EXC_PRIO_SHIFT
#define NILE_MCU_IRQ_EXC_PRIO_SHIFT  (8 - NILE_MCU_IRQ_EXC_PRIO_BITS)
#endif

//enum {
//    EXC_ID_NONE             = -1,
//    EXC_ID_RESET            = 1,
//    EXC_ID_NMI              = 2,
//    EXC_ID_HARDFAULT        = 3,
//    EXC_ID_MEMMANAGE        = 4,
//    EXC_ID_BUSFAULT         = 5,
//    EXC_ID_USAGEFAULT       = 6,
//    /* 7–10 reserved */
//    EXC_ID_SVCALL           = 11,
//    EXC_ID_DEBUGMON         = 12,
//    /* 13 reserved */
//    EXC_ID_PENDSV           = 14,
//    EXC_ID_SYSTICK          = 15
//};

/* ----------------------------- */
/* Helpers                       */
/* ----------------------------- */

static NILE_FORCEINLINE volatile uint32_t* nile_scb_shcsr(void) {
	return &SCB->SHCSR;
}

static NILE_FORCEINLINE volatile uint8_t* nile_scb_shpr_byte(uint32_t exc_num) {
	/* Exceptions 4–15 map to SHPR1–3 */
	uint32_t idx = exc_num - 4U; /* 0..11 */

	if (idx < 4U) {
		return &SCB->SHPR[0] + idx;
	} else if (idx < 8U) {
		return &SCB->SHPR[1] + (idx - 4U);
	} else {
		return &SCB->SHPR[2] + (idx - 8U);
	}
}

static NILE_FORCEINLINE volatile uint32_t* nile_scb_icsr(void) {
	return &SCB->ICSR;
}

static NILE_FORCEINLINE volatile uint32_t* nile_syst_csr(void) {
	return &SysTick->CTRL;
}

/* ----------------------------- */
/* OS exception ID / raw ID      */
/* ----------------------------- */

static NILE_FORCEINLINE nile_exc_id_t nile_exc_get_raw_exception_id(uint32_t nile_exception_id) {
	return nile_exception_id;
}

static NILE_FORCEINLINE uint32_t nile_exc_get_nile_exception_id(nile_exc_id_t raw_exception_id) {
	return raw_exception_id;
}

/* ----------------------------- */
/* Exception enable/disable      */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_exc_enable(nile_exc_id_t id) {
	volatile uint32_t *shcsr = nile_scb_shcsr();
	volatile uint32_t *csr = nile_syst_csr();

	switch (id) {
	case 4: /* MemManage */
		*shcsr |= SCB_SHCSR_MEMFAULTENA_Msk;
		break;
	case 5: /* BusFault */
		*shcsr |= SCB_SHCSR_BUSFAULTENA_Msk;
		break;
	case 6: /* UsageFault */
		*shcsr |= SCB_SHCSR_USGFAULTENA_Msk;
		break;
	case 12: /* DebugMonitor */
		*shcsr |= SCB_SHCSR_MONITORACT_Msk;
		break;
	case 15: /* SysTick */
		*csr |= SysTick_CTRL_TICKINT_Msk;
		break;
	default:
		break;
	}

	nile_dsb();
	nile_isb();
}

static NILE_FORCEINLINE void nile_exc_disable(nile_exc_id_t id) {
	volatile uint32_t *shcsr = nile_scb_shcsr();
	volatile uint32_t *csr = nile_syst_csr();

	switch (id) {
	case 4: /* MemManage */
		*shcsr &= ~SCB_SHCSR_MEMFAULTENA_Msk;
		break;
	case 5: /* BusFault */
		*shcsr &= ~SCB_SHCSR_BUSFAULTENA_Msk;
		break;
	case 6: /* UsageFault */
		*shcsr &= ~SCB_SHCSR_USGFAULTENA_Msk;
		break;
	case 12: /* DebugMonitor */
		*shcsr &= ~SCB_SHCSR_MONITORACT_Msk;
		break;
	case 15: /* SysTick */
		*csr &= ~SysTick_CTRL_TICKINT_Msk;
		break;
	default:
		break;
	}

	nile_dsb();
	nile_isb();
}

/* ----------------------------- */
/* Priority control              */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_exc_set_priority(nile_exc_id_t id, uint32_t priority) {
	if (id < 4 || id > 15) {
		return;
	}

	volatile uint8_t *p = nile_scb_shpr_byte((uint32_t) id);
	*p = (uint8_t) (priority << NILE_MCU_IRQ_EXC_PRIO_SHIFT);

	nile_dsb();
	nile_isb();
}

static NILE_FORCEINLINE uint32_t nile_exc_get_priority(nile_exc_id_t id) {
	if (id < 4 || id > 15) {
		return 0U;
	}

	volatile uint8_t *p = nile_scb_shpr_byte((uint32_t) id);
	return (uint32_t) (*p >> NILE_MCU_IRQ_EXC_PRIO_SHIFT);
}

/* ----------------------------- */
/* Pending / active              */
/* ----------------------------- */

static NILE_FORCEINLINE void nile_exc_set_pending(nile_exc_id_t id) {
	volatile uint32_t *icsr = nile_scb_icsr();

	switch (id) {
	case 2: /* NMI */
		*icsr |= SCB_ICSR_NMIPENDSET_Msk;
		break;
	case 14: /* PendSV */
		*icsr |= SCB_ICSR_PENDSVSET_Msk;
		break;
	case 15: /* SysTick */
		*icsr |= SCB_ICSR_PENDSTSET_Msk;
		break;
	default:
		break;
	}

	nile_dsb();
	nile_isb();
}

static NILE_FORCEINLINE void nile_exc_clear_pending(nile_exc_id_t id) {
	volatile uint32_t *icsr = nile_scb_icsr();

	switch (id) {
	case 14: /* PendSV */
		*icsr |= SCB_ICSR_PENDSVCLR_Msk;
		break;
	case 15: /* SysTick */
		*icsr |= SCB_ICSR_PENDSTCLR_Msk;
		break;
	default:
		break;
	}

	nile_dsb();
	nile_isb();
}

static NILE_FORCEINLINE bool nile_exc_is_active(nile_exc_id_t id) {
	uint32_t vect = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;
	return (vect == (uint32_t) id);
}

/* ----------------------------- */
/* Current exception             */
/* ----------------------------- */

static NILE_FORCEINLINE nile_exc_id_t nile_exc_current(void) {
	uint32_t vect = SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk;

	if (!vect)
		return NILE_EXC_ID_NONE;

	return (nile_exc_id_t) vect;
}

#endif /* NILE_ARCH_ARMV7M_EXCEPTION_H_ */
