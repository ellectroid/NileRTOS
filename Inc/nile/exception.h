#ifndef NILE_EXCEPTION_H_
#define NILE_EXCEPTION_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/arch.h"

/* Universal exception ID type */
typedef int32_t nile_exc_id_t;

/* ----------------------------- */
/* Public portable API           */
/* ----------------------------- */

//Nile exception numbering is based on ARMv7-M exceptions
enum {
	NILE_EXC_ID_NONE = -1,
    NILE_EXC_ID_CRITICAL_EVENT    = 2,
    NILE_EXC_ID_FATAL_ERROR       = 3,
    NILE_EXC_ID_MEMPROT_VIOLATION = 4,
    NILE_EXC_ID_BUS_ERROR         = 5,
    NILE_EXC_ID_BAD_INSTRUCTION   = 6,
    NILE_EXC_ID_SYSCALL           = 11,
    NILE_EXC_ID_CTX_SWITCH        = 14,
    NILE_EXC_ID_SYSTIMER          = 15,
};

static NILE_FORCEINLINE nile_exc_id_t nile_exc_get_raw_exception_id(uint32_t nile_exception_id);
static NILE_FORCEINLINE uint32_t nile_exc_get_nile_exception_id(nile_exc_id_t raw_exception_id);

//These all work with raw (hw-specific) id
static NILE_FORCEINLINE void nile_exc_enable(nile_exc_id_t raw_id);
static NILE_FORCEINLINE void nile_exc_disable(nile_exc_id_t raw_id);

static NILE_FORCEINLINE void nile_exc_set_priority(nile_exc_id_t raw_id, uint32_t raw_priority);
static NILE_FORCEINLINE uint32_t nile_exc_get_priority(nile_exc_id_t raw_id);

static NILE_FORCEINLINE void nile_exc_set_pending(nile_exc_id_t raw_id);
static NILE_FORCEINLINE void nile_exc_clear_pending(nile_exc_id_t raw_id);

static NILE_FORCEINLINE bool nile_exc_is_active(nile_exc_id_t raw_id);

static NILE_FORCEINLINE nile_exc_id_t nile_exc_current(void);

/* ----------------------------- */
/* Architecture-specific impl    */
/* ----------------------------- */

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/exception.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/exception.h"
#else
#error "No exception implementation for this architecture"
#endif

#endif /* NILE_EXCEPTION_H_ */
