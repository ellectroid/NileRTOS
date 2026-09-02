#ifndef NILE_SYSCALL_H_
#define NILE_SYSCALL_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/arch.h"

/* ----------------------------- */
/* Public portable API           */
/* ----------------------------- */

enum NILE_SYSCALL_ID {
	NILE_SYSCALL_NONE = 0,
	/* Kernel Info/Debug */
	NILE_SYSCALL_KERNEL_INFO_VERSION, //version major, minor, patch, kernel size
	NILE_SYSCALL_KERNEL_INFO_TIMING,  //tick duration (in us), tick prescaler, tick source freq
	NILE_SYSCALL_KERNEL_LOG,          //read/write kernel log
	NILE_SYSCALL_KERNEL_RESERVED0,
	NILE_SYSCALL_KERNEL_RESERVED1,
	NILE_SYSCALL_KERNEL_RESERVED2,
	NILE_SYSCALL_KERNEL_RESERVED3,

	/* Task Management */
	NILE_SYSCALL_TASK_YIELD,
	NILE_SYSCALL_TASK_BLOCK,
	NILE_SYSCALL_TASK_RESERVED0,
	NILE_SYSCALL_TASK_RESERVED1,
	NILE_SYSCALL_TASK_RESERVED2,
	NILE_SYSCALL_TASK_RESERVED3,
	NILE_SYSCALL_TASK_RESERVED4,
	NILE_SYSCALL_TASK_RESERVED5,

	/* IO */
	NILE_SYSCALL_IO_CHAR_DEV_IOCTL, //custom device-specific commands
	NILE_SYSCALL_IO_CHAR_DEV_OPEN,
	NILE_SYSCALL_IO_CHAR_DEV_CLOSE,
	NILE_SYSCALL_IO_CHAR_DEV_READ,
	NILE_SYSCALL_IO_CHAR_DEV_WRITE,

	NILE_SYSCALL_IO_BLOCK_DEV_IOCTL, //custom device-specific commands
	NILE_SYSCALL_IO_BLOCK_DEV_OPEN,
	NILE_SYSCALL_IO_BLOCK_DEV_CLOSE,
	NILE_SYSCALL_IO_BLOCK_DEV_READ,
	NILE_SYSCALL_IO_BLOCK_DEV_WRITE,
	NILE_SYSCALL_IO_BLOCK_DEV_ERASE,
	NILE_SYSCALL_IO_RESERVED0,
	NILE_SYSCALL_IO_RESERVED1,
	NILE_SYSCALL_IO_RESERVED2,
	NILE_SYSCALL_IO_RESERVED3,
	NILE_SYSCALL_IO_RESERVED4,
};

enum {
	NILE_SYSCALL_IOCTL_NONE = 0,

	NILE_SYSCALL_IOCTL_CHAR_DEV_CAPABILITY = 0x0010, //All fixed properties of device flags
	NILE_SYSCALL_IOCTL_CHAR_DEV_STATUS,  //all dynamic properties of device flags
	NILE_SYSCALL_IOCTL_CHAR_DEV_RESET_DEVICE,
	NILE_SYSCALL_IOCTL_CHAR_DEV_GET_HW_ERRCODE,

	NILE_SYSCALL_IOCTL_BLOCK_DEV_CAPABILITY = 0x0040, //All fixed properties of device flags, plus op granularity in output
	NILE_SYSCALL_IOCTL_BLOCK_DEV_STATUS,  //all dynamic properties of device flags
	NILE_SYSCALL_IOCTL_BLOCK_DEV_RESET_DEVICE,
	NILE_SYSCALL_IOCTL_BLOCK_DEV_GET_HW_ERRCODE,

};

