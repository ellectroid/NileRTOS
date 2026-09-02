#include "nile/kernel/scheduler.h"
NILE_USED void SysTick_Handler(void){
	nile_os_tick();
}
