#include "nile/compiler.h"
#include "nile/stdtypes.h"
#include "nile/syscall_api.h"
#include "nile/kernel.h"
#include "nile/kernel/io_char_dev.h"
#include "nile/cache.h"

#include "app/io_dev_id.h"

NILE_USED void task0(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
	(void) arg0;
	(void) arg1;
	(void) arg2;
	(void) arg3;
	union {
		uint32_t data_buffer_align;
		uint8_t data_buffer[8];
	} buf1;
	union {
		uint32_t data_buffer_align;
		uint8_t data_buffer[8];
	} buf2;
	union {
		uint32_t data_buffer_align;
		uint8_t data_buffer[8];
	} buf3;

	nile_syscall_params_io_op io1 = { 0 };
	io1.dev_id = CHAR_DEV_ID_PIPE0;
	io1.data_buffer = &buf1.data_buffer_align;
	io1.data_len = 8;
	io1.op_flags = 0;

	nile_syscall_params_io_op io2 = { 0 };
	io2.dev_id = CHAR_DEV_ID_PIPE0;
	io2.data_buffer = &buf2.data_buffer_align;
	io2.data_len = 8;
	io2.op_flags = 0;

	uint32_t data_len = 8;
	buf1.data_buffer[0] = 'w';
	buf1.data_buffer[1] = 'o';
	buf1.data_buffer[2] = 'o';
	buf1.data_buffer[3] = 'f';
	buf1.data_buffer[4] = 0x11;
	buf1.data_buffer[5] = 0x12;
	buf1.data_buffer[6] = 0x13;
	buf1.data_buffer[7] = 0x14;


	nile_syscall_params_io_op io3 = { 0 };

	nile_syscall_params_task_block tsk_blk = { 0 };
	tsk_blk.block_type = NILE_SYSCALL_TASK_BLOCK_TYPE_DELAY_MS;
	tsk_blk.io_op_params = &io1;
	tsk_blk.timer_val = 3000;
	nile_kernel *kk = (nile_kernel*) NILE_MEMORY_KERNEL_ADDR;
	uint8_t *status_mem_addr = &kk->status.kernel_log.mem[0];

	uint32_t syscall_retval = NILE_SYSCALL_RETVAL_OK;
	syscall_retval = nile_syscall(NILE_SYSCALL_TASK_BLOCK, &tsk_blk, NILE_SYSCALL_PARAM_CNT_TASK_BLOCK);

//	syscall_retval = nile_syscall(NILE_SYSCALL_IO_CHAR_DEV_OPEN, &io1, NILE_SYSCALL_PARAM_CNT_IO_OP);
//
//	syscall_retval = nile_syscall(NILE_SYSCALL_IO_CHAR_DEV_READ, &io2, NILE_SYSCALL_PARAM_CNT_IO_OP);
//
	syscall_retval = nile_syscall(NILE_SYSCALL_IO_CHAR_DEV_READ, &io1, NILE_SYSCALL_PARAM_CNT_IO_OP);
//
//
//	while(io3.retval_io_hw_op_finished_code == 0); //spin on io3
//
//	syscall_retval = nile_syscall(NILE_SYSCALL_IO_CHAR_DEV_WRITE, &io1, NILE_SYSCALL_PARAM_CNT_IO_OP);
//	volatile uint32_t readout[65];
//	for (int i = 0; i < 65; i++) {
//		readout[i] = *(uint32_t*) (0x90000000UL + 4 * i);
//	}
//
//	io3.dev_id = BLOCK_DEV_ID_QUADSPI;
//	io3.data_buffer = &buf3.data_buffer_align;
//	io3.data_len = 4096;
//	io3.op_flags = 0;
//	io3.block_dev_memaddr = 0;
//	syscall_retval = nile_syscall(NILE_SYSCALL_IO_BLOCK_DEV_OPEN, &io3, NILE_SYSCALL_PARAM_CNT_IO_OP);
//
//	syscall_retval = nile_syscall(NILE_SYSCALL_IO_BLOCK_DEV_ERASE, &io3, NILE_SYSCALL_PARAM_CNT_IO_OP);
//	syscall_retval = nile_syscall(NILE_SYSCALL_TASK_BLOCK, &tsk_blk, NILE_SYSCALL_PARAM_CNT_TASK_BLOCK);
//
//	for (int i = 0; i < 65; i++) {
//		readout[i] = *(uint32_t*) (0x90000000UL + 4 * i);
//	}
////
//	io3.data_buffer = readout;
//	io3.data_len = 256;
//	io3.block_dev_memaddr = 256;
//	for (int i = 0; i < 65; i++) {
//		readout[i] = i;
//	}
//	//readout[0] = 0x01234567;
//	//readout[63] = 0xFEEDFACE;
//	io3.retval_io_hw_op_finished_code = 0;
//	tsk_blk.retval_block_release_src = 0;
//	syscall_retval = nile_syscall(NILE_SYSCALL_IO_BLOCK_DEV_WRITE, &io3, NILE_SYSCALL_PARAM_CNT_IO_OP);
//	tsk_blk.block_type = NILE_SYSCALL_TASK_BLOCK_TYPE_IO_TIMEOUT_MS;
//	syscall_retval = nile_syscall(NILE_SYSCALL_TASK_BLOCK, &tsk_blk, NILE_SYSCALL_PARAM_CNT_TASK_BLOCK);
//
//	for (int i = 0; i < 65; i++) {
//		readout[i] = 12345678;
//	}
//
//	//readout[0] = *(uint32_t*) (0x90000000UL);
//
//	for (int i = 0; i < 65; i+=2) {
//		readout[i] = *(uint32_t*) (0x90000000UL + 4 * i + 256);
//		readout[i+1] = *(uint32_t*) (0x90000000UL + 4 * i + 4 + 256);
//	}

//	for (int i = 0; i < 65; i++) {
//		((uint8_t*) &readout[i])[0] = *(uint8_t*) (0x90000000UL + 4 * i + 0);
//		((uint8_t*) &readout[i])[1] = *(uint8_t*) (0x90000000UL + 4 * i + 1);
//		((uint8_t*) &readout[i])[2] = *(uint8_t*) (0x90000000UL + 4 * i + 2);
//		((uint8_t*) &readout[i])[3] = *(uint8_t*) (0x90000000UL + 4 * i + 3);
//
//	}

	while (1) {
	}
}
