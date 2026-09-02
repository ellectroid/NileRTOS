#include "nile/memlayout.h"

void mcu_hw_gpio_alternate_function_set(GPIO_TypeDef *port, uint8_t pin, uint8_t af) {
	port->MODER = (port->MODER & ~(0x03 << (pin << 1))) | (0x02 << (pin << 1)); //set moder register to alternate function
	port->AFR[pin >> 3] = (port->AFR[pin >> 3]
			& (~(0xFU << ((pin << 2) & (~(0x01 << 5U))))))
			| (af << ((pin << 2) & ~(0x01 << 5U)));  //set alternative function
}

void mcu_hw_gpio_open_drain_set(GPIO_TypeDef *port, uint8_t pin) {
	pin = pin & 0x0f;
	port->OTYPER |= (0x01 << pin);  // output open drain
}
void mcu_hw_gpio_push_pull_set(GPIO_TypeDef *port, uint8_t pin) {
	pin = pin & 0x0f;
	port->OTYPER &= ~(0x01 << pin);  // output push-pull
}
void mcu_hw_gpio_pullup_pulldown_disable(GPIO_TypeDef *port, uint8_t pin) {
	pin = pin & 0x0f;
	port->PUPDR &= ~(0x03 << (pin << 1));  //disable pull up and pull down
}
void mcu_hw_gpio_drive_speed_set(GPIO_TypeDef *port, uint8_t pin, uint8_t drive_speed) {
	pin = pin & 0x0f;
	if (drive_speed > 0x03)
		drive_speed = 0x03;
	port->OSPEEDR = ((port->OSPEEDR) & ~(0x03 << (pin << 1)))
			| (drive_speed << (pin << 1));  //set output speed

}

