#include "nile/compiler.h"
#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/kernel/syscall_helper.h"
#include "nile/syscall_api.h"
#include "nile/ctx_switch.h"
#include "nile/irq.h"

uint32_t system_call_task_yield(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	kk->scheduler.os_scheduler_flags |= NILE_KERNEL_OS_SCHEDULER_FLAGS_TASK_YIELD_RQX;
	ctx_switch_irq_trigger();
	return NILE_SYSCALL_RETVAL_OK;
}

uint32_t system_call_task_block(uint32_t *arg_list, uint32_t arg_cnt) {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_task_block *p = (nile_syscall_params_task_block*) arg_list;

	uint32_t os_tick_timeout_release_time = 0;
	uint32_t tcb_blocked_flag = NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED;
	uint32_t block_type;
	uint32_t *block_release_src_ptr;
	uint32_t block_timeout;
	/* Verify args */
	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_TASK_BLOCK) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto skip;
	}

	block_type = p->block_type;
	block_timeout = p->timer_val;
	block_release_src_ptr = (uint32_t*) &p->retval_block_release_src;

	if (*block_release_src_ptr)
		goto skip;
	if (block_type >= NILE_SYSCALL_TASK_BLOCK_TYPE_RESERVED) {
		retval = NILE_SYSCALL_RETVAL_ERR_BAD_TASK_BLOCK_TYPE;
		goto skip;
	}

	/* Args ok */

	if ((block_type == NILE_SYSCALL_TASK_BLOCK_TYPE_IO_TIMEOUT_US) || (block_type == NILE_SYSCALL_TASK_BLOCK_TYPE_DELAY_US)) {
		os_tick_timeout_release_time = (block_timeout / kk->scheduler.os_tick_real_duration_us);
	} else if ((block_type == NILE_SYSCALL_TASK_BLOCK_TYPE_IO_TIMEOUT_MS) || (block_type == NILE_SYSCALL_TASK_BLOCK_TYPE_DELAY_MS)) {
		os_tick_timeout_release_time = (block_timeout * 1000 / (kk->scheduler.os_tick_real_duration_us));
	}

	switch (block_type) {
	case (NILE_SYSCALL_TASK_BLOCK_TYPE_IO):
		tcb_blocked_flag |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO;
		break;
	case (NILE_SYSCALL_TASK_BLOCK_TYPE_IO_TIMEOUT_US):
		tcb_blocked_flag |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY;
		break;
	case (NILE_SYSCALL_TASK_BLOCK_TYPE_IO_TIMEOUT_MS):
		tcb_blocked_flag |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_IO | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY;
		break;
	case (NILE_SYSCALL_TASK_BLOCK_TYPE_DELAY_US):
		tcb_blocked_flag |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY;
		break;
	case (NILE_SYSCALL_TASK_BLOCK_TYPE_DELAY_MS):
		tcb_blocked_flag |= NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_DELAY;
		break;
	default:
		break;
	}

	nile_irq_global_disable();
	if (p->io_op_params) {
		if (p->io_op_params->retval_io_hw_op_finished_code) {
			//operation already finished, no need to block
			p->retval_block_release_src = NILE_SYSCALL_TASK_UNBLOCK_SRC_IO;
			nile_irq_global_enable();
			goto skip;
		}
		kk->scheduler.tcb_current_task->scheduling_blocking_io_op = p->io_op_params->retval_io_op_ptr;
		kk->scheduler.tcb_current_task->scheduling_blocking_io_op_queue = p->io_op_params->retval_io_op_q_ptr;
		kk->scheduler.tcb_current_task->scheduling_blocking_io_op_dev = p->io_op_params->retval_io_dev;
	}

	kk->scheduler.tcb_current_task->scheduling_blocked_os_tick_release_timestamp = kk->scheduler.os_tick_counter + os_tick_timeout_release_time;
	kk->scheduler.tcb_current_task->flags = (kk->scheduler.tcb_current_task->flags
			& ~(NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_RUNNING | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_READY | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED
					| NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_SUSPENDED | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_TERMINATED
					| NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_ARRIVED | NILE_KERNEL_TASK_CONTROL_BLOCK_FLAGS_BLOCKED_ALL_MASK)) | tcb_blocked_flag;
	kk->scheduler.tcb_current_task->scheduling_blocked_release_src = (uint32_t*) block_release_src_ptr;
	nile_os_scheduler_insert_into_blocked_queue(kk->scheduler.tcb_current_task);
	nile_irq_global_enable();
	system_call_task_yield(0, 0);  //blocked task will yield
	skip: return retval;
}

uint32_t system_call_task_reserved0(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_task_reserved1(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_task_reserved2(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_task_reserved3(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_task_reserved4(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_task_reserved5(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
