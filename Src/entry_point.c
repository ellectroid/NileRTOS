#include "mcu_reset_init.h"
#include "app/startup.h"

int main(void) {

	uint32_t init_ok = 0;
	volatile uint32_t return_code = init_ok;
	volatile uint32_t init_stage = 0;
	mcu_reset_mcu_clock_set_max();

	return_code = nile_kernel_init();
	if (return_code != init_ok) {
		while (1);
	}

	init_stage++;
	return_code = nile_kernel_init_devices();
	if (return_code != init_ok) {
		while (1);
	}

	init_stage++;
	return_code = task0_init();
	if (return_code != init_ok) {
		while (1);
	}

	init_stage++;
	return_code = task1_init();
	if (return_code != init_ok) {
		while (1);
	}

	init_stage++;
	nile_kernel_start(); //noreturn

	for (;;);
}
