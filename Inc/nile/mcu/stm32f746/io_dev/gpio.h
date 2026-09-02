#ifndef NILE_MCU_STM32F746_IO_DEV_GPIO_H_
#define NILE_MCU_STM32F746_IO_DEV_GPIO_H_

#include "nile/memlayout.h"

void mcu_hw_gpio_alternate_function_set(GPIO_TypeDef *port, uint8_t pin, uint8_t af);
void mcu_hw_gpio_open_drain_set(GPIO_TypeDef *port, uint8_t pin);
void mcu_hw_gpio_push_pull_set(GPIO_TypeDef *port, uint8_t pin);
void mcu_hw_gpio_pullup_pulldown_disable(GPIO_TypeDef *port, uint8_t pin);
void mcu_hw_gpio_drive_speed_set(GPIO_TypeDef *port, uint8_t pin, uint8_t drive_speed);


#endif /* NILE_MCU_STM32F746_IO_DEV_GPIO_H_ */
