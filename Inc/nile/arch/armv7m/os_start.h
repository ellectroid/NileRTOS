#ifndef NILE_ARCH_ARMV7M_OS_START_H_
#define NILE_ARCH_ARMV7M_OS_START_H_

#include "nile/compiler.h"
#include "nile/stdtypes.h"
#include "nile/sys_clock.h"
#include "nile/memlayout.h"

NILE_NAKED void os_start(uint32_t kernel_sp, uint32_t task_sp)
{
	(void) kernel_sp;
	(void) task_sp;
    /* Disable interrupts */
    __asm__ volatile ("CPSID I");

    /* Ensure all memory operations complete */
    __asm__ volatile ("DSB");
    __asm__ volatile ("ISB");

    /* Load PSP with task stack pointer */
    __asm__ volatile ("MSR PSP, R1");

    /* Load MSP with kernel stack pointer */
    __asm__ volatile ("MSR MSP, R0");

    /* Start SysTick counter: write ENABLE bit */
    __asm__ volatile ("MOVW R2, #0xE000E010 & 0xFFFF");
    __asm__ volatile ("MOVT R2, #0xE000E010 >> 16");
    __asm__ volatile ("LDR R3, [R2]");
    __asm__ volatile ("ORR R3, R3, #1");
    __asm__ volatile ("STR R3, [R2]");

    /* Ensure SysTick write completes */
    __asm__ volatile ("DSB");
    __asm__ volatile ("ISB");

    /* Re-enable interrupts */
    __asm__ volatile ("CPSIE I");
    __asm__ volatile ("CPSIE F");

    /* Drop to unprivileged thread mode, use PSP */
    __asm__ volatile ("MOV R4, #0x03");
    __asm__ volatile ("MSR CONTROL, R4");
    __asm__ volatile ("ISB");

    /* Idle loop */
    __asm__ volatile ("idle_loop:");
    __asm__ volatile ("WFI");
    __asm__ volatile ("B idle_loop");
}


#endif /* NILE_ARCH_ARMV7M_OS_START_H_ */
