#include "nile/compiler.h"
#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/kernel/syscall_helper.h"
#include "nile/kernel/io_op_queue.h"
#include "nile/kernel/io_dev.h"
#include "nile/irq.h"

#include "nile/syscall_api.h"

uint32_t system_call_io_char_dev_ioctl(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;

	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		return NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
	}

	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	nile_kernel_io_char_dev *dev;

	if (p->dev_id >= NILE_KERNEL_IO_CHAR_DEV_COUNT) {
		return NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
	}

	dev = &kk->io_char_dev[p->dev_id];

	if (p->io_ctl_cmd == NILE_SYSCALL_IOCTL_NONE) {
		return NILE_SYSCALL_RETVAL_OK;
	}

	uint32_t *out_buf = p->ioctl_out_buf;
	uint32_t out_len_words = p->ioctl_out_buf_len / sizeof(uint32_t);
	;

	switch (p->io_ctl_cmd) {
	case NILE_SYSCALL_IOCTL_CHAR_DEV_CAPABILITY: {
		const uint32_t required_words = 1;
		if (out_buf == NULL) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (!syscall_mem_range_in_curr_task_rw_mem(out_buf, (void*) ((uint8_t*) out_buf + required_words * sizeof(uint32_t) - 1))) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (out_len_words < required_words) {
			retval = NILE_SYSCALL_RETVAL_ERR_DATA_BUFFER_TOO_SMALL;
			break;
		}

		const uint32_t caps_mask =
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_BLOCK_DEV |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_CHAR_DEV |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_TX_ENABLED |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_RX_ENABLED |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_EX_ENABLED |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_MEMMAPPED |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_SHARED;

		out_buf[0] = (dev->flags & caps_mask);
		retval = NILE_SYSCALL_RETVAL_OK;
		break;
	}

	case NILE_SYSCALL_IOCTL_CHAR_DEV_STATUS: {
		const uint32_t required_words = 1;
		if (out_buf == NULL) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (!syscall_mem_range_in_curr_task_rw_mem(out_buf, (void*) ((uint8_t*) out_buf + required_words * sizeof(uint32_t) - 1))) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (out_len_words < required_words) {
			retval = NILE_SYSCALL_RETVAL_ERR_DATA_BUFFER_TOO_SMALL;
			break;
		}
		const uint32_t status_mask =
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_MASK;

		out_buf[0] = (dev->flags & status_mask);
		retval = NILE_SYSCALL_RETVAL_OK;
		break;
	}

	case NILE_SYSCALL_IOCTL_CHAR_DEV_RESET_DEVICE: {
		if (dev->hw_reset != NULL) {
			dev->flags &= ~(NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE |
			NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE |
			NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
			NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR |
			NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_MASK);
			dev->flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY;
			dev->hw_reset(dev);
		}
		retval = NILE_SYSCALL_RETVAL_OK;
		break;
	}

	case NILE_SYSCALL_IOCTL_CHAR_DEV_GET_HW_ERRCODE: {
		const uint32_t required_words = 1;

		if (out_buf == NULL) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}

		if (!syscall_mem_range_in_curr_task_rw_mem(out_buf, (void*) ((uint8_t*) out_buf + required_words * sizeof(uint32_t) - 1))) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}

		if (out_len_words < required_words) {
			retval = NILE_SYSCALL_RETVAL_ERR_DATA_BUFFER_TOO_SMALL;
			break;
		}

		out_buf[0] = (dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_MASK) >> NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_POS;
		retval = NILE_SYSCALL_RETVAL_OK;
		break;
	}

	default:
		retval = NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
		break;
	}

	return retval;
}

uint32_t system_call_io_block_dev_ioctl(uint32_t *arg_list, uint32_t arg_cnt) {
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;

	if (arg_cnt < NILE_SYSCALL_PARAM_CNT_IO_OP) {
		return NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
	}

	nile_syscall_params_io_op *p = (nile_syscall_params_io_op*) arg_list;
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	nile_kernel_io_block_dev *dev;

	if (p->dev_id >= NILE_KERNEL_IO_BLOCK_DEV_COUNT) {
		return NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST;
	}

	dev = &kk->io_block_dev[p->dev_id];

	if (p->io_ctl_cmd == NILE_SYSCALL_IOCTL_NONE) {
		return NILE_SYSCALL_RETVAL_OK;
	}

	uint32_t *out_buf = p->ioctl_out_buf;
	uint32_t out_len_words = p->ioctl_out_buf_len / sizeof(uint32_t);

	switch (p->io_ctl_cmd) {
	case NILE_SYSCALL_IOCTL_BLOCK_DEV_CAPABILITY: {

		const uint32_t required_words = 5;

		if (out_buf == NULL) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (!syscall_mem_range_in_curr_task_rw_mem(out_buf, (void*) ((uint8_t*) out_buf + required_words * sizeof(uint32_t) - 1))) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (out_len_words < required_words) {
			retval = NILE_SYSCALL_RETVAL_ERR_DATA_BUFFER_TOO_SMALL;
			break;
		}

		const uint32_t caps_mask =
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_BLOCK_DEV |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_CHAR_DEV |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_TX_ENABLED |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_RX_ENABLED |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_EX_ENABLED |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_MEMMAPPED |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_SHARED;

		out_buf[0] = (dev->flags & caps_mask);
		out_buf[1] = dev->capacity;
		out_buf[2] = dev->read_block_size;
		out_buf[3] = dev->write_block_size;
		out_buf[4] = dev->erase_block_size;

		retval = NILE_SYSCALL_RETVAL_OK;
		break;
	}

	case NILE_SYSCALL_IOCTL_BLOCK_DEV_STATUS: {
		const uint32_t required_words = 1;

		if (out_buf == NULL) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (!syscall_mem_range_in_curr_task_rw_mem(out_buf, (void*) ((uint8_t*) out_buf + required_words * sizeof(uint32_t) - 1))) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (out_len_words < required_words) {
			retval = NILE_SYSCALL_RETVAL_ERR_DATA_BUFFER_TOO_SMALL;
			break;
		}

		const uint32_t status_mask =
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_MASK;

		out_buf[0] = (dev->flags & status_mask);
		retval = NILE_SYSCALL_RETVAL_OK;
		break;

		case NILE_SYSCALL_IOCTL_BLOCK_DEV_RESET_DEVICE:
		{
			if (dev->hw_reset != NULL) {
				dev->flags &= ~(NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE |
				NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE |
				NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
				NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR |
				NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_MASK);
				dev->flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY;
			}
			dev->hw_reset(dev);
		}
		retval = NILE_SYSCALL_RETVAL_OK;
		break;
	}

	case NILE_SYSCALL_IOCTL_BLOCK_DEV_GET_HW_ERRCODE: {
		const uint32_t required_words = 1;

		if (out_buf == NULL) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (!syscall_mem_range_in_curr_task_rw_mem(out_buf, (void*) ((uint8_t*) out_buf + required_words * sizeof(uint32_t) - 1))) {
			retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
			break;
		}
		if (out_len_words < required_words) {
			retval = NILE_SYSCALL_RETVAL_ERR_DATA_BUFFER_TOO_SMALL;
			break;
		}

		out_buf[0] = (dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_MASK) >> NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_POS;

		retval = NILE_SYSCALL_RETVAL_OK;
		break;
	}

	default:
		retval = NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
		break;
	}

	return retval;
}
