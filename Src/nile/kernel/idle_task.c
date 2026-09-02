#include "nile/kernel.h"
#include "nile/barriers.h"

void nile_idle_task(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)(arg3);
	while (1) {
		volatile uint32_t tickless_idle = (volatile uint32_t)((volatile uint32_t)kk->scheduler.os_scheduler_flags & NILE_KERNEL_OS_SCHEDULER_FLAGS_TICKLESS_IDLE);
		//kk->status.kernel_log.mem32[0]++;
		if(tickless_idle){
			//disable os tick source
			nile_wfi();
		}
		else{
			nile_wfi();
		}
	}
}
