#ifndef NILE_KERNEL_STATUS_H_
#define NILE_KERNEL_STATUS_H_

#include "nile/stdtypes.h"
#include "nile/kernel_config.h"

#define NILE_KERNEL_LOG_ADDITIONAL_FLAGS_LOG_READ              0x01U
#define NILE_KERNEL_LOG_ADDITIONAL_FLAGS_DESTRUCTIVE_READ_MODE 0x02U

#define NILE_KERNEL_LOG_CONTROL_LENGTH_MASK                    0xFFFFU
#define NILE_KERNEL_LOG_CONTROL_ERROR_MASK                     (1U << 30)
#define NILE_KERNEL_LOG_CONTROL_FINISHED                       (1U << 31)

//for generic use when something goes wrong in the kernel, can write messages or error codes or whatever
//just a piece of memory to use in arbitrary way in case of OS error
typedef struct nile_kernel_system_status{
	volatile uint32_t SR1; //status register
	volatile uint32_t SR2; //status register
	uint16_t kernel_log_write_cursor;
	uint16_t kernel_log_read_cursor;
	uint16_t kernel_log_level;
	uint16_t kernel_log_len;
	union {
		volatile uint32_t mem32[NILE_SYSTEM_STATUS_LOG_MEM_SIZE/4 + 1];
		volatile uint8_t mem[NILE_SYSTEM_STATUS_LOG_MEM_SIZE + 4];
	}kernel_log;
}nile_kernel_system_status;


#endif /* NILE_KERNEL_STATUS_H_ */
