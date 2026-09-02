#include "nile/stdtypes.h"
#include "nile/kernel/io_dev.h"
#include "nile/kernel/io_op_queue.h"

void io_char_dev_event_hw_rx_data_rdy_handler(nile_kernel_io_char_dev *dev) {
	/* Hardware register alias */
	union {
		volatile void *base;
		volatile uint8_t *u8;
		volatile uint16_t *u16;
		volatile uint32_t *u32;
		volatile uint64_t *u64;
	} hw;

	/* Destination buffer alias */
	union {
		uint8_t *u8;
		uint16_t *u16;
		uint32_t *u32;
		uint64_t *u64;
	} dst;

	/* Get granularity config */
	uint16_t gran_cfg;
	if ((dev->rx_gran_config & NILE_KERNEL_IO_CHAR_DEV_GRN_DYNAMIC_MASK) != 0U) {
		gran_cfg = dev->hw_rx_dynamic_gran_config_get(dev);
	} else {
		gran_cfg = dev->rx_gran_config;
	}

	/* Decode fields */
	uint16_t total_bytes = (gran_cfg & NILE_KERNEL_IO_CHAR_DEV_GRN_TOTAL_MASK) >> NILE_KERNEL_IO_CHAR_DEV_GRN_TOTAL_POS;

	uint8_t mem_width_raw = (gran_cfg & NILE_KERNEL_IO_CHAR_DEV_GRN_MEM_WIDTH_MASK) >> NILE_KERNEL_IO_CHAR_DEV_GRN_MEM_WIDTH_POS;

	uint8_t dev_width_raw = (gran_cfg & NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_WIDTH_MASK) >> NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_WIDTH_POS;

	uint8_t mem_width_bytes = (uint8_t) (1U << mem_width_raw);
	uint8_t dev_width_bytes = (uint8_t) (1U << dev_width_raw);

	uint8_t access_bytes;
	uint8_t access_bytes_raw;

	if (mem_width_bytes < dev_width_bytes) {
		access_bytes = mem_width_bytes;
		access_bytes_raw = mem_width_raw;
	} else {
		access_bytes = dev_width_bytes;
		access_bytes_raw = dev_width_raw;
	}

	uint8_t dev_autoinc = 0U;
	if ((gran_cfg & NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_AUTOINC_MASK) != 0U) {
		dev_autoinc = 1U;
	}

	/* Current operation */
	nile_kernel_io_char_dev_op *op = io_char_dev_queue_peek(dev->op_queue_rx);
	if (op == NULL) {
		//not supposed to happen
		return;
	}

	uint32_t op_remaining = op->op.data_len - op->op.cursor;
	if (op_remaining < total_bytes) {
		total_bytes = (uint16_t) op_remaining;
	}

	if (total_bytes == 0U) {
		return;
	}

	uint32_t iteration_cnt = total_bytes >> access_bytes_raw;
	uint32_t leftover = total_bytes & (access_bytes - 1U);

	/* Init aliases */
	hw.base = dev->hw_rx_data_reg;
	dst.u8 = op->op.data_buffer + op->op.cursor;

	/* Width selector */
	uint8_t selector = (uint8_t) ((mem_width_raw << 2) | dev_width_raw);

	/* Main transfer loop */
	for (uint32_t i = 0U; i < iteration_cnt; i++) {

		uint32_t idx = 0U;
		if (dev_autoinc != 0U) {
			idx = i;
		}

		switch (selector) {

		/* MEM=8, DEV=8/16/32/64 */
		case 0U:
			*dst.u8 = hw.u8[idx];
			break;
		case 1U:
			*dst.u8 = (uint8_t) hw.u16[idx];
			break;
		case 2U:
			*dst.u8 = (uint8_t) hw.u32[idx];
			break;
		case 3U:
			*dst.u8 = (uint8_t) hw.u64[idx];
			break;

			/* MEM=16, DEV=16/32/64 */
		case 5U:
			*dst.u16 = hw.u16[idx];
			break;
		case 6U:
			*dst.u16 = (uint16_t) hw.u32[idx];
			break;
		case 7U:
			*dst.u16 = (uint16_t) hw.u64[idx];
			break;

			/* MEM=32, DEV=32/64 */
		case 10U:
			*dst.u32 = hw.u32[idx];
			break;
		case 11U:
			*dst.u32 = (uint32_t) hw.u64[idx];
			break;

			/* MEM=64, DEV=64 */
		case 15U:
			*dst.u64 = hw.u64[idx];
			break;

		default:
			return; /* unsupported packed combination */
		}

		/* Single increment per iteration */
		dst.u8 += access_bytes;
	}

	/* Partial leftover */
	if (leftover > 0U) {

		uint32_t idx = 0U;
		if (dev_autoinc != 0U) {
			idx = iteration_cnt;
		}

		uint64_t word = 0U;
		if (dev_width_raw == 0U)
			word = (uint64_t) hw.u8[idx];
		else if (dev_width_raw == 1U)
			word = (uint64_t) hw.u16[idx];
		else if (dev_width_raw == 2U)
			word = (uint64_t) hw.u32[idx];
		else
			word = hw.u64[idx];

		uint8_t *src8 = (uint8_t*) &word;

		for (uint32_t b = 0U; b < leftover; b++) {
			*dst.u8 = src8[b];
			dst.u8 += 1U;
		}
	}

	/* Mark active */
	if (op->op.cursor == 0U) {
		op->op.flags = (op->op.flags & ~NILE_KERNEL_IO_DEV_OP_FLAG_PENDING) |
		NILE_KERNEL_IO_DEV_OP_FLAG_ACTIVE;

		dev->flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE;
	}

	op->op.cursor += total_bytes;

	/* Finish op */
	if (op->op.cursor == op->op.data_len) {

		*op->op.io_hw_op_finished_code_ptr = (op->op.cursor << IO_HW_OP_FINISHED_CODE_OP_LENGTH_POS) & IO_HW_OP_FINISHED_CODE_OP_LENGTH_MASK;

		io_char_dev_queue_pop(dev->op_queue_rx);

		nile_kernel_io_char_dev_op *next_op = io_char_dev_queue_peek(dev->op_queue_rx);
		while (next_op && (next_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED)) {
			io_char_dev_queue_pop(dev->op_queue_rx);
			next_op = io_char_dev_queue_peek(dev->op_queue_rx);
		}

		if (next_op == NULL) {

			if (dev->hw_rx_stop != NULL) {
				dev->hw_rx_stop(dev);
			}

			dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE;

			if ((dev->flags & (NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
			NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE)) == 0U) {

				dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE;
			}
		}
	}
}

