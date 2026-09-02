#ifndef NILE_KERNEL_CONFIG_H_
#define NILE_KERNEL_CONFIG_H_

#include "mcu.h"
#include "arch.h"

#ifndef NILE_REQUIRED
#define NILE_REQUIRED 0x87654321U
#endif

/* Sentinel meaning "must be overridden by MCU-specific backend" */
#define MCU_INIT_CPU_FREQUENCY NILE_REQUIRED

#if defined (NILE_MCU_STM32F746)
#include "nile/mcu/stm32f746/kernel_config.h"
#endif

/* Validation: ensure backend overrode the sentinel */
#if MCU_INIT_CPU_FREQUENCY == NILE_REQUIRED
#error "NILE_MEMORY_KERNEL_ADDR must be defined by MCU-specific config"
#endif

#undef NILE_REQUIRED

#define NILE_KERNEL_INFO_VERSION_MAJOR	(1)
#define NILE_KERNEL_INFO_VERSION_MINOR	(2)
#define NILE_KERNEL_INFO_VERSION_PATCH	(1)


#define NILE_MEMORY_KHEAP_BYTESIZE       		    2048
#define NILE_SYSTEM_STATUS_LOG_MEM_SIZE  	        64
#define NILE_KERNEL_INFO_INFOTEXT_SIZE   		    16
#define NILE_SYSCALL_VECTOR_COUNT        		    32
#define NILE_VTABLE_VECTOR_COUNT         		    4
#define NILE_HW_FPU_UNPRIV                          1
#define NILE_HW_FPU_PRIV                            1

//Task config
#define NILE_KERNEL_TASK_MEMPROT_REGION_COUNT       6
#define NILE_KERNEL_TASK_MEMPROT_REGION_PRECEDENCE_ASCENDING   (1)
#define NILE_KERNEL_TASK_MEMPROT_REGION_PRECEDENCE_DESCENDING  (!NILE_KERNEL_TASK_MEMPROT_REGION_PRECEDENCE_ASCENDING)

//IO device: char
#define NILE_KERNEL_IO_CHAR_DEV_COUNT               6
#define NILE_KERNEL_IO_CHAR_DEV_OP_QUEUE_CAPACITY   4

//IO device: block
#define NILE_KERNEL_IO_BLOCK_DEV_COUNT              1
#define NILE_KERNEL_IO_BLOCK_DEV_OP_QUEUE_CAPACITY  4

//Scheduler config
#define NILE_SCHEDULER_OS_TICK_OVERFLOW_LEVEL       0x0FFFFFFFU

#endif /* NILE_KERNEL_CONFIG_H_ */
