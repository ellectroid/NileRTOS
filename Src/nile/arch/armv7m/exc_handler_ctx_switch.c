#include "nile/stdtypes.h"
#include "nile/kernel_config.h"
#include "nile/compiler.h"
#include "nile/barriers.h"
#include "nile/memprot.h"
#include "nile/kernel.h"
#include "nile/kernel/tcb.h"
#include "nile/ctx_switch.h"
#include "nile/cache.h"
#include "nile/external_cache.h"
#include "nile/fpu.h"

static NILE_USED NILE_NOINLINE uint64_t ctx_switch_get_current_task_ctx_save_destination_gp_fpu(void);
static NILE_USED NILE_NOINLINE uint64_t ctx_switch_get_new_task_ctx_restore_src_set_new_current_task();
static NILE_USED NILE_NOINLINE void ctx_update_fpu_settings();
static NILE_USED NILE_NOINLINE void ctx_update_memprot_settings();

NILE_NAKED NILE_USED void PendSV_Handler(void) {

	__asm__ volatile("CPSID   i");
	__asm__ volatile("PUSH    {R12, LR}");
	__asm__ volatile("BL      ctx_switch_get_current_task_ctx_save_destination_gp_fpu");
	__asm__ volatile("PUSH    {R0, R1}");
	__asm__ volatile("BL      ctx_switch_get_new_task_ctx_restore_src_set_new_current_task");
	__asm__ volatile("POP     {R2, R3}");
	__asm__ volatile("POP     {R12, LR}");
	/*
	 R0 - restore GP ctx addr
	 R1 - restore FPU ctx addr (0 if no FPU ctx)
	 R2 - save GP ctx addr
	 R3 - save FPU ctx addr (0 if no FPU ctx) */

	__asm__ volatile("CMP     R0, R2");
	__asm__ volatile("BEQ     skip_context_switch");

	//Ctx switch necessary, tripping lazy stacking
	__asm__ volatile(
			"CMP     R3, #0\n"
			"BEQ     after_tripping_lazy_stacking\n" //if no FPU ctx to save, skip the instruction
			"VMOV.F32 S0, S0 \n"
			"after_tripping_lazy_stacking:"
	);

	//while stacking is happening...
	/* Set EXC_RETURN */
	__asm__ volatile("MOVW    LR, #0xFFFD");
	__asm__ volatile(
			"CMP     R1, #0     \n"
			"IT      NE         \n" //there is FPU ctx to restore
			"MOVWNE    LR, #0xFFED\n"
			"MOVT    LR, #0xFFFF"
	);

	/* Save old GP registers R4-R11 to [R2] */
	__asm__ volatile("STMIA   R2!, {R4-R11}");
	/* Restore new GP registers from [R0] */
	__asm__ volatile("LDMIA   R0!, {R4-R11}");

	/* Save old PSP */
	__asm__ volatile("MRS     R12, PSP");
	__asm__ volatile("STR     R12, [R2]");

	/* Save FPU registers if R3 != 0 */
	__asm__ volatile("CMP     R3, #0");
	__asm__ volatile("BEQ     skip_fpu_save");
	__asm__ volatile("VSTMIA  R3!, {S16-S31}");
	__asm__ volatile("skip_fpu_save:");


	/* Restore new PSP */
	__asm__ volatile ("DSB 0xF" ::: "memory"); //lazy stacking finished guarantee
	__asm__ volatile("LDR     R12, [R0]");
	__asm__ volatile("MSR     PSP, R12");

	/* Restore FPU registers if R1 != 0 */
	__asm__ volatile("CMP     R1, #0");
	__asm__ volatile("BEQ     skip_restore_fpu");
	__asm__ volatile("VLDMIA  R1!, {S16-S31}");
	__asm__ volatile("skip_restore_fpu:");

	/* Update MPU & FPU settings */
	__asm__ volatile("PUSH    {R12, LR}");
	__asm__ volatile("BL      ctx_update_memprot_settings");
	__asm__ volatile("BL      ctx_update_fpu_settings");
	__asm__ volatile("POP     {R12, LR}");

	__asm__ volatile("skip_context_switch:");
	__asm__ volatile ("DSB 0xF" ::: "memory");
	__asm__ volatile("CPSIE   i");
	__asm__ volatile("BX      LR");
}

