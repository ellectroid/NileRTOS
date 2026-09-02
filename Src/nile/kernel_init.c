#include "nile/kernel.h"

#include "nile/memprot.h"
#include "nile/ctx_init.h"
#include "nile/fpu.h"
#include "nile/syscall_api.h"
#include "nile/exception.h"

#include "nile/kernel/syscall_handlers.h"
#include "nile/kernel/io_op_queue.h"

void nile_idle_task(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
uint32_t nile_kernel_init() {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	//Step 1: clear the kernel memory
	unsigned long kk_size = sizeof(nile_kernel);
	uint8_t *kk_mem = (uint8_t*) kk;
	uint8_t *kk_end = kk_mem + kk_size;
	for (uint8_t *kernel_ptr = kk_mem; kernel_ptr < kk_end; kernel_ptr++) {
		*kernel_ptr = 0x00;
	}

	//Step 2: initialize the info structure
	kk->info.version_major = NILE_KERNEL_INFO_VERSION_MAJOR;
	kk->info.version_minor = NILE_KERNEL_INFO_VERSION_MINOR;
	kk->info.version_patch = NILE_KERNEL_INFO_VERSION_PATCH;
	kk->info.kernel_size = kk_size;

	//Step 3: initialize the system report structure
	kk->status.kernel_log_len = NILE_SYSTEM_STATUS_LOG_MEM_SIZE;

	//Step 4: initialize scheduler structure
	kk->scheduler.cpu_frequency = MCU_INIT_CPU_FREQUENCY;
	kk->scheduler.kernel_space_stack_pointer_base = NILE_MEMORY_KERNEL_STACK_START;
	kk->scheduler.os_tick_hardware_timer_interrupt_frequency = 1000; //1000Hz, 1ms
	kk->scheduler.os_tick_counter = 0;
	kk->scheduler.os_tick_prescaler = 1;
	kk->scheduler.os_tick_prescaler_counter = 1; //always 1 for instant startup
	kk->scheduler.os_tick_real_duration_ns = (1000000000 / kk->scheduler.os_tick_hardware_timer_interrupt_frequency) * kk->scheduler.os_tick_prescaler; //Default 1ms * prescaler
	kk->scheduler.os_tick_real_duration_us = (1000000 / kk->scheduler.os_tick_hardware_timer_interrupt_frequency) * kk->scheduler.os_tick_prescaler;
	kk->scheduler.os_scheduler_flags = 0;

	// Step 5: initialize syscall vtable
	kk->syscall_vtable.vector_count = NILE_SYSCALL_VECTOR_COUNT;

	/* NONE */
	kk->syscall_vtable.vector[NILE_SYSCALL_NONE] = system_call_none;

	/* Kernel Info / Debug */
	kk->syscall_vtable.vector[NILE_SYSCALL_KERNEL_INFO_VERSION] = system_call_kernel_info_version;
	kk->syscall_vtable.vector[NILE_SYSCALL_KERNEL_INFO_TIMING] = system_call_kernel_info_timing;
	kk->syscall_vtable.vector[NILE_SYSCALL_KERNEL_LOG] = system_call_kernel_log;
	kk->syscall_vtable.vector[NILE_SYSCALL_KERNEL_RESERVED0] = system_call_kernel_reserved0;
	kk->syscall_vtable.vector[NILE_SYSCALL_KERNEL_RESERVED1] = system_call_kernel_reserved1;
	kk->syscall_vtable.vector[NILE_SYSCALL_KERNEL_RESERVED2] = system_call_kernel_reserved2;
	kk->syscall_vtable.vector[NILE_SYSCALL_KERNEL_RESERVED3] = system_call_kernel_reserved3;

	/* Task Management */
	kk->syscall_vtable.vector[NILE_SYSCALL_TASK_YIELD] = system_call_task_yield;
	kk->syscall_vtable.vector[NILE_SYSCALL_TASK_BLOCK] = system_call_task_block;
	kk->syscall_vtable.vector[NILE_SYSCALL_TASK_RESERVED0] = system_call_task_reserved0;
	kk->syscall_vtable.vector[NILE_SYSCALL_TASK_RESERVED1] = system_call_task_reserved1;
	kk->syscall_vtable.vector[NILE_SYSCALL_TASK_RESERVED2] = system_call_task_reserved2;
	kk->syscall_vtable.vector[NILE_SYSCALL_TASK_RESERVED3] = system_call_task_reserved3;
	kk->syscall_vtable.vector[NILE_SYSCALL_TASK_RESERVED4] = system_call_task_reserved4;
	kk->syscall_vtable.vector[NILE_SYSCALL_TASK_RESERVED5] = system_call_task_reserved5;

	/* IO: Char devices */
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_CHAR_DEV_IOCTL] = system_call_io_char_dev_ioctl;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_CHAR_DEV_OPEN] = system_call_io_char_dev_open;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_CHAR_DEV_CLOSE] = system_call_io_char_dev_close;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_CHAR_DEV_READ] = system_call_io_char_dev_read;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_CHAR_DEV_WRITE] = system_call_io_char_dev_write;

	/* IO: Block devices */
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_BLOCK_DEV_IOCTL] = system_call_io_block_dev_ioctl;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_BLOCK_DEV_OPEN] = system_call_io_block_dev_open;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_BLOCK_DEV_CLOSE] = system_call_io_block_dev_close;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_BLOCK_DEV_READ] = system_call_io_block_dev_read;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_BLOCK_DEV_WRITE] = system_call_io_block_dev_write;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_BLOCK_DEV_ERASE] = system_call_io_block_dev_erase;

	/* IO: Reserved */
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_RESERVED0] = system_call_io_reserved0;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_RESERVED1] = system_call_io_reserved1;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_RESERVED2] = system_call_io_reserved2;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_RESERVED3] = system_call_io_reserved3;
	kk->syscall_vtable.vector[NILE_SYSCALL_IO_RESERVED4] = system_call_io_reserved4;

	//Step 6: initialize vtable
	kk->vtable.vector_count = NILE_VTABLE_VECTOR_COUNT;
