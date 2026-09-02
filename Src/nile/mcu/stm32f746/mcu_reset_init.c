#include "nile/memlayout.h"

void mcu_reset_mcu_clock_set_max() {
	/* Activate High Speed External Oscillator */
	RCC->CR |= (0x01 << 16); //Enable
	while (!(RCC->CR & (0x01 << 17))); //Wait until HSE is ready

	/* Activate PWR peripheral */
	RCC->APB1ENR |= (0x01 << 28);
	while (!(RCC->APB1ENR & (0x01 << 28))); //Waste cycles

	/* Activate internal regulator, overdrive and switching overdrive */
	PWR->CR1 = (PWR->CR1 & ~(0x03 << 14)) | (0x03 << 14); //Regulator voltage scale 1 mode
	PWR->CR1 |= (0x01 << 16); //Overdrive enable
	while (!(PWR->CSR1 & (0x01 << 16))); //Wait until overdrive enabled
	PWR->CR1 |= (0x01 << 17); //Overdrive switching enable
	while (!(PWR->CSR1 & (0x01 << 17))); //Wait until overdrive switching enabled

	/* Configure PLL */
	RCC->PLLCFGR |= (0x01 << 22); //PLL Source: HSE
	RCC->PLLCFGR = (RCC->PLLCFGR & ~(0x1F << 0)) | (25U << 0); //PLLM = 25
	RCC->PLLCFGR = (RCC->PLLCFGR & ~(0x1FF << 6)) | (432U << 6); //PLLN = 432
	RCC->PLLCFGR = (RCC->PLLCFGR & ~(0x03 << 16)) | (0U << 16); //PLLP = 2
	RCC->PLLCFGR = (RCC->PLLCFGR & ~(0x0F << 24)) | (9U << 24); //PLLQ = 9 (48MHz to PLL48CLK)
	RCC->CR |= (0x01 << 24); //PLL enable
	while (!(RCC->CR & (0x01 << 25))); //Wait until PLL ready

	/* Configure Flash wait states, ART accelerator, prefetch */
	FLASH->ACR |= (0x07 << 0) | (0x01 << 8) | (0x01 << 9); //7 wait states, enable prefetch, enable ART accelerator

	/* Configure AHB/APB prescalers */
	RCC->CFGR = (RCC->CFGR & ~(0x0F << 4)) | (0x00 << 4); //AHB Prescaler = 1
	RCC->CFGR = (RCC->CFGR & ~(0x07 << 10)) | (0x05 << 10); //APB1 Prescaler = 4
	RCC->CFGR = (RCC->CFGR & ~(0x07 << 13)) | (0x04 << 13); //APB Prescaler = 2

	/* Switch to PLL as system clock */
	RCC->CFGR = (RCC->CFGR & (0x03 << 0)) | (0x02 << 0);
	while (!(RCC->CFGR & (0x03 << 2))); //Wait until system clock switched
}
