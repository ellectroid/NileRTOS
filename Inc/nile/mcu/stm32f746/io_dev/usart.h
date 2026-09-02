#ifndef NILE_MCU_STM32F746_IO_DEV_USART_H_
#define NILE_MCU_STM32F746_IO_DEV_USART_H_

#include "nile/kernel/io_dev.h"

enum {
    USART_DEV_ERRCODE_RX_NONE = 0,
    USART_DEV_ERRCODE_RX_OP_ABORTED,
    USART_DEV_ERRCODE_RX_FRAME_ERR,
    USART_DEV_ERRCODE_RX_NOISE_ERR,
    USART_DEV_ERRCODE_RX_OVERRUN_ERR,
    USART_DEV_ERRCODE_RX_PARITY_ERR,
};

void usart_hw_init(struct nile_kernel_io_char_dev *dev);
void usart_hw_reset(struct nile_kernel_io_char_dev *dev);
void usart_hw_tx_start(struct nile_kernel_io_char_dev *dev);
void usart_hw_rx_start(struct nile_kernel_io_char_dev *dev);
uint16_t usart_hw_tx_dynamic_gran_config_get(struct nile_kernel_io_char_dev *dev);
uint16_t usart_hw_rx_dynamic_gran_config_get(struct nile_kernel_io_char_dev *dev);
void usart_hw_tx_stop(struct nile_kernel_io_char_dev *dev);
void usart_hw_rx_stop(struct nile_kernel_io_char_dev *dev);

#endif /* NILE_MCU_STM32F746_IO_DEV_USART_H_ */
