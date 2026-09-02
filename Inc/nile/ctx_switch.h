#ifndef NILE_CTX_SWITCH_H_
#define NILE_CTX_SWITCH_H_

#include "nile/compiler.h"
#include "nile/arch.h"
#include "nile/kernel/tcb.h"

static NILE_FORCEINLINE void ctx_switch_irq_trigger();
static NILE_FORCEINLINE void cpu_task_stack_pointer_set(nile_kernel_tcb* tcb);

static NILE_FORCEINLINE void* function_ptr_adjust_for_isa(void* ptr);
static NILE_FORCEINLINE void asm_userspace_priv_exec_mode(void);
static NILE_FORCEINLINE void asm_userspace_unpriv_exec_mode(void);
static NILE_FORCEINLINE void asm_use_banked_stack_ptr_userspace(void);
static NILE_FORCEINLINE void asm_use_banked_stack_ptr_kernelspace(void);
static NILE_FORCEINLINE void asm_enable_lazy_fpu_stacking(void);
static NILE_FORCEINLINE void asm_disable_lazy_fpu_stacking(void);

/* Architecture-specific ctx_switch */
#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/ctx_switch.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/ctx_switch.h"
#else
#error "No ctx_switch implementation for this architecture"
#endif

#endif /* NILE_CTX_SWITCH_H_ */
