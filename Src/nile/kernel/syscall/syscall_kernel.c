#include "nile/compiler.h"
#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/kernel/syscall_helper.h"
#include "nile/syscall_api.h"

uint32_t system_call_none(uint32_t *arg_list, uint32_t arg_cnt) {
	/* no-op */
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_OK;
}

/*
 * Arguments (2): uint32_t* output_ptr, uint32_t output_cnt
 * Output (1-5): output_ptr[0 .. output_cnt-1]
 *
 * Output fields:
 * 0: Kernel size in bytes
 * 1: Kernel heap space size in bytes
 * 2: Kernel version Major
 * 3: Kernel version Minor
 * 4: Kernel version Patch
 */
uint32_t system_call_kernel_info_version(uint32_t *arg_list, uint32_t arg_cnt) {
	/* return version major/minor/patch, kernel size, kernel heap size */
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	uint32_t retval = NILE_SYSCALL_RETVAL_OK;
	uint32_t *output;
	uint32_t output_cnt;

	/* Verify args */
	if (arg_cnt < 2) {
		retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
		goto skip;
	}
	output = (uint32_t*) arg_list[0];
	output_cnt = arg_list[1];

	if (!syscall_mem_range_in_curr_task_rw_mem(output, &output[output_cnt - 1])) {
		retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
		goto skip;
	}

	/* Args valid, produce output */
	switch (output_cnt) {
	default:
	case (5):
		output[4] = kk->info.version_patch;
	NILE_FALLTHROUGH;
	case (4):
		output[3] = kk->info.version_minor;
	NILE_FALLTHROUGH;
	case (3):
		output[2] = kk->info.version_major;
	NILE_FALLTHROUGH;
	case (2):
		output[1] = (uint32_t) kk->heap.heap.managed_memory_bytelen;
	NILE_FALLTHROUGH;
	case (1):
		output[0] = kk->info.kernel_size;
	NILE_FALLTHROUGH;
	case (0):
		break;
	}
	skip: return retval;
}

/*
 * Arguments (2): uint32_t* output_ptr, uint32_t output_cnt
 * Output (1-3): output_ptr[0 .. output_cnt-1]
 *
 * Output fields:
 * 0: OS tick real duration in microseconds
 * 1: OS tick hardware timer interrupt frequency (Hz)
 * 2: OS tick prescaler
 */
uint32_t system_call_kernel_info_timing(uint32_t *arg_list, uint32_t arg_cnt) {
    nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
    uint32_t retval = NILE_SYSCALL_RETVAL_OK;
    uint32_t *output;
    uint32_t output_cnt;

    /* Verify args */
    if (arg_cnt < 2) {
        retval = NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS;
        goto skip;
    }
    output = (uint32_t*) arg_list[0];
    output_cnt = arg_list[1];

    if (!syscall_mem_range_in_curr_task_rw_mem(output, &output[output_cnt - 1])) {
        retval = NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE;
        goto skip;
    }

    /* Args valid, produce output */
    switch (output_cnt) {
    default:
    case (3):
        output[2] = kk->scheduler.os_tick_prescaler;
        NILE_FALLTHROUGH;
    case (2):
        output[1] = kk->scheduler.os_tick_hardware_timer_interrupt_frequency;
        NILE_FALLTHROUGH;
    case (1):
        output[0] = kk->scheduler.os_tick_real_duration_us;
        NILE_FALLTHROUGH;
    case (0):
        break;
    }

skip:
    return retval;
}

uint32_t system_call_kernel_log(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	/* read/write kernel log */
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}

uint32_t system_call_kernel_reserved0(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_kernel_reserved1(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_kernel_reserved2(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
uint32_t system_call_kernel_reserved3(uint32_t *arg_list, uint32_t arg_cnt) {
	(void) arg_list;
	(void) arg_cnt;
	return NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED;
}
