#include "nile/kernel.h"
#include "nile/irq.h"
#include "nile/mcu/stm32f746/io_dev/gpio.h"
#include "app/io_dev_irq_priority.h"

static IRQn_Type usart_hw_identify_irqn(USART_TypeDef *hw) {
	IRQn_Type irqn = (IRQn_Type) -1;
	switch ((uintptr_t) hw) {
	case USART1_BASE:
		irqn = USART1_IRQn;
		break;
	case USART2_BASE:
		irqn = USART2_IRQn;
		break;
	case USART3_BASE:
		irqn = USART3_IRQn;
		break;
	case UART4_BASE:
		irqn = UART4_IRQn;
		break;
	case UART5_BASE:
		irqn = UART5_IRQn;
		break;
	case USART6_BASE:
		irqn = USART6_IRQn;
		break;
	case UART7_BASE:
		irqn = UART7_IRQn;
		break;
	case UART8_BASE:
		irqn = UART8_IRQn;
		break;

	default:
		irqn = (IRQn_Type) -1;
		break;
	}

	return irqn;
}

/* ----------------------------- */
/* UART priority level selector  */
/* ----------------------------- */

static uint32_t usart_hw_get_priority_level(IRQn_Type irqn) {
	uint32_t level = NILE_IRQ_PRIO_LVL_USR_0;

	switch (irqn) {
	case USART1_IRQn:
		level = UART1_IRQ_LEVEL;
		break;
	case USART2_IRQn:
		level = UART2_IRQ_LEVEL;
		break;
	case USART3_IRQn:
		level = UART3_IRQ_LEVEL;
		break;
	case UART4_IRQn:
		level = UART4_IRQ_LEVEL;
		break;
	case UART5_IRQn:
		level = UART5_IRQ_LEVEL;
		break;
	case USART6_IRQn:
		level = UART6_IRQ_LEVEL;
		break;
	case UART7_IRQn:
		level = UART7_IRQ_LEVEL;
		break;
	case UART8_IRQn:
		level = UART8_IRQ_LEVEL;
		break;
	default:
		level = NILE_IRQ_PRIO_LVL_USR_0;
		break;
	}

	return level;
}

static void usart_hw_clock_reset(IRQn_Type irqn) {
	switch (irqn) {

	case USART1_IRQn:
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
		break;

	case USART2_IRQn:
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;
		break;

	case USART3_IRQn:
		RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;
		break;

	case UART4_IRQn:
		RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_UART4RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_UART4RST;
		break;

	case UART5_IRQn:
		RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_UART5RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_UART5RST;
		break;

	case USART6_IRQn:
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
		RCC->APB2RSTR |= RCC_APB2RSTR_USART6RST;
		RCC->APB2RSTR &= ~RCC_APB2RSTR_USART6RST;
		break;

	case UART7_IRQn:
		RCC->APB1ENR |= RCC_APB1ENR_UART7EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_UART7RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_UART7RST;
		break;

	case UART8_IRQn:
		RCC->APB1ENR |= RCC_APB1ENR_UART8EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_UART8RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_UART8RST;
		break;

	default:
		break;
	}
}

static void usart_hw_gpio_config(IRQn_Type irqn) {
	switch (irqn) {

	case USART1_IRQn: /* PA9 TX, PB7 RX */
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
		RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

		mcu_hw_gpio_alternate_function_set(GPIOA, 9, 7);
		mcu_hw_gpio_push_pull_set(GPIOA, 9);
		mcu_hw_gpio_pullup_pulldown_disable(GPIOA, 9);
		mcu_hw_gpio_drive_speed_set(GPIOA, 9, 0xFF);

		mcu_hw_gpio_alternate_function_set(GPIOB, 7, 7);
		mcu_hw_gpio_push_pull_set(GPIOB, 7);
		mcu_hw_gpio_pullup_pulldown_disable(GPIOB, 7);
		mcu_hw_gpio_drive_speed_set(GPIOB, 7, 0xFF);
		break;

		/* other periphs here */

	default:
		break;
	}
}

