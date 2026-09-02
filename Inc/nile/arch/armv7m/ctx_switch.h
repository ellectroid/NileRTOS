#ifndef NILE_ARCH_ARMV7M_CTX_SWITCH_H_
#define NILE_ARCH_ARMV7M_CTX_SWITCH_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/barriers.h"
#include "nile/memlayout.h"

#define FUNC_PTR_ARM_THUMB_BIT	(0x01)

#define CORTEX_M7_CONTROL_REGISTER_THREAD_MODE_UNPRIVILEGED (1 << 0)
#define CORTEX_M7_CONTROL_REGISTER_USE_PSP (1 << 1)

static NILE_FORCEINLINE void ctx_switch_irq_trigger(void)
{
	nile_dsb();
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
	nile_dsb();
	nile_isb();
}

static NILE_FORCEINLINE void cpu_task_stack_pointer_set(nile_kernel_tcb* tcb){
	nile_dsb();
	nile_isb();
	__asm__ volatile ("MSR PSP, %0" : : "r" (tcb->saved_context_gp.SP) : "memory");
	nile_dsb();
	nile_isb();
}

static NILE_FORCEINLINE void* function_ptr_adjust_for_isa(void* ptr)
{
	return (void*)((uintptr_t)ptr | FUNC_PTR_ARM_THUMB_BIT);
}

static NILE_FORCEINLINE void asm_userspace_priv_exec_mode(void)
{
    __asm__ volatile(
        "MRS R0, CONTROL        \n"
        "BIC R0, R0, #1         \n"   /* clear nPRIV - privileged */
        "MSR CONTROL, R0        \n"
        "ISB                    \n"
        : : : "r0"
    );
}

static NILE_FORCEINLINE void asm_userspace_unpriv_exec_mode(void)
{
    __asm__ volatile(
        "MRS R0, CONTROL        \n"
        "ORR R0, R0, #1         \n"   /* set nPRIV - unprivileged */
        "MSR CONTROL, R0        \n"
        "ISB                    \n"
        : : : "r0"
    );
}

static NILE_FORCEINLINE void asm_use_banked_stack_ptr_userspace(void)
{
    __asm__ volatile(
        "MRS R0, CONTROL        \n"
        "ORR R0, R0, #2         \n"   /* set SPSEL - PSP */
        "MSR CONTROL, R0        \n"
        "ISB                    \n"
        : : : "r0"
    );
}

static NILE_FORCEINLINE void asm_use_banked_stack_ptr_kernelspace(void)
{
    __asm__ volatile(
        "MRS R0, CONTROL        \n"
        "BIC R0, R0, #2         \n"   /* clear SPSEL - MSP */
        "MSR CONTROL, R0        \n"
        "ISB                    \n"
        : : : "r0"
    );
}

#define ASM_ENABLE_LAZY_FPU_STACKING \
    "MOVW R0, #0xEF34        \n" \
    "MOVT R0, #0xE000        \n" \
    "LDR  R1, [R0]           \n" \
    "ORR  R1, R1, #(1 << 30) \n" \
    "ORR  R1, R1, #(1 << 31) \n" \
    "STR  R1, [R0]           \n" \
    "DSB                     \n" \
    "ISB                     \n"

#define ASM_DISABLE_LAZY_FPU_STACKING \
    "MOVW R0, #0xEF34        \n" \
    "MOVT R0, #0xE000        \n" \
    "LDR  R1, [R0]           \n" \
    "BIC  R1, R1, #(1 << 30) \n" \
    "BIC  R1, R1, #(1 << 31) \n" \
    "STR  R1, [R0]           \n" \
    "DSB                     \n" \
    "ISB                     \n"

static NILE_FORCEINLINE void asm_enable_lazy_fpu_stacking(void)
{
    __asm__ volatile(
        ASM_ENABLE_LAZY_FPU_STACKING
        : : : "r0", "r1"
    );
}

static NILE_FORCEINLINE void asm_disable_lazy_fpu_stacking(void)
{
    __asm__ volatile(
        ASM_DISABLE_LAZY_FPU_STACKING
        : : : "r0", "r1"
    );
}



#endif /* NILE_ARCH_ARMV7M_CTX_SWITCH_H_ */