static NILE_USED NILE_NOINLINE uint64_t ctx_switch_get_current_task_ctx_save_destination_gp_fpu() {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	uint64_t retval = (uint64_t) (uintptr_t) &kk->scheduler.tcb_current_task->saved_context_gp;
	kk->scheduler.tcb_current_task->flags &= ~NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_RUNNING;
	if (kk->scheduler.tcb_current_task->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_USES_FPU) {
		retval |= ((uint64_t) (uintptr_t) &((nile_kernel_tcb_fpu*) (kk->scheduler.tcb_current_task))->saved_context_fpu) << 32;
	}
	if(kk->scheduler.os_scheduler_flags & NILE_KERNEL_OS_SCHEDULER_FLAGS_TASK_YIELD_RQX){
		if(!(kk->scheduler.tcb_current_task->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED)){
			nile_os_reschedule_current_task();
		}
	}
	return retval;
}

static NILE_USED NILE_NOINLINE uint64_t ctx_switch_get_new_task_ctx_restore_src_set_new_current_task() {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	nile_kernel_tcb *new_task = kk->scheduler.tcb_ready_queue_first;
	if(kk->scheduler.os_scheduler_flags & NILE_KERNEL_OS_SCHEDULER_FLAGS_TASK_YIELD_RQX){
		//consume the flag
		kk->scheduler.os_scheduler_flags &= ~NILE_KERNEL_OS_SCHEDULER_FLAGS_TASK_YIELD_RQX;
		new_task = NULLPTR;
	}
	if (!new_task)
		new_task = kk->scheduler.tcb_idle_task;
	else {
		//only if new task is not the same as current task
		//if (kk->scheduler.tcb_current_task != new_task) {

		//remove it from the ready queue
		nile_kernel_tcb *temp = kk->scheduler.tcb_ready_queue_first;
		kk->scheduler.tcb_ready_queue_first = (nile_kernel_tcb*) kk->scheduler.tcb_ready_queue_first->tcb_next;
		temp->tcb_next = NULLPTR;
		if (kk->scheduler.tcb_ready_queue_first)
			kk->scheduler.tcb_ready_queue_first->tcb_prev = NULLPTR;
		if (!kk->scheduler.tcb_ready_queue_first)
			kk->scheduler.tcb_ready_queue_last = NULLPTR; //was last
		//}
	}
	uint64_t retval = (uint64_t) (uintptr_t) &new_task->saved_context_gp;
	if ((new_task->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_USES_FPU)) {
		retval |= ((uint64_t) (uintptr_t) &((nile_kernel_tcb_fpu*) (new_task))->saved_context_fpu) << 32;
	}

	new_task->flags &= ~(NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_READY | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED
			| NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_SUSPENDED);
	new_task->flags |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_RUNNING;
	kk->scheduler.tcb_current_task = new_task;
	return retval;
}

static NILE_USED NILE_NOINLINE void ctx_update_fpu_settings() {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	if ((kk->scheduler.tcb_current_task->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_USES_FPU)) {
		nile_fpu_enable_stacking();
		if ((kk->scheduler.tcb_current_task->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_FPU_LAZY_STACKING)) {
			nile_fpu_enable_lazy_stacking();
		} else {
			nile_fpu_disable_lazy_stacking();
		}
	} else {
		nile_fpu_disable_stacking();
	}

}

static NILE_USED NILE_NOINLINE void ctx_update_memprot_settings() {
	apply_memprot_settings(((nile_kernel*) NILE_MEMORY_KERNEL_ADDR)->scheduler.tcb_current_task);
}

