#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/kernel/io_op_queue.h"

void pipe_hw_init(struct nile_kernel_io_char_dev *dev) {

	dev->flags &= ~(NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE |
	NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE |
	NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
	NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR |
	NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_MASK);

	dev->flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY;

	//drain the queues
	while (io_char_dev_queue_pop(dev->op_queue_tx));
	while (io_char_dev_queue_pop(dev->op_queue_rx));
}

void pipe_hw_reset(struct nile_kernel_io_char_dev *dev) {
	pipe_hw_init(dev);
}

void pipe_hw_tx_start(struct nile_kernel_io_char_dev *dev) {
	nile_kernel_io_char_dev_op *rx_op = io_char_dev_queue_peek(dev->op_queue_rx);
	nile_kernel_io_char_dev_op *tx_op = io_char_dev_queue_peek(dev->op_queue_tx);
	while (rx_op && (rx_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED)) {
		io_char_dev_queue_pop(dev->op_queue_rx);
		rx_op = io_char_dev_queue_peek(dev->op_queue_rx);
	}
	while (tx_op && (tx_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED)) {
		io_char_dev_queue_pop(dev->op_queue_tx);
		tx_op = io_char_dev_queue_peek(dev->op_queue_tx);
	}
	while (rx_op && tx_op) {
		uint16_t cpy_len = tx_op->op.data_len - tx_op->op.cursor;
		if (rx_op->op.data_len < cpy_len) {
			cpy_len = rx_op->op.data_len - rx_op->op.cursor;
		}
		//actual copy
		const uint8_t cpy_len_remainder = cpy_len % sizeof(uint32_t);
		const uint16_t cpy_len_wrd = cpy_len / sizeof(uint32_t);
		for (uint16_t cpywordi = 0; cpywordi < cpy_len_wrd; cpywordi++) {
			*(uint32_t*) &rx_op->op.data_buffer[rx_op->op.cursor] = *(uint32_t*) &tx_op->op.data_buffer[tx_op->op.cursor];
			rx_op->op.cursor += sizeof(uint32_t);
			tx_op->op.cursor += sizeof(uint32_t);
		}
		for (uint8_t cpybi = 0; cpybi < cpy_len_remainder; cpybi++) {
			rx_op->op.data_buffer[rx_op->op.cursor] = tx_op->op.data_buffer[tx_op->op.cursor];
			rx_op->op.cursor++;
			tx_op->op.cursor++;
		}
		//whatever operation ended, pop it, drain the aborted operations
		if (rx_op->op.cursor == rx_op->op.data_len) {
			*(rx_op->op.io_hw_op_finished_code_ptr) = rx_op->op.data_len;
			io_char_dev_queue_pop(dev->op_queue_rx);
			rx_op = io_char_dev_queue_peek(dev->op_queue_rx);
			while (rx_op && rx_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED) {
				io_char_dev_queue_pop(dev->op_queue_rx);
				rx_op = io_char_dev_queue_peek(dev->op_queue_rx);
			}
		}
		if (tx_op->op.cursor == tx_op->op.data_len) {
			*(tx_op->op.io_hw_op_finished_code_ptr) = tx_op->op.data_len;
			io_char_dev_queue_pop(dev->op_queue_tx);
			tx_op = io_char_dev_queue_peek(dev->op_queue_tx);
			while (tx_op && tx_op->op.flags & NILE_KERNEL_IO_DEV_OP_FLAG_ABORTED) {
				io_char_dev_queue_pop(dev->op_queue_tx);
				tx_op = io_char_dev_queue_peek(dev->op_queue_tx);
			}
		}
	}

}

void pipe_hw_rx_start(struct nile_kernel_io_char_dev *dev) {
	pipe_hw_tx_start(dev);
}

void pipe_hw_tx_stop(struct nile_kernel_io_char_dev *dev) {
	(void) dev;
}
void pipe_hw_rx_stop(struct nile_kernel_io_char_dev *dev) {
	(void) dev;
}
