#include "nile/compiler.h"
#include "nile/stdtypes.h"

NILE_USED void task2(uint32_t arg0, uint32_t arg1, float arg2, float arg3) {
	(void) arg0;
	(void) arg1;
	float temp = 1.001f * arg2 + arg3;
	volatile float temp2 = 100.0f;
	while (1) {
		temp2 *= temp;
		if(((*(uint32_t*)&temp2 & 0x7FFFFFFFu) == 0x7F800000u)){
			temp2 = 100.0f;
		}
	}
}
