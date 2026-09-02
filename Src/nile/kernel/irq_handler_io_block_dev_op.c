#include "nile/stdtypes.h"
#include "nile/kernel/io_dev.h"
#include "nile/kernel/io_op_queue.h"

void io_block_dev_event_hw_rx_data_rdy_handler(nile_kernel_io_block_dev *dev) {
	(void) dev;
//	/* Current operation (block device op wrapper) */
//	nile_kernel_io_block_dev_op *bop = io_block_dev_queue_peek(dev->rqx);
//	if (bop == NULL) {
//		/* Nothing queued — spurious interrupt */
//		return;
//	}
//
//	nile_kernel_io_dev_op *op = &bop->op;
//
//	/* Remaining bytes in current op */
//	uint32_t op_remaining = op->data_len - op->cursor;
//	if (op_remaining == 0U) {
//		return;
//	}
//
//	/* Decode granularity bitfield (fixed for block devices) */
//	uint16_t gran_cfg = dev->read_granularity;
//
//	uint32_t total_bytes = (uint32_t) ((gran_cfg & NILE_KERNEL_IO_BLOCK_DEV_GRN_TOTAL_MASK) >> NILE_KERNEL_IO_BLOCK_DEV_GRN_TOTAL_POS);
//
//	/* Enforce invariant: syscall layer guarantees whole-number granularity.
//	 * If violated, bail out to avoid corruption.
//	 */
//	if (total_bytes == 0U) {
//		return;
//	}
//
//	uint8_t mem_width_raw = (uint8_t) ((gran_cfg & NILE_KERNEL_IO_BLOCK_DEV_GRN_MEM_WIDTH_MASK) >> NILE_KERNEL_IO_BLOCK_DEV_GRN_MEM_WIDTH_POS);
//	uint8_t dev_width_raw = (uint8_t) ((gran_cfg & NILE_KERNEL_IO_BLOCK_DEV_GRN_DEV_WIDTH_MASK) >> NILE_KERNEL_IO_BLOCK_DEV_GRN_DEV_WIDTH_POS);
//
//	uint32_t mem_width_bytes = (uint32_t) (1U << mem_width_raw);
//	uint32_t dev_width_bytes = (uint32_t) (1U << dev_width_raw);
//
//	/* access width = min(mem_width, dev_width) */
//	uint32_t access_bytes;
//	uint8_t access_bytes_raw;
//	if (mem_width_bytes < dev_width_bytes) {
//		access_bytes = mem_width_bytes;
//		access_bytes_raw = mem_width_raw;
//	} else {
//		access_bytes = dev_width_bytes;
//		access_bytes_raw = dev_width_raw;
//	}
//
//	/* autoinc flag (device-side indexing) */
//	uint8_t dev_autoinc = ((gran_cfg & NILE_KERNEL_IO_BLOCK_DEV_GRN_DEV_AUTOINC_MASK) != 0U) ? 1U : 0U;
//
//	/* Transfer exactly one granularity unit per interrupt (controller signals per granularity) */
//	uint32_t transfer_bytes = total_bytes;
//
//	/* iteration count (no leftover by design) */
//	uint32_t iteration_cnt = transfer_bytes >> access_bytes_raw;
//
//	/* Prepare hardware source pointer:
//	 * - If device is memory-mapped, compute base = rx_memmap_addr + block_dev_mem_off + cursor
//	 * - Otherwise use hw_rx_data_reg as FIFO/register base
//	 *
//	 * Typed pointers are set so indexing (idx) is element-based (not byte-based).
//	 */
//	union {
//		volatile void *base;
//		volatile uint8_t *u8;
//		volatile uint16_t *u16;
//		volatile uint32_t *u32;
//		volatile uint64_t *u64;
//	} hw;
//
//	if ((dev->flags & NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_MEMMAPPED) != 0U && dev->rx_memmap_addr != NULL) {
//		/* memory mapped: start at block offset + current cursor */
//		uint8_t *base_byte = (uint8_t*) dev->rx_memmap_addr + bop->block_dev_mem_off + op->cursor;
//		hw.base = (volatile void*) base_byte;
//	} else {
//		/* register/FIFO mapped: base is hw_rx_data_reg */
//		hw.base = dev->hw_rx_data_reg;
//	}
//
//	/* Destination buffer pointer (start at cursor) */
//	union {
//		uint8_t *u8;
//		uint16_t *u16;
//		uint32_t *u32;
//		uint64_t *u64;
//	} dst;
//	dst.u8 = op->data_buffer + op->cursor;
//
//	/* Build selector like char handler: (mem_raw << 2) | dev_raw */
//	uint8_t selector = (uint8_t) ((mem_width_raw << 2) | dev_width_raw);
//
//	/* Main transfer loop: read iteration_cnt words of access_bytes each */
//	for (uint32_t i = 0U; i < iteration_cnt; i++) {
//		uint32_t idx = dev_autoinc ? i : 0U;
//
//		switch (selector) {
//		/* MEM=8, DEV=8/16/32/64 */
//		case 0U:
//			*dst.u8 = hw.u8[idx];
//			break;
//		case 1U:
//			*dst.u8 = (uint8_t) hw.u16[idx];
//			break;
//		case 2U:
//			*dst.u8 = (uint8_t) hw.u32[idx];
//			break;
//		case 3U:
//			*dst.u8 = (uint8_t) hw.u64[idx];
//			break;
//
//			/* MEM=16, DEV=16/32/64 */
//		case 5U:
//			*dst.u16 = hw.u16[idx];
//			break;
//		case 6U:
//			*dst.u16 = (uint16_t) hw.u32[idx];
//			break;
//		case 7U:
//			*dst.u16 = (uint16_t) hw.u64[idx];
//			break;
//
//			/* MEM=32, DEV=32/64 */
//		case 10U:
//			*dst.u32 = hw.u32[idx];
//			break;
//		case 11U:
//			*dst.u32 = (uint32_t) hw.u64[idx];
//			break;
//
//			/* MEM=64, DEV=64 */
//		case 15U:
//			*dst.u64 = hw.u64[idx];
//			break;
//
//		default:
//			/* Unsupported combination — bail out to avoid corruption */
//			return;
//		}
//
//		/* advance destination by access width */
//		dst.u8 += access_bytes;
//	}
//
//	/* Mark active if this was the first transfer for the op */
//	if (op->cursor == 0U) {
//		op->flags = (op->flags & ~NILE_KERNEL_IO_DEV_OP_FLAG_PENDING) |
//		NILE_KERNEL_IO_DEV_OP_FLAG_ACTIVE;
//
//		dev->flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE |
//		NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE;
//	}
//
//	/* Advance cursor by exactly the granularity */
//	op->cursor += transfer_bytes;
//
//	/* Finish op if complete */
//	if (op->cursor == op->data_len) {
//		/* write finished code (length) */
//		*op->io_hw_op_finished_code_ptr = (op->cursor << IO_HW_OP_FINISHED_CODE_OP_LENGTH_POS) & IO_HW_OP_FINISHED_CODE_OP_LENGTH_MASK;
//
//		io_block_dev_queue_pop(dev->rqx);
//
//		/* drop aborted ops */
//		nile_kernel_io_block_dev_op *next_bop = io_block_dev_queue_peek(dev->rqx);
//		while (next_bop && (next_bop->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED)) {
//			io_block_dev_queue_pop(dev->rqx);
//			next_bop = io_block_dev_queue_peek(dev->rqx);
//		}
//
//		if (next_bop == NULL) {
//			/* stop hardware if no more ops */
//			if (dev->hw_stop != NULL) {
//				dev->hw_stop(dev);
//			}
//			dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE;
//
//			if ((dev->flags & (NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
//			NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE)) == 0U) {
//
//				dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE;
//			}
//		} else {
//			//if next op exists, and it's not read, remove read active flag
//			//(because of the single op queue)
//			if (next_bop->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_WRITE) {
//				dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE;
//
//				if ((dev->flags & (NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
//				NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE)) == 0U) {
//
//					dev->flags &= ~NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE;
//				}
//			}
//		}
//	}
}

void io_block_dev_event_hw_tx_data_write_rdy_handler(nile_kernel_io_char_dev *dev) {
	(void) dev;

}

void io_block_dev_event_hw_ex_rdy_handler(nile_kernel_io_char_dev *dev) {
	(void) dev;

}

void io_block_dev_event_hw_error_handler(nile_kernel_io_char_dev *dev, uint8_t hw_specific_errcode) {
	(void) dev;
	(void) hw_specific_errcode;

}

