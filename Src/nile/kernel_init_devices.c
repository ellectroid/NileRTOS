#include "nile/kernel.h"
#include "nile/kernel/io_op_queue.h"

#include "app/io_dev_id.h"
#include "nile/mcu/stm32f746/io_dev/usart.h"
#include "nile/mcu/stm32f746/io_dev/quadspi.h"
#include "nile/kernel/io_dev/pipe.h"

static uint32_t init_device_usart1(nile_kernel *kk);
static uint32_t init_device_quadspi(nile_kernel *kk);
static uint32_t init_device_pipe0(nile_kernel *kk);

uint32_t nile_kernel_init_devices() {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	uint32_t return_code;

	return_code = init_device_usart1(kk);
	if (return_code)
		return return_code;

	return_code = init_device_quadspi(kk);
	if (return_code)
		return return_code;

	return_code = init_device_pipe0(kk);
	if (return_code)
		return return_code;

	return 0;
}

static uint32_t init_device_usart1(nile_kernel *kk) {
	kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_tx = kalloc(sizeof(nile_kernel_io_char_dev_op_queue), 0);
	kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_rx = kalloc(sizeof(nile_kernel_io_char_dev_op_queue), 0);
	if (!kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_tx || !kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_rx) {
		return 0x01;
	}
	for (uint8_t *i = (uint8_t*) kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_tx; i < (uint8_t*) (kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_tx + 1); i++) {
		*i = 0x00;
	}
	for (uint8_t *i = (uint8_t*) kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_rx; i < (uint8_t*) (kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_rx + 1); i++) {
		*i = 0x00;
	}
	kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_tx->queue.slot_capacity = NILE_KERNEL_IO_CHAR_DEV_OP_QUEUE_CAPACITY;
	kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_tx->queue.flags |= IO_OP_RING_BUFFER_FLAGS_IS_CHAR_DEV | IO_OP_RING_BUFFER_FLAGS_IS_TX;

	kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_rx->queue.slot_capacity = NILE_KERNEL_IO_CHAR_DEV_OP_QUEUE_CAPACITY;
	kk->io_char_dev[CHAR_DEV_ID_USART1].op_queue_rx->queue.flags |= IO_OP_RING_BUFFER_FLAGS_IS_CHAR_DEV | IO_OP_RING_BUFFER_FLAGS_IS_RX;

	kk->io_char_dev[CHAR_DEV_ID_USART1].hw = USART1;
	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_init = usart_hw_init;
	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_reset = usart_hw_reset;
	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_tx_start = usart_hw_tx_start;
	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_rx_start = usart_hw_rx_start;
	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_tx_stop = usart_hw_tx_stop;
	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_rx_stop = usart_hw_rx_stop;
	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_tx_dynamic_gran_config_get = usart_hw_tx_dynamic_gran_config_get;
	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_rx_dynamic_gran_config_get = usart_hw_rx_dynamic_gran_config_get;

	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_tx_data_reg = &USART1->TDR;
	kk->io_char_dev[CHAR_DEV_ID_USART1].hw_rx_data_reg = &USART1->RDR;

	kk->io_char_dev[CHAR_DEV_ID_USART1].tcb_owner = NULL;

	kk->io_char_dev[CHAR_DEV_ID_USART1].additional_data = NULL;
	kk->io_char_dev[CHAR_DEV_ID_USART1].additional_data_bytelen = 0;

	if (kk->io_char_dev[CHAR_DEV_ID_USART1].hw_init)
		kk->io_char_dev[CHAR_DEV_ID_USART1].hw_init(&kk->io_char_dev[CHAR_DEV_ID_USART1]);
	kk->io_char_dev[CHAR_DEV_ID_USART1].flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY;
	kk->io_char_dev[CHAR_DEV_ID_USART1].flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_CHAR_DEV;
	kk->io_char_dev[CHAR_DEV_ID_USART1].flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_TX_ENABLED | NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_RX_ENABLED;

	kk->io_char_dev[CHAR_DEV_ID_USART1].tx_gran_config = 1U |
	NILE_KERNEL_IO_CHAR_DEV_GRN_MEM_WIDTH_8 |
	NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_WIDTH_32;

	kk->io_char_dev[CHAR_DEV_ID_USART1].rx_gran_config = 1U |
	NILE_KERNEL_IO_CHAR_DEV_GRN_MEM_WIDTH_8 |
	NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_WIDTH_32;
	return 0;
}