void io_char_dev_event_hw_tx_data_write_rdy_handler(nile_kernel_io_char_dev *dev) {
	/* Hardware register alias */
	union {
		volatile void *base;
		volatile uint8_t *u8;
		volatile uint16_t *u16;
		volatile uint32_t *u32;
		volatile uint64_t *u64;
	} hw;

	/* Source buffer alias */
	union {
		uint8_t *u8;
		uint16_t *u16;
		uint32_t *u32;
		uint64_t *u64;
	} src;

	/* Get granularity config (dynamic or static) */
	uint16_t gran_cfg;
	if ((dev->tx_gran_config & NILE_KERNEL_IO_CHAR_DEV_GRN_DYNAMIC_MASK) != 0U) {
		gran_cfg = dev->hw_tx_dynamic_gran_config_get(dev);
	} else {
		gran_cfg = dev->tx_gran_config;
	}

	/* Decode granularity fields */
	uint16_t total_bytes = (gran_cfg & NILE_KERNEL_IO_CHAR_DEV_GRN_TOTAL_MASK) >> NILE_KERNEL_IO_CHAR_DEV_GRN_TOTAL_POS;

	uint8_t mem_width_raw = (gran_cfg & NILE_KERNEL_IO_CHAR_DEV_GRN_MEM_WIDTH_MASK) >> NILE_KERNEL_IO_CHAR_DEV_GRN_MEM_WIDTH_POS;

	uint8_t dev_width_raw = (gran_cfg & NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_WIDTH_MASK) >> NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_WIDTH_POS;

	uint8_t mem_width_bytes = (uint8_t) (1U << mem_width_raw);
	uint8_t dev_width_bytes = (uint8_t) (1U << dev_width_raw);

	uint8_t access_bytes;
	uint8_t access_bytes_raw;

	if (mem_width_bytes < dev_width_bytes) {
		access_bytes = mem_width_bytes;
		access_bytes_raw = mem_width_raw;
	} else {
		access_bytes = dev_width_bytes;
		access_bytes_raw = dev_width_raw;
	}

	uint8_t dev_autoinc = 0U;
	if ((gran_cfg & NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_AUTOINC_MASK) != 0U) {
		dev_autoinc = 1U;
	}

	/* Current operation */
	nile_kernel_io_char_dev_op *op = io_char_dev_queue_peek(dev->op_queue_tx);
	if (op == NULL) {
		return;
	}

	uint32_t remaining = op->op.data_len - op->op.cursor;
	if (remaining < total_bytes) {
		total_bytes = (uint16_t) remaining;
	}

	if (total_bytes == 0U) {
		return;
	}

	uint32_t iteration_cnt = total_bytes >> access_bytes_raw;
	uint32_t leftover = total_bytes & (access_bytes - 1U);

	/* Init aliases */
	hw.base = dev->hw_tx_data_reg;
	src.u8 = op->op.data_buffer + op->op.cursor;

	/* Width selector: MEM(2 bits) + DEV(2 bits) */
	uint8_t selector = (uint8_t) ((mem_width_raw << 2) | dev_width_raw);

	/* Main transfer loop */
	for (uint32_t i = 0U; i < iteration_cnt; i++) {

		uint32_t idx = 0U;
		if (dev_autoinc != 0U) {
			idx = i;
		}

		switch (selector) {

		/* MEM=8, DEV=8/16/32/64 */
		case 0U: /* 00 00 */
			hw.u8[idx] = *src.u8;
			break;

		case 1U: /* 00 01 */
			hw.u16[idx] = (uint16_t) (*src.u8);
			break;

		case 2U: /* 00 02 */
			hw.u32[idx] = (uint32_t) (*src.u8);
			break;

		case 3U: /* 00 03 */
			hw.u64[idx] = (uint64_t) (*src.u8);
			break;

			/* MEM=16, DEV=16/32/64 */
		case 5U: /* 01 01 */
			hw.u16[idx] = *src.u16;
			break;

		case 6U: /* 01 02 */
			hw.u32[idx] = (uint32_t) (*src.u16);
			break;

		case 7U: /* 01 03 */
			hw.u64[idx] = (uint64_t) (*src.u16);
			break;

			/* MEM=32, DEV=32/64 */
		case 10U: /* 10 02 */
			hw.u32[idx] = *src.u32;
			break;

		case 11U: /* 10 03 */
			hw.u64[idx] = (uint64_t) (*src.u32);
			break;

			/* MEM=64, DEV=64 */
		case 15U: /* 11 03 */
			hw.u64[idx] = *src.u64;
			break;

		default:
			return; /* unsupported packed combination */
		}

		/* Advance source by amount of data written */
		src.u8 += access_bytes;
	}

	/* Partial leftover: write tail bytes into one device word */
	if (leftover > 0U) {

		uint32_t idx = 0U;
		if (dev_autoinc != 0U) {
			idx = iteration_cnt;
		}

		uint64_t word = 0U;
		uint8_t *dst8 = (uint8_t*) &word;

		for (uint32_t b = 0U; b < leftover; b++) {
			dst8[b] = *src.u8;
			src.u8 += 1U;
		}

		if (dev_width_raw == 0U) {
			hw.u8[idx] = (uint8_t) word;
		} else if (dev_width_raw == 1U) {
			hw.u16[idx] = (uint16_t) word;
		} else if (dev_width_raw == 2U) {
			hw.u32[idx] = (uint32_t) word;
		} else {
			hw.u64[idx] = word;
		}
	}

	/* Mark operation active on first data */
	if (op->op.cursor == 0U) {
		op->op.flags = (op->op.flags & ~NILE_KERNEL_IO_DEV_OP_FLAG_PENDING) |
		NILE_KERNEL_IO_DEV_OP_FLAG_ACTIVE;

		dev->flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE |
		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE;
	}

	op->op.cursor += total_bytes;

	/* Operation finished */
	if (op->op.cursor == op->op.data_len) {

		*op->op.io_hw_op_finished_code_ptr = (op->op.cursor << IO_HW_OP_FINISHED_CODE_OP_LENGTH_POS) & IO_HW_OP_FINISHED_CODE_OP_LENGTH_MASK;

		io_char_dev_queue_pop(dev->op_queue_tx);

		nile_kernel_io_char_dev_op *next_op = io_char_dev_queue_peek(dev->op_queue_tx);
		while (next_op != NULL && (next_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED) != 0U) {

			io_char_dev_queue_pop(dev->op_queue_tx);
			next_op = io_char_dev_queue_peek(dev->op_queue_tx);
		}

		if (next_op == NULL) {

			if (dev->hw_tx_stop != NULL) {
				dev->hw_tx_stop(dev);
			}

			dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE;

			if ((dev->flags & (NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
			NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE)) == 0U) {

				dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE;
			}
		}
	}
}

