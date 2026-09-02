#include "nile/compiler.h"
#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/syscall_api.h"
#include "app/io_dev_id.h"

NILE_USED void task1(uint32_t arg0, uint32_t arg1, float arg2, float arg3) {
	(void) arg0;
	(void) arg1;
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	float temp = 1.001f * arg2 + arg3;
	union {
		uint32_t data_buffer_align;
		uint8_t data_buffer[8];
	} buf1;
	nile_syscall_params_io_op io1 = { 0 };
	io1.dev_id = CHAR_DEV_ID_PIPE0;
	io1.data_buffer = &buf1.data_buffer_align;
	io1.data_len = 8;
	io1.op_flags = 0;

	buf1.data_buffer[0] = 'm';
	buf1.data_buffer[1] = 'e';
	buf1.data_buffer[2] = 'o';
	buf1.data_buffer[3] = 'w';
	buf1.data_buffer[4] = 0x01;
	buf1.data_buffer[5] = 0x02;
	buf1.data_buffer[6] = 0x03;
	buf1.data_buffer[7] = 0x04;

	nile_syscall_params_task_block tsk_blk = { 0 };
	tsk_blk.block_type = NILE_SYSCALL_TASK_BLOCK_TYPE_IO_TIMEOUT_MS;
	tsk_blk.io_op_params = &io1;
	tsk_blk.timer_val = 2000;

	uint32_t syscall_retval = NILE_SYSCALL_RETVAL_OK;
	syscall_retval = nile_syscall(NILE_SYSCALL_IO_CHAR_DEV_WRITE, &io1, NILE_SYSCALL_PARAM_CNT_IO_OP);
	syscall_retval = nile_syscall(NILE_SYSCALL_TASK_BLOCK, &tsk_blk, NILE_SYSCALL_PARAM_CNT_TASK_BLOCK);


	volatile float temp2 = 100.0f;
	while (1) {
		temp2 *= temp;
		if (((*(uint32_t*) &temp2 & 0x7FFFFFFFu) == 0x7F800000u)) {
			temp2 = 100.0f;
		}
	}
}