static uint32_t init_device_quadspi(nile_kernel *kk) {
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].rqx = kalloc(sizeof(nile_kernel_io_block_dev_op_queue), 0);
	if (!kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].rqx) {
		return 0x01;
	}
	for (uint8_t *i = (uint8_t*) kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].rqx; i < (uint8_t*) (kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].rqx + 1); i++) {
		*i = 0x00;
	}

	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].rqx->queue.slot_capacity = NILE_KERNEL_IO_BLOCK_DEV_OP_QUEUE_CAPACITY;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].rqx->queue.flags |= IO_OP_RING_BUFFER_FLAGS_IS_BLOCK_DEV | IO_OP_RING_BUFFER_FLAGS_IS_TX
			| IO_OP_RING_BUFFER_FLAGS_IS_EX;

	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw = QUADSPI;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw_init = quadspi_hw_init;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw_reset = quadspi_hw_init;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].rqx_execute = quadspi_hw_rqx_start;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw_stop = quadspi_hw_stop;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw_memmapped_enable = quadspi_memory_mapped_mode_enable;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw_memmapped_disable = quadspi_memory_mapped_mode_disable;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].read_block_size = 0;

	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].rx_memmap_addr = (void*) NILE_QUADSPI_MEMMAPPED_ADDR;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw_tx_data_reg = &QUADSPI->DR;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw_rx_data_reg = &QUADSPI->DR;

	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].tcb_owner = NULL;

	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].additional_data = NULL;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].additional_data_bytelen = 0;

	if (kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw_init)
		kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].hw_init(&kk->io_block_dev[BLOCK_DEV_ID_QUADSPI]);
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].flags |= NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_MEMMAPPED;

	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].flags |= NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_READY;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].flags |= NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_BLOCK_DEV;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].flags |= NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_TX_ENABLED | NILE_KERNEL_IO_BLOCK_DEV_FLAG_DEV_EX_ENABLED;

	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].write_granularity = 16U |
	NILE_KERNEL_IO_BLOCK_DEV_GRN_MEM_WIDTH_32 |
	NILE_KERNEL_IO_BLOCK_DEV_GRN_DEV_WIDTH_32;

	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].read_granularity = 0;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].read_block_size = 0;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].write_block_size = 256;
	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].erase_block_size = 4096;

	kk->io_block_dev[BLOCK_DEV_ID_QUADSPI].capacity = 16 * 1024 * 1024;

	return 0;
}

static uint32_t init_device_pipe0(nile_kernel *kk) {
	CHAR_DEV_ID_PIPE0;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_tx = kalloc(sizeof(nile_kernel_io_char_dev_op_queue), 0);
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_rx = kalloc(sizeof(nile_kernel_io_char_dev_op_queue), 0);
	if (!kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_tx || !kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_rx) {
		return 0x01;
	}
	for (uint8_t *i = (uint8_t*) kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_tx; i < (uint8_t*) (kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_tx + 1); i++) {
		*i = 0x00;
	}
	for (uint8_t *i = (uint8_t*) kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_rx; i < (uint8_t*) (kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_rx + 1); i++) {
		*i = 0x00;
	}
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_tx->queue.slot_capacity = NILE_KERNEL_IO_CHAR_DEV_OP_QUEUE_CAPACITY;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_tx->queue.flags |= IO_OP_RING_BUFFER_FLAGS_IS_CHAR_DEV | IO_OP_RING_BUFFER_FLAGS_IS_TX;

	kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_rx->queue.slot_capacity = NILE_KERNEL_IO_CHAR_DEV_OP_QUEUE_CAPACITY;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].op_queue_rx->queue.flags |= IO_OP_RING_BUFFER_FLAGS_IS_CHAR_DEV | IO_OP_RING_BUFFER_FLAGS_IS_RX;

	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw = 0;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_init = pipe_hw_init;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_reset = pipe_hw_reset;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_tx_start = pipe_hw_tx_start;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_rx_start = pipe_hw_rx_start;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_tx_stop = pipe_hw_tx_stop;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_rx_stop = pipe_hw_rx_stop;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_tx_dynamic_gran_config_get = 0;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_rx_dynamic_gran_config_get = 0;

	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_tx_data_reg = 0;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_rx_data_reg = 0;

	kk->io_char_dev[CHAR_DEV_ID_PIPE0].tcb_owner = NULL;

	kk->io_char_dev[CHAR_DEV_ID_PIPE0].additional_data = NULL;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].additional_data_bytelen = 0;

	if (kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_init)
		kk->io_char_dev[CHAR_DEV_ID_PIPE0].hw_init(&kk->io_char_dev[CHAR_DEV_ID_PIPE0]);
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_CHAR_DEV;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_TX_ENABLED | NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_RX_ENABLED;
	kk->io_char_dev[CHAR_DEV_ID_PIPE0].flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_SHARED;

	kk->io_char_dev[CHAR_DEV_ID_PIPE0].tx_gran_config = 4U |
	NILE_KERNEL_IO_CHAR_DEV_GRN_MEM_WIDTH_32 |
	NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_WIDTH_32;

	kk->io_char_dev[CHAR_DEV_ID_PIPE0].rx_gran_config = 4U |
	NILE_KERNEL_IO_CHAR_DEV_GRN_MEM_WIDTH_32 |
	NILE_KERNEL_IO_CHAR_DEV_GRN_DEV_WIDTH_32;
	return 0;
}
