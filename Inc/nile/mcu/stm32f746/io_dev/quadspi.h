#ifndef NILE_MCU_STM32F746_IO_DEV_QUADSPI_H_
#define NILE_MCU_STM32F746_IO_DEV_QUADSPI_H_

#include "nile/mcu/stm32f746/io_dev/quadspi_flash_ic.h"

/* ---------- QUADSPI Definitions  ---------- */

#define QSPI_ADSIZE_8 ((uint8_t)0x00)
#define QSPI_ADSIZE_16 ((uint8_t)0x01)
#define QSPI_ADSIZE_24 ((uint8_t)0x02)
#define QSPI_ADSIZE_32 ((uint8_t)0x03)

#define QSPI_SKIP ((uint8_t)0x00)
#define QSPI_SINGLE ((uint8_t)0x01)
#define QSPI_DOUBLE ((uint8_t)0x02)
#define QSPI_QUAD ((uint8_t)0x03)

#define QSPI_FMODE_INDIRECT_WRITE ((uint8_t)0x00)
#define QSPI_FMODE_INDIRECT_READ ((uint8_t)0x01)
#define QSPI_FMODE_AUTOMATIC_POLLING ((uint8_t)0x02)
#define QSPI_FMODE_MEMORY_MAPPED ((uint8_t)0x03)

#define QSPI_MEMMAPPED_ADDRESS  (0x90000000UL)

enum {
    QSPI_DEV_ERRCODE_NONE = 0,
    QSPI_DEV_ERRCODE_OP_ABORTED,
    QSPI_DEV_ERRCODE_TRANSFER_ERR,
	QSPI_DEV_ERRCODE_BAD_OP_LENGTH,
};


/* ---------- QUADSPI Functions ---------- */

void quadspi_hw_init(struct nile_kernel_io_block_dev *dev);
void quadspi_memory_mapped_mode_enable(struct nile_kernel_io_block_dev *dev);
void quadspi_memory_mapped_mode_disable(struct nile_kernel_io_block_dev *dev);
void quadspi_hw_rqx_start(struct nile_kernel_io_block_dev *dev);
void quadspi_hw_stop(struct nile_kernel_io_block_dev *dev);
void quadspi_operation_error_handler(struct nile_kernel_io_block_dev *dev, uint8_t hw_specific_errcode);
void quadspi_write_cmd_issue(struct nile_kernel_io_block_dev *dev);
void quadspi_erase_cmd_issue(struct nile_kernel_io_block_dev *dev);
void quadspi_cmd_finished(struct nile_kernel_io_block_dev *dev);
void quadspi_write_cmd_load_next_data(struct nile_kernel_io_block_dev *dev);

#endif /* NILE_MCU_STM32F746_IO_DEV_QUADSPI_H_ */