void io_char_dev_event_hw_rx_error_handler(nile_kernel_io_char_dev *dev, uint8_t hw_specific_errcode) {

	//Device not ready, so we don't accept further operations
	dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY;

	//Set device error code
	dev->flags = (dev->flags & ~(NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR | NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_MASK));
	dev->flags = (dev->flags | (NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR | (hw_specific_errcode << NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_POS)));

	//drain the rx operation queue, set error codes for all operations in the queue
	nile_kernel_io_char_dev_op *it_op = io_char_dev_queue_peek(dev->op_queue_rx);
	*it_op->op.io_hw_op_finished_code_ptr = hw_specific_errcode << IO_HW_OP_FINISHED_CODE_ERRCODE_POS;
	io_char_dev_queue_pop(dev->op_queue_rx);
	it_op = io_char_dev_queue_peek(dev->op_queue_rx);
	while (it_op != NULL) {
		it_op->op.flags = (it_op->op.flags | NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED) & ~NILE_KERNEL_IO_DEV_OP_FLAG_PENDING;
		*it_op->op.io_hw_op_finished_code_ptr = IO_HW_OP_FINISHED_CODE_ERRCODE_ABORTED << IO_HW_OP_FINISHED_CODE_ERRCODE_POS;
		io_char_dev_queue_pop(dev->op_queue_rx);
		it_op = io_char_dev_queue_peek(dev->op_queue_rx);
	}

	//Stop reception
	if (dev->hw_rx_stop != NULL) {
		dev->hw_rx_stop(dev);
	}

	dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE;
	if ((dev->flags & (NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
	NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE)) == 0U) {
		dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE;
	}
}

