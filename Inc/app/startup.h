#ifndef APP_STARTUP_H_
#define APP_STARTUP_H_

#include "nile/stdtypes.h"
#include "nile/kernel.h"
#include "nile/kernel/tcb_queue.h"
#include "nile/ctx_init.h"
#include "app/tasks.h"

#define TASK0_STACKSPACE_SIZE  (1024U)
#define TASK0_STACKSPACE_BASE  (NILE_MCU_SRAM1 + TASK0_STACKSPACE_SIZE)

#define TASK1_STACKSPACE_SIZE  (512U)
#define TASK1_STACKSPACE_BASE  (TASK0_STACKSPACE_BASE + TASK1_STACKSPACE_SIZE)

#define TASK2_STACKSPACE_SIZE  (512U)
#define TASK2_STACKSPACE_BASE  (TASK1_STACKSPACE_BASE + TASK2_STACKSPACE_SIZE)

#define TASK3_STACKSPACE_SIZE  (512U)
#define TASK3_STACKSPACE_BASE  (TASK2_STACKSPACE_BASE + TASK3_STACKSPACE_SIZE)

uint32_t task0_init();
uint32_t task1_init();
uint32_t task2_init();
uint32_t task3_init();

#endif /* APP_STARTUP_TASKS_H_ */