static void usart_hw_clk_src_cpu_config(IRQn_Type irqn) {
	uint32_t shift = 0U;

	switch (irqn) {
	case USART1_IRQn:
		shift = 0U;
		break; /* USART1SEL bits [1:0]  */
	case USART2_IRQn:
		shift = 2U;
		break; /* USART2SEL bits [3:2]  */
	case USART3_IRQn:
		shift = 4U;
		break; /* USART3SEL bits [5:4]  */
	case UART4_IRQn:
		shift = 6U;
		break; /* UART4SEL  bits [7:6]  */
	case UART5_IRQn:
		shift = 8U;
		break; /* UART5SEL  bits [9:8]  */
	case USART6_IRQn:
		shift = 10U;
		break; /* USART6SEL bits [11:10]*/
	case UART7_IRQn:
		shift = 12U;
		break; /* UART7SEL  bits [13:12]*/
	case UART8_IRQn:
		shift = 14U;
		break; /* UART8SEL  bits [15:14]*/
	default: //will never happen
		break;
	}
	RCC->DCKCFGR2 = (RCC->DCKCFGR2 & ~(0x03 << shift)) | (0x01 << shift);
}

void usart_hw_init(struct nile_kernel_io_char_dev *dev) {
	USART_TypeDef *USART = (USART_TypeDef*) dev->hw;
	IRQn_Type irqn = usart_hw_identify_irqn(USART);
	const uint32_t default_baud_rate = 9600;

	nile_irq_disable(irqn);
	usart_hw_clock_reset(irqn);
	usart_hw_gpio_config(irqn);
	usart_hw_clk_src_cpu_config(irqn);

	USART->CR1 &= ~USART_CR1_M;
	USART->CR1 &= ~USART_CR1_OVER8;
	USART->CR3 |= USART_CR3_OVRDIS;
	USART->CR3 |= USART_CR3_EIE; //error interrupt

	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	uint32_t BRR_Val = kk->scheduler.cpu_frequency / default_baud_rate;
	USART->BRR = BRR_Val;

	USART->CR1 |= USART_CR1_UE;

	dev->flags &= ~(NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_ACTIVE |
	NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_TX_ACTIVE |
	NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_HW_RX_ACTIVE |
	NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR |
	NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_ERROR_CODE_MASK);

	dev->flags |= NILE_KERNEL_IO_CHAR_DEV_FLAG_DEV_READY;

	nile_irq_clear_pending(irqn);
	nile_irq_set_priority(irqn, nile_irq_get_raw_priority_value(usart_hw_get_priority_level(irqn)));
	nile_irq_enable(irqn);
}

void usart_hw_reset(struct nile_kernel_io_char_dev *dev) {
	usart_hw_init(dev);
}

void usart_hw_tx_start(struct nile_kernel_io_char_dev *dev) {
	//use in critical sections to maintain data structure validity
	USART_TypeDef *USART = (USART_TypeDef*) dev->hw;
	USART->CR1 |= USART_CR1_TE;
	USART->CR1 |= USART_CR1_TXEIE;
}

void usart_hw_rx_start(struct nile_kernel_io_char_dev *dev) {
	//use in critical sections to maintain data structure validity
	USART_TypeDef *USART = (USART_TypeDef*) dev->hw;
	USART->RQR = USART_RQR_RXFRQ;
	USART->ICR = USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_PECF | USART_ICR_NCF;
	USART->CR1 |= USART_CR1_RE;
	USART->CR1 |= USART_CR1_RXNEIE;
}

uint16_t usart_hw_tx_dynamic_gran_config_get(struct nile_kernel_io_char_dev *dev) {
	return dev->tx_gran_config;
}

uint16_t usart_hw_rx_dynamic_gran_config_get(struct nile_kernel_io_char_dev *dev) {
	return dev->rx_gran_config;
}

void usart_hw_tx_stop(struct nile_kernel_io_char_dev *dev) {
	//use in critical sections to maintain data structure validity
	USART_TypeDef *USART = (USART_TypeDef*) dev->hw;
	USART->CR1 &= ~USART_CR1_TXEIE;
	USART->CR1 &= ~USART_CR1_TE;
}

void usart_hw_rx_stop(struct nile_kernel_io_char_dev *dev) {
	//use in critical sections to maintain data structure validity
	USART_TypeDef *USART = (USART_TypeDef*) dev->hw;
	USART->CR1 &= ~USART_CR1_RXNEIE;
	USART->CR1 &= ~USART_CR1_RE;
	USART->RQR = USART_RQR_RXFRQ;
}
