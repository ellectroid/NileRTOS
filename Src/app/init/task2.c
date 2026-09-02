#include "app/startup.h"

uint32_t task2_init() {
	nile_kernel_tcb_fpu *const new_task_fpu = kalloc(sizeof(nile_kernel_tcb_fpu), 8);
	if (!new_task_fpu) {
		return 0xFFFFFFFF;
	}
	for (uint8_t *it = (uint8_t*) new_task_fpu; it < ((uint8_t*) new_task_fpu + sizeof(nile_kernel_tcb_fpu)); it++) {
		*it = 0x00;
	}
	nile_kernel_tcb *const new_task = (nile_kernel_tcb*) new_task_fpu;
	new_task->task_id = 2;
	new_task->scheduling_priority_base = 11;
	new_task->scheduling_priority_current = new_task->scheduling_priority_base;
	new_task->scheduling_burst_time_base = 1;
	new_task->scheduling_burst_time_remaining = new_task->scheduling_burst_time_base;
	new_task->scheduling_dynamic_priority_wait_time_prescaler = 1;
	new_task->scheduling_dynamic_priority_wait_time_counter = 0;
	new_task->scheduling_dynamic_priority_step = 1;
	new_task->stack_size = TASK2_STACKSPACE_SIZE;
	new_task->stack_base_address = TASK2_STACKSPACE_BASE;

	new_task->memprot_regions[0].base = NILE_MCU_FLASH_AXI;          // flash base
	new_task->memprot_regions[0].size = NILE_MCU_FLASH_LEN;          // flash size
	new_task->memprot_regions[0].perms = NILE_MEMPROT_PERM_RO_FULL;  // full read, no write
	new_task->memprot_regions[0].type = NILE_MEMPROT_MEM_NORMAL;     // normal memory
	new_task->memprot_regions[0].executable = true;                  // code allowed

	new_task->memprot_regions[1].base = NILE_MCU_RAM_START;          // RAM base
	new_task->memprot_regions[1].size = 1 << 20;                     // RAM size
	new_task->memprot_regions[1].perms = NILE_MEMPROT_PERM_RW_FULL;  // full read/write
	new_task->memprot_regions[1].type = NILE_MEMPROT_MEM_NORMAL;     // normal cached RAM
	new_task->memprot_regions[1].executable = false;                 // XN

	for (int i = 2; i < NILE_KERNEL_TASK_MEMPROT_REGION_COUNT; ++i) {
		new_task->memprot_regions[i].base = 0;
		new_task->memprot_regions[i].size = 0;  //size 0 disables the region
		new_task->memprot_regions[i].perms = NILE_MEMPROT_PERM_NONE;
		new_task->memprot_regions[i].type = NILE_MEMPROT_MEM_NORMAL;
		new_task->memprot_regions[i].executable = false;
	}

	tcb_set_fpu_config(new_task, true, false);

	nile_kernel_tcb_startup_frame_gp_fpu new_task_startup_frame = { 0 };
	new_task_startup_frame.startup_sp = new_task->stack_base_address;
	new_task_startup_frame.startup_pc = (uint32_t) task2;
	for (int i = 0; i < 32; i++) {
		new_task_startup_frame.startup_gp_reg[i] = 0 + i;
		new_task_startup_frame.startup_fpu_reg_f[i] = 200.0f + (float) i;
	}
	ctx_init_gp_fpu(new_task_fpu, &new_task_startup_frame);

	tcb_queue_task_init_add(new_task);
	return 0; //OK
}
