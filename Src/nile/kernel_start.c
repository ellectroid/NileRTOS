#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/kernel.h"
#include "nile/fpu.h"
#include "nile/cache.h"
#include "nile/external_cache.h"
#include "nile/sys_clock.h"
#include "nile/memprot.h"
#include "nile/os_start.h"
#include "nile/exception.h"

NILE_NORETURN void nile_kernel_start() {

	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	nile_kernel_tcb *idle_task = kk->scheduler.tcb_idle_task;

	/* Step 1: initializing exceptions */
	nile_exc_enable(nile_exc_get_raw_exception_id(NILE_EXC_ID_CTX_SWITCH));
	nile_exc_set_priority(nile_exc_get_raw_exception_id(NILE_EXC_ID_CTX_SWITCH), nile_irq_get_raw_priority_value(NILE_IRQ_PRIO_LVL_SYS_CTX_SWITCH));

	nile_exc_enable(nile_exc_get_raw_exception_id(NILE_EXC_ID_SYSCALL));
	nile_exc_set_priority(nile_exc_get_raw_exception_id(NILE_EXC_ID_SYSCALL), nile_irq_get_raw_priority_value(NILE_IRQ_PRIO_LVL_SYS_SYSCALL));

	nile_exc_enable(nile_exc_get_raw_exception_id(NILE_EXC_ID_MEMPROT_VIOLATION));
	nile_exc_set_priority(nile_exc_get_raw_exception_id(NILE_EXC_ID_MEMPROT_VIOLATION), nile_irq_get_raw_priority_value(NILE_IRQ_PRIO_LVL_SYS_EXC));

	/* Step 2: enabling cache */
	nile_cache_invalidate_dcache_all();
	nile_cache_invalidate_icache_all();
	nile_cache_enable_dcache();
	nile_cache_enable_icache();
	nile_external_cache_enable();

	/* Step 3: enable FPU */
	nile_fpu_enable_full_access();

	/* Step 4: configuring OS tick timer */
	nile_sys_clock_counter_overflow_val_write((kk->scheduler.cpu_frequency / 1000U) - 1U);
	nile_sys_clock_counter_write(nile_sys_clock_counter_overflow_val_read() - 100U);
	nile_sys_clock_src_set(NILE_SYS_CLOCK_SRC_CPU);
	nile_exc_enable(nile_exc_get_raw_exception_id(NILE_EXC_ID_SYSTIMER));
	nile_exc_set_priority(nile_exc_get_raw_exception_id(NILE_EXC_ID_SYSTIMER), nile_irq_get_raw_priority_value(NILE_IRQ_PRIO_LVL_SYS_TICK_SRC));

	/* Step 5: preparing for dispatch */
	uint32_t kernel_sp = kk->scheduler.kernel_space_stack_pointer_base;
	uint32_t task_sp = tcb_get_sp(&idle_task->saved_context_gp);
	os_start(kernel_sp, task_sp);

	while (1);
}
