#ifndef NILE_KERNEL_IO_DEV_IRQ_HANDLER_IO_CHAR_DEV_H_
#define NILE_KERNEL_IO_DEV_IRQ_HANDLER_IO_CHAR_DEV_H_
#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/kernel/io_op_queue.h"

void io_char_dev_event_hw_rx_data_rdy_handler(nile_kernel_io_char_dev *dev);
void io_char_dev_event_hw_tx_data_write_rdy_handler(nile_kernel_io_char_dev *dev);
void io_char_dev_event_hw_rx_error_handler(nile_kernel_io_char_dev *dev, uint8_t hw_specific_errcode);

#endif /* NILE_KERNEL_IO_DEV_IRQ_HANDLER_IO_CHAR_DEV_H_ */
