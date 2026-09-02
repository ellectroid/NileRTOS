#include "nile/compiler.h"
#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/kernel/syscall_helper.h"
#include "nile/kernel/io_op_queue.h"
#include "nile/kernel/io_dev.h"
#include "nile/irq.h"

#include "nile/syscall_api.h"

uint32_t system_call_io_char_dev_open(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk;
	nile_kernel_io_char_dev *dev;
	nile_kernel_tcb *current;

	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto done;
	}

	if (p->dev_id >= NILE_KERNEL_IO_CHAR_DEV_COUNT) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
		goto done;
	}
	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	dev = &kk->io_char_dev[p->dev_id];
	if (!(dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY;
		goto done;
	}
	current = kk->scheduler.tcb_current_task;
	if (dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_SHARED) {
		if (dev->tcb_owner == NULLPTR) {
			dev->tcb_owner = current;
		}
		goto done;
	}
	if (dev->tcb_owner == NULLPTR || dev->tcb_owner == current) {
		dev->tcb_owner = current;
		goto done;
	}
	retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY;
	done: return retval;
}

uint32_t system_call_io_char_dev_close(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk;
	nile_kernel_io_char_dev *dev;
	nile_kernel_tcb *current;
	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto done;
	}

	if (p->dev_id >= NILE_KERNEL_IO_CHAR_DEV_COUNT) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
		goto done;
	}
	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	dev = &kk->io_char_dev[p->dev_id];
	if (!(dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY;
		goto done;
	}
	current = kk->scheduler.tcb_current_task;
	if (dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_SHARED) {
		if (dev->tcb_owner == current) {
			dev->tcb_owner = NULLPTR;
		}
		goto done;
	}

	if (dev->tcb_owner == NULLPTR || dev->tcb_owner == current) {
		dev->tcb_owner = NULLPTR;
		goto done;
	}
	retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY;
	done: return retval;
}
uint32_t system_call_io_char_dev_read(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk;
	nile_kernel_io_char_dev *dev;
	nile_kernel_tcb *current;
	uint32_t queue_push_retval;

	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto done;
	}

	if (p->dev_id >= NILE_KERNEL_IO_CHAR_DEV_COUNT) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
		goto done;
	}

	p->op_flags = (p->op_flags & ~NILE_KERNEL_IO_DEV_OP_FLAG_SYSTEM_MASK) | NILE_KERNEL_IO_DEV_OP_FLAG_READ;

	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	dev = &kk->io_char_dev[p->dev_id];
	current = kk->scheduler.tcb_current_task;

	if (p->data_len == 0) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OPERATION_LENGTH_0;
		goto done;
	}

	if (!syscall_mem_range_in_curr_task_rw_mem(p->data_buffer, (void*) ((uint8_t*) p->data_buffer + p->data_len - 1))) {
		retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_SHARED)) {
		if (dev->tcb_owner == NULLPTR) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_OPEN;
			goto done;
		}
		if (dev->tcb_owner != current) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY;
			goto done;
		}
	}

	if (!(dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_RX_ENABLED)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_MODE_NOT_SUPPORTED;
		goto done;
	}

	nile_irq_global_disable();
	queue_push_retval = io_char_dev_queue_push(dev->op_queue_rx, p, current);
	if (queue_push_retval) {
		nile_kernel_io_char_dev_op *slot = io_char_dev_queue_peek(dev->op_queue_rx);
		p->retval_io_op_ptr = (uint32_t*) &slot->op;
		p->retval_io_op_q_ptr = (void*) dev->op_queue_rx;
		p->retval_io_dev = (void*) dev;
		if (io_char_dev_queue_level(dev->op_queue_rx) == 1) {
			if (dev->hw_rx_start)
				dev->hw_rx_start(dev);
		}
	} else {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OP_QUEUE_FULL;
	}
	nile_irq_global_enable();
	done: return retval;
}

uint32_t system_call_io_char_dev_write(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk;
	nile_kernel_io_char_dev *dev;
	nile_kernel_tcb *current;
	uint32_t queue_push_retval;

	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto done;
	}

	if (p->dev_id >= NILE_KERNEL_IO_CHAR_DEV_COUNT) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
		goto done;
	}

	p->op_flags = (p->op_flags & ~NILE_KERNEL_IO_DEV_OP_FLAG_SYSTEM_MASK) | NILE_KERNEL_IO_DEV_OP_FLAG_WRITE;

	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	dev = &kk->io_char_dev[p->dev_id];
	current = kk->scheduler.tcb_current_task;

	if (p->data_len == 0) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OPERATION_LENGTH_0;
		goto done;
	}

	if (!syscall_mem_range_in_curr_task_rw_mem(p->data_buffer, (void*) ((uint8_t*) p->data_buffer + p->data_len - 1))) {
		retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_SHARED)) {
		if (dev->tcb_owner == NULLPTR) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_OPEN;
			goto done;
		}
		if (dev->tcb_owner != current) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY;
			goto done;
		}
	}

	if (!(dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_TX_ENABLED)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_MODE_NOT_SUPPORTED;
		goto done;
	}

	nile_irq_global_disable();
	queue_push_retval = io_char_dev_queue_push(dev->op_queue_tx, p, current);
	if (queue_push_retval) {
		nile_kernel_io_char_dev_op *slot = io_char_dev_queue_peek(dev->op_queue_tx);
		p->retval_io_op_ptr = (uint32_t*) &slot->op;
		p->retval_io_op_q_ptr = (io_op_ringbuffer_queue*) dev->op_queue_tx;
		p->retval_io_dev = (void*) dev;
		if (io_char_dev_queue_level(dev->op_queue_tx) == 1) {
			if (dev->hw_tx_start)
				dev->hw_tx_start(dev);
			dev->flags |= NILE_KERNEL_IO_DEV_FLAG_DEV_HW_ACTIVE;
		}
	} else {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OP_QUEUE_FULL;
	}
	nile_irq_global_enable();

	done: return retval;
}

