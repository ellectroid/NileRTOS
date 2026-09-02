#include "nile/kernel.h"

bool syscall_mem_addr_in_curr_task_stack_mem(void *addr) {
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	nile_kernel_tcb *current_task = kk->scheduler.tcb_current_task;
	uint32_t a = (uint32_t) addr;
	uint32_t hi = current_task->stack_base_address;
	uint32_t lo = current_task->stack_base_address - current_task->stack_size;
	return (a >= lo) && (a < hi);
}

bool syscall_mem_range_in_curr_task_stack_mem(void *addr_first, void *addr_last) {
	return syscall_mem_addr_in_curr_task_stack_mem(addr_first) && syscall_mem_addr_in_curr_task_stack_mem(addr_last);
}

bool syscall_mem_addr_in_curr_task_rw_mem(void *addr) {
	nile_kernel *kk;
	nile_kernel_tcb *current;
	uintptr_t a;
	uintptr_t base;
	size_t size;
	uint32_t perms;

	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	current = kk->scheduler.tcb_current_task;
	a = (uintptr_t) addr;

#if NILE_KERNEL_TASK_MEMPROT_REGION_PRECEDENCE_ASCENDING
	// Higher index = higher priority - check N-1..0
	for (int32_t i = NILE_KERNEL_TASK_MEMPROT_REGION_COUNT - 1; i >= 0; i--)
#else
    // Lower index = higher priority - check 0..N-1
    for (int32_t i = 0; i < NILE_KERNEL_TASK_MEMPROT_REGION_COUNT; i++)
#endif

			{
		base = current->memprot_regions[i].base;
		size = current->memprot_regions[i].size;
		perms = current->memprot_regions[i].perms;

		if (size == 0)
			continue;

		if (a >= base && a < (base + size)) {
			if (perms == NILE_MEMPROT_PERM_RW_FULL)
				return true;
			return false;
		}
	}

	return false;
}

bool syscall_mem_range_in_curr_task_rw_mem(void *addr_first, void *addr_last) {
	nile_kernel *kk;
	nile_kernel_tcb *current;
	uintptr_t a_first;
	uintptr_t a_last;
	uintptr_t base;
	size_t size;
	uint32_t perms;
	bool first_allowed;
	bool last_allowed;

	kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	current = kk->scheduler.tcb_current_task;

	a_first = (uintptr_t) addr_first;
	a_last = (uintptr_t) addr_last;

	first_allowed = false;
	last_allowed = false;

#if NILE_KERNEL_TASK_MEMPROT_REGION_PRECEDENCE_ASCENDING
	// Higher index = higher priority - check N-1..0
	for (int32_t i = NILE_KERNEL_TASK_MEMPROT_REGION_COUNT - 1; i >= 0; i--)
#else
    // Lower index = higher priority - check 0..N-1
    for (int32_t i = 0; i < NILE_KERNEL_TASK_MEMPROT_REGION_COUNT; i++)
#endif
			{
		base = current->memprot_regions[i].base;
		size = current->memprot_regions[i].size;
		perms = current->memprot_regions[i].perms;

		if (size == 0)
			continue;

		if (!first_allowed && a_first >= base && a_first < (base + size)) {
			if (perms == NILE_MEMPROT_PERM_RW_FULL)
				first_allowed = true;
			else
				return false;
		}

		if (!last_allowed && a_last >= base && a_last < (base + size)) {
			if (perms == NILE_MEMPROT_PERM_RW_FULL)
				last_allowed = true;
			else
				return false;
		}
	}

	return first_allowed && last_allowed;
}

