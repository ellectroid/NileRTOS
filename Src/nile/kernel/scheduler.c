#include "nile/stdtypes.h"
#include "nile/kernel_config.h"
#include "nile/compiler.h"
#include "nile/irq.h"
#include "nile/barriers.h"
#include "nile/ctx_switch.h"
#include "nile/kernel.h"
#include "nile/kernel/tcb_queue.h"
#include "nile/syscall_api.h"

static void nile_os_tick_handle_os_tick_overflow(nile_kernel *kk);
static NILE_FORCEINLINE void nile_os_tick_update_ready_queue_wait_times(nile_kernel *kk);
static void nile_os_tick_reschedule_unblocked_tasks(nile_kernel *kk);
static NILE_FORCEINLINE void nile_os_tick_current_task_decrement_burst_time(nile_kernel *kk);
static void nile_os_tick_startup_pick_idle_task(nile_kernel *kk);
static NILE_FORCEINLINE int nile_os_task_switch_needed(nile_kernel *kk);

//Public functions:
void nile_os_reschedule_current_task();
void nile_os_scheduler_insert_into_ready_queue(nile_kernel_tcb *new_task);
void nile_os_scheduler_insert_into_blocked_queue(nile_kernel_tcb *new_task);

void nile_os_tick() { //call from hw tick source handler (e.g. systick/timer interrupt handler)
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	kk->scheduler.os_tick_prescaler_counter--;
	if (kk->scheduler.os_tick_prescaler_counter > 0)
		return;
	kk->scheduler.os_tick_prescaler_counter = kk->scheduler.os_tick_prescaler;
	kk->scheduler.os_tick_counter++;
	if (kk->scheduler.os_scheduler_flags & NILE_KERNEL_OS_SCHEDULER_FLAGS_TASK_YIELD_RQX) {
		return; //let context switch IRQ handle this (already triggered) and clear the flag
	}
	nile_irq_global_disable();
	int need_to_switch_task = 0;
	if (kk->scheduler.os_tick_counter & ~NILE_SCHEDULER_OS_TICK_OVERFLOW_LEVEL) {
		nile_os_tick_handle_os_tick_overflow(kk);
	}

	nile_os_tick_update_ready_queue_wait_times(kk);
	nile_os_tick_reschedule_unblocked_tasks(kk);
	if (kk->scheduler.tcb_current_task)
		nile_os_tick_current_task_decrement_burst_time(kk);
	else {
		nile_os_tick_startup_pick_idle_task(kk);
		need_to_switch_task = 1;
	}

	if (kk->scheduler.tcb_current_task->scheduling_burst_time_remaining == 0) {
		nile_os_reschedule_current_task();
		need_to_switch_task = nile_os_task_switch_needed(kk);
	}

	if (need_to_switch_task) {
		ctx_switch_irq_trigger();
	}
	nile_irq_global_enable();
}

static void nile_os_tick_handle_os_tick_overflow(nile_kernel *kk) {
	//get reset value
	uint32_t os_tick_reset_val = kk->scheduler.os_tick_counter;
	kk->scheduler.os_tick_counter = 0;

	//go through all delay-blocked tasks and subtract their unblocking times
	nile_kernel_tcb *blocked_it = kk->scheduler.tcb_blocked_queue_last;
	while (blocked_it != NULLPTR) {
		if (!(blocked_it->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY))
			break;
		blocked_it->scheduling_blocked_os_tick_release_timestamp -= os_tick_reset_val;
		blocked_it = (nile_kernel_tcb*) blocked_it->tcb_prev;
	}
}

static void ready_queue_dynamic_priority_update(nile_kernel *kk, nile_kernel_tcb *task) {
	tcb_queue_tcb_remove(&kk->scheduler.tcb_ready_queue_first, &kk->scheduler.tcb_ready_queue_last, task);

	nile_kernel_tcb *ready_it = kk->scheduler.tcb_ready_queue_first;
	while (ready_it != NULLPTR) {
		if (ready_it->scheduling_priority_current < task->scheduling_priority_current) {
			break;
		}
		ready_it = (nile_kernel_tcb*) ready_it->tcb_next;
	}
	if (ready_it)
		tcb_queue_tcb_add_before(&kk->scheduler.tcb_ready_queue_first, &kk->scheduler.tcb_ready_queue_last, task, ready_it);
	else
		tcb_queue_tcb_add_after(&kk->scheduler.tcb_ready_queue_first, &kk->scheduler.tcb_ready_queue_last, task, ready_it);
}

