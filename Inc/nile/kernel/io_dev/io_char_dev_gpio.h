#ifndef NILE_KERNEL_IO_DEV_IO_CHAR_DEV_GPIO_H_
#define NILE_KERNEL_IO_DEV_IO_CHAR_DEV_GPIO_H_

#include "nile/stdtypes.h"

typedef struct{
	uint64_t ioctl_cmd;
	uint64_t op_mask;
	uint64_t val; //also IO input/output
}nile_syscall_params_io_ioctl_gpio_1bpp; //1 bit per pin

typedef struct{
	uint64_t ioctl_cmd;
	uint64_t op_mask;
	uint64_t val[2];
}nile_syscall_params_io_ioctl_gpio_2bpp; //2 bits per pin

typedef struct{
	uint64_t ioctl_cmd;
	uint64_t op_mask;
	uint64_t val[4];
}nile_syscall_params_io_ioctl_gpio_4bpp; //4 bits per pin

typedef struct{
	uint64_t ioctl_cmd;
	uint64_t op_mask;
	uint64_t val[8];
}nile_syscall_params_io_ioctl_gpio_8bpp; //8 bits per pin

#define NILE_SYSCALL_PARAM_CNT_IO_IOCTL_GPIO_1BPP   24/4
#define NILE_SYSCALL_PARAM_CNT_IO_IOCTL_GPIO_2BPP   32/4
#define NILE_SYSCALL_PARAM_CNT_IO_IOCTL_GPIO_4BPP   48/4
#define NILE_SYSCALL_PARAM_CNT_IO_IOCTL_GPIO_8BPP   80/4


#endif /* NILE_KERNEL_IO_DEV_IO_CHAR_DEV_GPIO_H_ */
