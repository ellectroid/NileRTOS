#ifndef NILE_CTX_INIT_H_
#define NILE_CTX_INIT_H_

#include "nile/kernel/tcb.h"

/* ----------------------------- */
/* Public portable API           */
/* ----------------------------- */

static NILE_FORCEINLINE void ctx_init_gp(nile_kernel_tcb *tcb, nile_kernel_tcb_startup_frame_gp *ctx_val);
static NILE_FORCEINLINE void ctx_init_gp_fpu(nile_kernel_tcb_fpu *tcb, nile_kernel_tcb_startup_frame_gp_fpu *ctx_val);

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/ctx_init.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/ctx_init.h"
#else
#error "No CPU context initialization implementation for this architecture"
#endif


#endif /* NILE_CTX_INIT_H_ */