static NILE_FORCEINLINE void nile_os_tick_update_ready_queue_wait_times(nile_kernel *kk) {
	nile_kernel_tcb *ready_it = kk->scheduler.tcb_ready_queue_first;
	while (ready_it) {
		uint32_t task_current_priority = ready_it->scheduling_priority_current;
		ready_it->scheduling_ready_queue_wait_time_accumulator++;
		if (ready_it->scheduling_dynamic_priority_wait_time_prescaler) {
			ready_it->scheduling_dynamic_priority_wait_time_counter++;
			if (ready_it->scheduling_dynamic_priority_wait_time_counter >= ready_it->scheduling_dynamic_priority_wait_time_prescaler) {
				ready_it->scheduling_priority_current += ready_it->scheduling_dynamic_priority_step;
				ready_it->scheduling_dynamic_priority_wait_time_counter = 0;
			}
		}
		if (task_current_priority != ready_it->scheduling_priority_current) {
			ready_queue_dynamic_priority_update(kk, ready_it);
		}
		ready_it = (nile_kernel_tcb*) ready_it->tcb_next;
	}
}

static void reschedule_blocked_task(nile_kernel *kk, nile_kernel_tcb *task) {
	tcb_queue_tcb_remove(&kk->scheduler.tcb_blocked_queue_first, &kk->scheduler.tcb_blocked_queue_last, task);
	task->flags &= ~NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_ALL_MASK;
	task->flags |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_READY;
	task->scheduling_blocked_release_src = NULLPTR;
	task->scheduling_blocked_os_tick_release_timestamp = 0;
//	task->scheduling_arrival_queue_release_timestamp = 0;
	task->scheduling_priority_current = task->scheduling_priority_base;
	task->scheduling_ready_queue_wait_time_accumulator = 0;
	task->scheduling_dynamic_priority_wait_time_counter = 0;
	task->scheduling_burst_time_remaining = task->scheduling_burst_time_base;

	nile_os_scheduler_insert_into_ready_queue(task);
}