//	kk->vtable.vector[0] = 0;//

	//Step 7: initialize kheap
	kk->heap.init = tlsf_allocator_init;
	kk->heap.alloc = tlsf_alloc;
	kk->heap.free = tlsf_free;
	kheap_init();

	//Step 8: initialize TCB for idle task
	const int idle_task_stack_space_size = 64;
	const int idle_task_stack_space_align = 8;
	unsigned char *idle_task_stackspace = kalloc(idle_task_stack_space_size, idle_task_stack_space_align); //64 bytes - 16 words of stack space for idle task
	nile_kernel_tcb *idle_task = kalloc(sizeof(nile_kernel_tcb), 0);
	for (uint8_t *ptr = (uint8_t*) idle_task; ptr < (uint8_t*) (idle_task + 1); ptr++) {
		*ptr = 0;
	}
	if (!idle_task || !idle_task_stackspace) {
		return 0x01;
	}
	kk->scheduler.tcb_idle_task = idle_task;
	idle_task->scheduling_priority_base = 0;
	idle_task->scheduling_priority_current = idle_task->scheduling_priority_base;
	idle_task->scheduling_burst_time_base = 1;
	idle_task->scheduling_burst_time_remaining = idle_task->scheduling_burst_time_base;
	idle_task->stack_size = idle_task_stack_space_size;
	idle_task->stack_base_address = (uint32_t) (idle_task_stackspace + idle_task_stack_space_size);
	idle_task->flags = 0;

	idle_task->memprot_regions[0].base = NILE_MCU_FLASH_AXI;          // flash base
	idle_task->memprot_regions[0].size = NILE_MCU_FLASH_LEN;          // flash size
	idle_task->memprot_regions[0].perms = NILE_MEMPROT_PERM_RO_FULL;   // full read, no write
	idle_task->memprot_regions[0].type = NILE_MEMPROT_MEM_NORMAL;     // normal memory
	idle_task->memprot_regions[0].executable = true;                        // code allowed

	idle_task->memprot_regions[1].base = NILE_MCU_RAM_START;          // RAM base
	idle_task->memprot_regions[1].size = 1 << 20;                     // RAM size
	idle_task->memprot_regions[1].perms = NILE_MEMPROT_PERM_RW_FULL;   // full read/write
	idle_task->memprot_regions[1].type = NILE_MEMPROT_MEM_NORMAL;     // normal cached RAM
	idle_task->memprot_regions[1].executable = false;                       // XN

	for (int i = 2; i < NILE_KERNEL_TASK_MEMPROT_REGION_COUNT; ++i) {
		idle_task->memprot_regions[i].base = 0;
		idle_task->memprot_regions[i].size = 0;
		idle_task->memprot_regions[i].perms = NILE_MEMPROT_PERM_NONE;
		idle_task->memprot_regions[i].type = NILE_MEMPROT_MEM_NORMAL;
		idle_task->memprot_regions[i].executable = false;
	}

	tcb_set_fpu_config(idle_task, 0, 0);
	nile_kernel_tcb_startup_frame_gp idle_task_startup_frame = { 0 };
	idle_task_startup_frame.startup_sp = idle_task->stack_base_address;
	idle_task_startup_frame.startup_pc = (uint32_t) nile_idle_task;
	for (int i = 0; i < 31; i++) {
		idle_task_startup_frame.startup_gp_reg[i] = i;
	}

	ctx_init_gp(idle_task, &idle_task_startup_frame);

	//Step 9: initialize system fault exceptions
	nile_exc_enable(nile_exc_get_raw_exception_id(NILE_EXC_ID_BUS_ERROR));
	nile_exc_set_priority(nile_exc_get_raw_exception_id(NILE_EXC_ID_BUS_ERROR), nile_irq_get_raw_priority_value(NILE_IRQ_PRIO_LVL_SYS_EXC));

	nile_exc_enable(nile_exc_get_raw_exception_id(NILE_EXC_ID_CRITICAL_EVENT));
	nile_exc_set_priority(nile_exc_get_raw_exception_id(NILE_EXC_ID_CRITICAL_EVENT), nile_irq_get_raw_priority_value(NILE_IRQ_PRIO_LVL_SYS_EXC));

	nile_exc_enable(nile_exc_get_raw_exception_id(NILE_EXC_ID_FATAL_ERROR));
	nile_exc_set_priority(nile_exc_get_raw_exception_id(NILE_EXC_ID_FATAL_ERROR), nile_irq_get_raw_priority_value(NILE_IRQ_PRIO_LVL_SYS_EXC));

	nile_exc_enable(nile_exc_get_raw_exception_id(NILE_EXC_ID_BAD_INSTRUCTION));
	nile_exc_set_priority(nile_exc_get_raw_exception_id(NILE_EXC_ID_BAD_INSTRUCTION), nile_irq_get_raw_priority_value(NILE_IRQ_PRIO_LVL_SYS_EXC));

	//Step 10: configure FPU
	if (NILE_HW_FPU_PRIV) {
		if (NILE_HW_FPU_UNPRIV) {
			nile_fpu_enable_full_access();
		} else {
			nile_fpu_enable_privileged_only();
		}
	} else {
		nile_fpu_disable();
	}

	return 0;
}