uint32_t system_call_io_block_dev_open(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk;
	nile_kernel_io_block_dev *dev;
	nile_kernel_tcb *current;
	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto done;
	}

	if (p->dev_id >= NILE_KERNEL_IO_BLOCK_DEV_COUNT) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
		goto done;
	}
	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	dev = &kk->io_block_dev[p->dev_id];
	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_READY)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY;
		goto done;
	}
	current = kk->scheduler.tcb_current_task;
	if (dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_SHARED) {
		if (dev->tcb_owner == current) {
			dev->tcb_owner = NULLPTR;
		}
		goto done;
	}

	if (dev->tcb_owner == NULLPTR || dev->tcb_owner == current) {
		dev->tcb_owner = current;
		goto done;
	}
	retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY;
	done: return retval;
}

uint32_t system_call_io_block_dev_close(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk;
	nile_kernel_io_block_dev *dev;
	nile_kernel_tcb *current;
	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto done;
	}

	if (p->dev_id >= NILE_KERNEL_IO_BLOCK_DEV_COUNT) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
		goto done;
	}
	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	dev = &kk->io_block_dev[p->dev_id];
	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_READY)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY;
		goto done;
	}
	if (dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_SHARED)
		goto done;
	current = kk->scheduler.tcb_current_task;
	if (dev->tcb_owner == NULLPTR || dev->tcb_owner == current) {
		dev->tcb_owner = NULLPTR;
		goto done;
	}
	retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY;
	done: return retval;
}

uint32_t system_call_io_block_dev_read(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk;
	nile_kernel_io_block_dev *dev;
	nile_kernel_tcb *current;
	uint32_t queue_push_retval;

	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto done;
	}

	if (p->dev_id >= NILE_KERNEL_IO_BLOCK_DEV_COUNT) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
		goto done;
	}

	p->op_flags = (p->op_flags & ~NILE_KERNEL_IO_DEV_OP_FLAG_SYSTEM_MASK) | NILE_KERNEL_IO_DEV_OP_FLAG_WRITE;

	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	dev = &kk->io_block_dev[p->dev_id];
	current = kk->scheduler.tcb_current_task;

	if (p->data_len == 0) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OPERATION_LENGTH_0;
		goto done;
	}

	if (!syscall_mem_range_in_curr_task_rw_mem(p->data_buffer, (void*) ((uint8_t*) p->data_buffer + p->data_len - 1))) {
		retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_READY)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_SHARED)) {
		if (dev->tcb_owner == NULLPTR) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_OPEN;
			goto done;
		}
		if (dev->tcb_owner != current) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY;
			goto done;
		}
	}

	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_RX_ENABLED)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_MODE_NOT_SUPPORTED;
		goto done;
	}

	/* Block-dev specific: verify operation block size */
	if (((p->block_dev_memaddr % dev->read_block_size) != 0) || ((p->data_len % dev->read_block_size) != 0)) {
		retval = NILE_SYSCALL_RETVAL_ERR_BAD_ALIGNMENT;
		goto done;
	}

	nile_irq_global_disable();
	queue_push_retval = io_block_dev_queue_push(dev->rqx, p, current);
	if (queue_push_retval) {
		nile_kernel_io_block_dev_op *slot = io_block_dev_queue_peek(dev->rqx);
		p->retval_io_op_ptr = (uint32_t*) &slot->op;
		p->retval_io_op_q_ptr = (io_op_ringbuffer_queue*) dev->rqx;
		p->retval_io_dev = (void*) dev;
		slot->op.flags &= ~NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_MASK;
		slot->op.flags |= NILE_KERNEL_IO_DEV_OP_FLAG_READ;
		if (io_block_dev_queue_level(dev->rqx) == 1) {
			if (dev->rqx_execute)
				dev->rqx_execute(dev);
		}
	} else {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OP_QUEUE_FULL;
	}
	nile_irq_global_enable();

	done: return retval;
}