static void nile_os_tick_reschedule_unblocked_tasks(nile_kernel *kk) {
	uint32_t now = kk->scheduler.os_tick_counter;
	nile_kernel_tcb *it;

	/* --- Subregion 1: IO-only blocked --- */
	it = kk->scheduler.tcb_blocked_queue_first;
	while (it && (it->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO) && !(it->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY)) {

		nile_kernel_tcb *next = (nile_kernel_tcb*) it->tcb_next;

		volatile nile_kernel_io_dev_op *op = it->scheduling_blocking_io_op;
		volatile uint32_t *op_hw_retval = op->io_hw_op_finished_code_ptr;
		if (*op_hw_retval) {
			*(it->scheduling_blocked_release_src) = NILE_SYSCALL_TASK_UNBLOCK_SRC_IO;
			reschedule_blocked_task(kk, it);
		}

		it = next;
	}

	/* --- Subregion 2: IO + timeout blocked --- */
	int subregion2_check_timestamp = 1;

	while (it && (it->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO) && (it->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY)) {

		nile_kernel_tcb *next = (nile_kernel_tcb*) it->tcb_next;
		int unblock = 0;

		/* IO release always checked */
		volatile nile_kernel_io_dev_op *op = it->scheduling_blocking_io_op;
		volatile uint32_t *op_hw_retval = op->io_hw_op_finished_code_ptr;
		if (*op_hw_retval) {
			*(it->scheduling_blocked_release_src) = NILE_SYSCALL_TASK_UNBLOCK_SRC_IO;
			unblock = 1;
		}

		/* Timeout release checked only while flag is true */
		if (!unblock && subregion2_check_timestamp) {
			uint32_t ts = it->scheduling_blocked_os_tick_release_timestamp;

			if (ts <= now) {
				//check current operation, if it's already active, we let it finish
				nile_kernel_io_dev_op *op = it->scheduling_blocking_io_op;
				io_op_ringbuffer_queue *op_q = it->scheduling_blocking_io_op_queue;
				nile_kernel_io_char_dev *dev_char = it->scheduling_blocking_io_op_dev;
				nile_kernel_io_block_dev *dev_block = it->scheduling_blocking_io_op_dev;
				uint32_t dev_is_char_dev = dev_char->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_CHAR_DEV;

				if (op->flags & NILE_KERNEL_IO_DEV_OP_FLAG_ACTIVE) {
					//not changing release timestamp (would need to sort)
					//no need to reinsert, IO should be done soon
					//the first operation in the queue, if it's TX, is immediately active
				} else {
					//Pending flag - not active yet
					//abort the IO operation
					//unblock the task
					//The first operation in the queue, if it's RX, is pending until the first data in
					op->flags = (op->flags | NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED) & ~NILE_KERNEL_IO_DEV_OP_FLAG_PENDING;
					*op->io_hw_op_finished_code_ptr = IO_HW_OP_FINISHED_CODE_ERRCODE_ABORTED << IO_HW_OP_FINISHED_CODE_ERRCODE_POS;
					uint32_t is_char_dev_write_queue = op_q->flags & IO_OP_RING_BUFFER_FLAGS_IS_TX;
					*(it->scheduling_blocked_release_src) = NILE_SYSCALL_TASK_UNBLOCK_SRC_TIMEOUT;
					unblock = 1;
					if (dev_is_char_dev) {
						nile_kernel_io_char_dev_op *next_op = io_char_dev_queue_peek((nile_kernel_io_char_dev_op_queue*) op_q);
						while (next_op && (next_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED)) {
							*next_op->op.io_hw_op_finished_code_ptr = IO_HW_OP_FINISHED_CODE_ERRCODE_ABORTED << IO_HW_OP_FINISHED_CODE_ERRCODE_POS;
							io_char_dev_queue_pop((nile_kernel_io_char_dev_op_queue*) op_q);
							next_op = io_char_dev_queue_peek((nile_kernel_io_char_dev_op_queue*) op_q);
						}
						if (!next_op) {
							//queue empty
							if (is_char_dev_write_queue) {
								if (dev_char->hw_tx_stop) {
									dev_char->hw_tx_stop(dev_char);
								}
								dev_char->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE;
								if (!(dev_char->flags & (NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE | NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE))) {
									dev_char->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE;
								}
							} else {
								if (dev_char->hw_rx_stop) {
									dev_char->hw_rx_stop(dev_char);
								}

								dev_char->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE;
								if (!(dev_char->flags & (NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE | NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE))) {
									dev_char->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE;
								}

							}

						}
					} else {
						nile_kernel_io_block_dev_op *next_op = io_block_dev_queue_peek((nile_kernel_io_block_dev_op_queue*) op_q);
						while (next_op && (next_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED)) {
							*op->io_hw_op_finished_code_ptr = IO_HW_OP_FINISHED_CODE_ERRCODE_ABORTED << IO_HW_OP_FINISHED_CODE_ERRCODE_POS;
							io_block_dev_queue_pop((nile_kernel_io_block_dev_op_queue*) op_q);
							next_op = io_block_dev_queue_peek((nile_kernel_io_block_dev_op_queue*) op_q);
						}
						if (!next_op) {
							//queue empty
							if (dev_block->hw_stop)
								dev_block->hw_stop(dev_block);
						}
					}

				}
			} else {
				/* first too-high timestamp - stop timeout checks, but keep scanning IO */
				subregion2_check_timestamp = 0;
			}
		}

		if (unblock)
			reschedule_blocked_task(kk, it);

		it = next;
	}

	/* --- Subregion 3: timeout-only blocked (backwards) --- */
	nile_kernel_tcb *rev = kk->scheduler.tcb_blocked_queue_last;
	while (rev && (rev->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY) && !(rev->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO)) {

		nile_kernel_tcb *prev = (nile_kernel_tcb*) rev->tcb_prev;
		uint32_t ts = rev->scheduling_blocked_os_tick_release_timestamp;

		if (ts <= now) {
			*(rev->scheduling_blocked_release_src) = NILE_SYSCALL_TASK_UNBLOCK_SRC_TIMEOUT;
			reschedule_blocked_task(kk, rev);
		} else {
			break; // descending timestamps - all earlier ones are even higher
		}
		rev = prev;
	}
}

static NILE_FORCEINLINE void nile_os_tick_current_task_decrement_burst_time(nile_kernel *kk) {
	kk->scheduler.tcb_current_task->scheduling_burst_time_remaining--;
}

static void nile_os_tick_startup_pick_idle_task(nile_kernel *kk) {
	kk->scheduler.tcb_current_task = kk->scheduler.tcb_idle_task;
	apply_memprot_settings(kk->scheduler.tcb_current_task);
	kk->scheduler.tcb_current_task->scheduling_burst_time_remaining = 1;
	cpu_task_stack_pointer_set(kk->scheduler.tcb_idle_task);
}

static NILE_FORCEINLINE int nile_os_task_switch_needed(nile_kernel *kk) {
	int retval_need_to_switch_task = 0;
//check if there is anything in the ready queue
	if (kk->scheduler.tcb_ready_queue_first) {
		//if it's not the current task
		if (kk->scheduler.tcb_ready_queue_first != kk->scheduler.tcb_current_task) {
			retval_need_to_switch_task = 1;
		}
	} else {
		//if ready queue empty, we need to switch only if are switching out non-idle task to idle task
		if (kk->scheduler.tcb_current_task != kk->scheduler.tcb_idle_task) {
			retval_need_to_switch_task = 1;
		}
	}
	return retval_need_to_switch_task;
}

void nile_os_reschedule_current_task() {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	nile_os_scheduler_insert_into_ready_queue(kk->scheduler.tcb_current_task);
}

