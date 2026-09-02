#ifndef NILE_KERNEL_SYSCALL_HELPER_H_
#define NILE_KERNEL_SYSCALL_HELPER_H_

bool syscall_mem_addr_in_curr_task_stack_mem(void* addr);
bool syscall_mem_range_in_curr_task_stack_mem(void* addr_first, void* addr_last);
bool syscall_mem_addr_in_curr_task_rw_mem(void *addr);
bool syscall_mem_range_in_curr_task_rw_mem(void *addr_first, void *addr_last);

#endif /* NILE_KERNEL_SYSCALL_HELPER_H_ */
