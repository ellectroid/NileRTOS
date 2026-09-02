#include "nile/kernel/io_dev/irq_handler_io_char_dev.h"
#include "nile/mcu/stm32f746/io_dev/usart.h"
#include "app/io_dev_id.h"

void USART1_IRQHandler(void) {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	nile_kernel_io_char_dev *dev = &kk->io_char_dev[CHAR_DEV_ID_USART1];
	USART_TypeDef *USART = USART1;

	uint32_t isr = USART->ISR;

	//if rx error happened
	if (isr & (USART_ISR_ORE | USART_ISR_FE | USART_ISR_NE | USART_ISR_PE)) {
		USART->ICR = USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_PECF | USART_ICR_NCF;
		uint8_t err_code;
		if (isr & USART_ISR_ORE)
			err_code = USART_DEV_ERRCODE_RX_OVERRUN_ERR;
		if (isr & USART_ISR_FE)
			err_code = USART_DEV_ERRCODE_RX_FRAME_ERR;
		if (isr & USART_ISR_NE)
			err_code = USART_DEV_ERRCODE_RX_NOISE_ERR;
		if (isr & USART_ISR_PE)
			err_code = USART_DEV_ERRCODE_RX_PARITY_ERR;
		io_char_dev_event_hw_rx_error_handler(dev, err_code);
		return;
	}

	//if received a byte
	if (isr & USART_ISR_RXNE) {
		io_char_dev_event_hw_rx_data_rdy_handler(dev);
		return;
	}
	//if there is a byte to send
	if (isr & USART_ISR_TXE) {
		io_char_dev_event_hw_tx_data_write_rdy_handler(dev);
		return;
	}

}
