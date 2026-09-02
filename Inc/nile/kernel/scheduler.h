#ifndef NILE_KERNEL_SCHEDULER_H_
#define NILE_KERNEL_SCHEDULER_H_

#include "nile/stdtypes.h"
#include "nile/kernel/tcb.h"

#define NILE_KERNEL_OS_SCHEDULER_FLAGS_TICKLESS_IDLE_POS   1
#define NILE_KERNEL_OS_SCHEDULER_FLAGS_TICKLESS_IDLE  (1 << NILE_KERNEL_OS_SCHEDULER_FLAGS_TICKLESS_IDLE_POS)
#define NILE_KERNEL_OS_SCHEDULER_FLAGS_TASK_YIELD_RQX_POS  2
#define NILE_KERNEL_OS_SCHEDULER_FLAGS_TASK_YIELD_RQX (1 << NILE_KERNEL_OS_SCHEDULER_FLAGS_TASK_YIELD_RQX_POS)

typedef struct nile_kernel_scheduler {
	uint32_t kernel_space_stack_pointer_base;
	uint32_t cpu_frequency;
	uint32_t os_tick_hardware_timer_interrupt_frequency;
	uint32_t os_tick_real_duration_ns;
	uint32_t os_tick_real_duration_us;
	uint32_t os_tick_counter;
	uint32_t os_tick_prescaler;
	uint32_t os_tick_prescaler_counter;
    volatile uint32_t os_scheduler_flags;
	nile_kernel_tcb* tcb_current_task;
	nile_kernel_tcb* tcb_idle_task;
	nile_kernel_tcb* tcb_arrival_queue_first;
	nile_kernel_tcb* tcb_arrival_queue_last;
	nile_kernel_tcb* tcb_blocked_queue_first;
	nile_kernel_tcb* tcb_blocked_queue_last;
	nile_kernel_tcb* tcb_ready_queue_first;
	nile_kernel_tcb* tcb_ready_queue_last;
	nile_kernel_tcb* tcb_unused_queue_first;
	nile_kernel_tcb* tcb_unused_queue_last;
}nile_kernel_scheduler;

void nile_os_tick();
void nile_os_reschedule_current_task();
void nile_os_scheduler_insert_into_blocked_queue(nile_kernel_tcb *new_task);

#endif /* NILE_KERNEL_SCHEDULER_H_ */