typedef struct {
	uint32_t dev_id;
	union {
		uint32_t *data_buffer;                           //IO for hw/ISR
		uint32_t *ioctl_out_buf;                         //IO for IOCTL request
	};
	union {
		uint32_t data_len;
		uint32_t ioctl_out_buf_len;
	};
	union {
		volatile uint32_t op_flags;
		volatile uint32_t io_ctl_cmd;
	};
	uint32_t block_dev_memaddr;
	union {
		uint32_t *volatile retval_io_op_ptr;                 //filled by IO syscall
		uint32_t ioctl_arg0;
	};
	union {
		void *volatile retval_io_op_q_ptr;                   //filled by IO syscall
		uint32_t ioctl_arg1;
	};
	union {
		void *volatile retval_io_dev;                        //filled by IO syscall
		uint32_t ioctl_arg2;
	};
	union {
		volatile uint32_t retval_io_hw_op_finished_code;     //filled by hw/ISR
		uint32_t ioctl_arg3;
	};
} nile_syscall_params_io_op;

typedef struct {
	uint32_t block_type;
	nile_syscall_params_io_op *io_op_params;
	uint32_t timer_val;
	volatile uint32_t retval_block_release_src;
} nile_syscall_params_task_block;

enum {
	NILE_SYSCALL_PARAM_CNT_IO_OP = 9,
	NILE_SYSCALL_PARAM_CNT_TASK_BLOCK = 4,
};

enum NILE_SYSCALL_RETVAL {
	NILE_SYSCALL_RETVAL_OK,
	NILE_SYSCALL_RETVAL_ERR_NOT_IMPLEMENTED,
	NILE_SYSCALL_RETVAL_ERR_INVALID_HANDLER,
	NILE_SYSCALL_RETVAL_ERR_TOO_FEW_ARGUMENTS,
	NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_STACK_SPACE,
	NILE_SYSCALL_RETVAL_ERR_PTR_OUTSIDE_TASK_RW_SPACE,
	NILE_SYSCALL_RETVAL_ERR_BAD_TIMEOUT_VAL,
	NILE_SYSCALL_RETVAL_ERR_BAD_TASK_BLOCK_TYPE,
	NILE_SYSCALL_RETVAL_ERR_DEVICE_DOES_NOT_EXIST,
	NILE_SYSCALL_RETVAL_ERR_DEVICE_BUSY,
	NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_OPEN,
	NILE_SYSCALL_RETVAL_ERR_DEVICE_NOT_READY,
	NILE_SYSCALL_RETVAL_ERR_DEVICE_MODE_NOT_SUPPORTED,
	NILE_SYSCALL_RETVAL_ERR_DEVICE_OPERATION_LENGTH_0,
	NILE_SYSCALL_RETVAL_ERR_DATA_BUFFER_TOO_SMALL,
	NILE_SYSCALL_RETVAL_ERR_BAD_ALIGNMENT,
	NILE_SYSCALL_RETVAL_ERR_DEVICE_OP_QUEUE_FULL,
	NILE_SYSCALL_RETVAL_ERR_BAD_DATA_LEN,
};

enum NILE_SYSCALL_TASK_BLOCK {
	NILE_SYSCALL_TASK_UNBLOCK_SRC_IO = 1,
	NILE_SYSCALL_TASK_UNBLOCK_SRC_TIMEOUT = 2,

	NILE_SYSCALL_TASK_BLOCK_TYPE_IO = 0,
	NILE_SYSCALL_TASK_BLOCK_TYPE_IO_TIMEOUT_US,
	NILE_SYSCALL_TASK_BLOCK_TYPE_IO_TIMEOUT_MS,
	NILE_SYSCALL_TASK_BLOCK_TYPE_DELAY_US,
	NILE_SYSCALL_TASK_BLOCK_TYPE_DELAY_MS,
	NILE_SYSCALL_TASK_BLOCK_TYPE_RESERVED,

};

static NILE_NAKED NILE_USED uint32_t nile_syscall(uint32_t syscall_id, void *uint32_t_arg_list, uint32_t arg_cnt);

/* ----------------------------- */
/* Architecture-specific impl    */
/* ----------------------------- */

#if NILE_CPU_ARCH_ARMV7M
#include "nile/arch/armv7m/syscall.h"
#elif NILE_CPU_ARCH_RISCV
#include "nile/arch/riscv/syscall.h"
#else
#error "No syscall implementation for this architecture"
#endif

#endif /* NILE_SYSCALL_H_ */