uint32_t system_call_io_block_dev_write(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk;
	nile_kernel_io_block_dev *dev;
	nile_kernel_tcb *current;
	uint32_t queue_push_retval;

	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto done;
	}

	if (p->dev_id >= NILE_KERNEL_IO_BLOCK_DEV_COUNT) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
		goto done;
	}
	p->op_flags = (p->op_flags & ~NILE_KERNEL_IO_DEV_OP_FLAG_SYSTEM_MASK) | NILE_KERNEL_IO_DEV_OP_FLAG_WRITE;

	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	dev = &kk->io_block_dev[p->dev_id];
	current = kk->scheduler.tcb_current_task;

	if (p->data_len == 0) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OPERATION_LENGTH_0;
		goto done;
	}

	if (!syscall_mem_range_in_curr_task_rw_mem(p->data_buffer, (void*) ((uint8_t*) p->data_buffer + p->data_len - 1))) {
		retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_READY)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_SHARED)) {
		if (dev->tcb_owner == NULLPTR) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_OPEN;
			goto done;
		}
		if (dev->tcb_owner != current) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY;
			goto done;
		}
	}

	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_TX_ENABLED)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_MODE_NOT_SUPPORTED;
		goto done;
	}

	/* Block-dev specific: verify operation block size */
	if (((p->block_dev_memaddr % dev->write_block_size) != 0) || ((p->data_len % dev->write_block_size) != 0)) {
		retval = NILE_SYSCALL_RETVAL_ERR_BAD_ALIGNMENT;
		goto done;
	}

	nile_irq_global_disable();
	queue_push_retval = io_block_dev_queue_push(dev->rqx, p, current);
	if (queue_push_retval) {
		nile_kernel_io_block_dev_op *slot = io_block_dev_queue_peek(dev->rqx);
		p->retval_io_op_ptr = (uint32_t*) &slot->op;
		p->retval_io_op_q_ptr = (io_op_ringbuffer_queue*) dev->rqx;
		p->retval_io_dev = (void*) dev;
		slot->op.flags &= ~NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_MASK;
		slot->op.flags |= NILE_KERNEL_IO_DEV_OP_FLAG_WRITE;
		if (io_block_dev_queue_level(dev->rqx) == 1) {
			if (dev->rqx_execute)
				dev->rqx_execute(dev);
		}
	} else {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OP_QUEUE_FULL;
	}
	nile_irq_global_enable();

	done: return retval;
}

uint32_t system_call_io_block_dev_erase(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk;
	nile_kernel_io_block_dev *dev;
	nile_kernel_tcb *current;
	uint32_t queue_push_retval;

	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto done;
	}

	if (p->dev_id >= NILE_KERNEL_IO_BLOCK_DEV_COUNT) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
		goto done;
	}

	p->op_flags = (p->op_flags & ~NILE_KERNEL_IO_DEV_OP_FLAG_SYSTEM_MASK) | NILE_KERNEL_IO_DEV_OP_FLAG_WRITE;

	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	dev = &kk->io_block_dev[p->dev_id];
	current = kk->scheduler.tcb_current_task;

	if (p->data_len == 0) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OPERATION_LENGTH_0;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_READY)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY;
		goto done;
	}

	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_SHARED)) {
		if (dev->tcb_owner == NULLPTR) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_OPEN;
			goto done;
		}
		if (dev->tcb_owner != current) {
			retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY;
			goto done;
		}
	}

	if (!(dev->flags & NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_EX_ENABLED)) {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_MODE_NOT_SUPPORTED;
		goto done;
	}

	/* Block-dev specific: verify operation block size */
	if (((p->block_dev_memaddr % dev->erase_block_size) != 0) || ((p->data_len % dev->erase_block_size) != 0)) {
		retval = NILE_SYSCALL_RETVAL_ERR_BAD_ALIGNMENT;
		goto done;
	}

	nile_irq_global_disable();
	queue_push_retval = io_block_dev_queue_push(dev->rqx, p, current);
	if (queue_push_retval) {
		nile_kernel_io_block_dev_op *slot = io_block_dev_queue_peek(dev->rqx);
		p->retval_io_op_ptr = (uint32_t*) &slot->op;
		p->retval_io_op_q_ptr = (io_op_ringbuffer_queue*) dev->rqx;
		p->retval_io_dev = (void*) dev;
		slot->op.flags &= ~NILE_KERNEL_IO_DEV_OP_FLAG_OPERATION_MASK;
		slot->op.flags |= NILE_KERNEL_IO_DEV_OP_FLAG_ERASE;
		if (io_block_dev_queue_level(dev->rqx) == 1) {
			if (dev->rqx_execute)
				dev->rqx_execute(dev);
		}
	} else {
		retval = NILE_SYSCALL_RETVAL_ERR_DEVICE_OP_QUEUE_FULL;
	}
	nile_irq_global_enable();

	done: return retval;
}

uint32_t system_call_io_reserved0(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_io_reserved1(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_io_reserved2(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_io_reserved3(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_io_reserved4(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