void nile_os_scheduler_insert_into_ready_queue(nile_kernel_tcb *new_task) {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;

	nile_kernel_tcb *ready_it = kk->scheduler.tcb_ready_queue_first;

//reset the burst time
	new_task->scheduling_burst_time_remaining = new_task->scheduling_burst_time_base;

//Reinsert into the ready queue
	if (new_task != kk->scheduler.tcb_idle_task) {
		new_task->scheduling_priority_current = new_task->scheduling_priority_base;
		new_task->scheduling_ready_queue_wait_time_accumulator = 0;
		new_task->scheduling_dynamic_priority_wait_time_counter = 0;

		while (ready_it != NULLPTR) {
			if (ready_it->scheduling_priority_current < new_task->scheduling_priority_current) {
				//current takes its place in the queue
				break;
			}
			if(ready_it == new_task){
				return; //task already in the queue
			}
			ready_it = (nile_kernel_tcb*) ready_it->tcb_next;
		}
		if (ready_it)
			tcb_queue_tcb_add_before(&kk->scheduler.tcb_ready_queue_first, &kk->scheduler.tcb_ready_queue_last, new_task, ready_it);
		else
			tcb_queue_tcb_add_after(&kk->scheduler.tcb_ready_queue_first, &kk->scheduler.tcb_ready_queue_last, new_task, ready_it);
	}
}

void nile_os_scheduler_insert_into_blocked_queue(nile_kernel_tcb *new_task) {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;

	nile_kernel_tcb *it = kk->scheduler.tcb_blocked_queue_first;

	uint32_t flags = new_task->flags;
	uint32_t release_ts = new_task->scheduling_blocked_os_tick_release_timestamp;

	/* Parameter guard: ensure task is actually blocked */
	if (!(flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED)) {
		return; /* Not blocked at all */
	}
	if (!(flags & (NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO |
	NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY))) {
		return; /* No specific blocked reason → cannot insert */
	}
	/* ---------------------------------------------------------
	 * Subregion 1: IO-blocked only (no timeout)
	 * Condition: BLOCKED_IO set, BLOCKED_DELAY not set
	 * Insert at the very front.
	 * --------------------------------------------------------- */
	if ((flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO) && !(flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY)) {

		tcb_queue_tcb_add_before(&kk->scheduler.tcb_blocked_queue_first, &kk->scheduler.tcb_blocked_queue_last, new_task,
				kk->scheduler.tcb_blocked_queue_first);
		return;
	}

	/* ---------------------------------------------------------
	 * Subregion 2: IO + timeout
	 * Condition: BLOCKED_IO set, BLOCKED_DELAY set
	 * Sorted ascending by release timestamp.
	 * Insert after subregion 1.
	 * --------------------------------------------------------- */
	if ((flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO) && (flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY)) {

		/* Skip subregion 1 */
		while (it && ((it->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO) && !(it->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY))) {
			it = (nile_kernel_tcb*) it->tcb_next;
		}

		/* Now insert in ascending order */
		while (it && (it->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO) && (it->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY)
				&& it->scheduling_blocked_os_tick_release_timestamp <= release_ts) {
			it = (nile_kernel_tcb*) it->tcb_next;
		}

		if (it)
			tcb_queue_tcb_add_before(&kk->scheduler.tcb_blocked_queue_first, &kk->scheduler.tcb_blocked_queue_last, new_task, it);
		else
			tcb_queue_tcb_add_after(&kk->scheduler.tcb_blocked_queue_first, &kk->scheduler.tcb_blocked_queue_last, new_task,
			NULLPTR);
		return;
	}

	/* ---------------------------------------------------------
	 * Subregion 3: timeout-only
	 * Condition: BLOCKED_DELAY set, BLOCKED_IO not set
	 * Sorted descending by release timestamp.
	 * Insert at the back.
	 * --------------------------------------------------------- */
	/* Subregion 3: timeout-only (descending order) */
	if ((flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY) && !(flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO)) {

		nile_kernel_tcb *rev = kk->scheduler.tcb_blocked_queue_last;

		/* Walk backwards while rev_ts <= new_ts */
		while (rev && (rev->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY) && !(rev->flags & NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO)
				&& rev->scheduling_blocked_os_tick_release_timestamp <= release_ts) {
			rev = (nile_kernel_tcb*) rev->tcb_prev;
		}
		if (rev) {
			/* Insert AFTER rev (rev_ts > new_ts) */
			tcb_queue_tcb_add_after(&kk->scheduler.tcb_blocked_queue_first, &kk->scheduler.tcb_blocked_queue_last, new_task, rev);
		} else {
			/* new_task has the largest timestamp - insert at front of subregion 3 */
			tcb_queue_tcb_add_before(&kk->scheduler.tcb_blocked_queue_first, &kk->scheduler.tcb_blocked_queue_last, new_task,
					kk->scheduler.tcb_blocked_queue_first);
		}

		return;
	}
}

