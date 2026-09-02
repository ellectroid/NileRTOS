#ifndef NILE_MEMLAYOUT_H_
#define NILE_MEMLAYOUT_H_
#include "nile/arch.h"
#include "nile/mcu.h"

#ifndef NILE_REQUIRED
#define NILE_REQUIRED 0x87654321U
#endif

/* Sentinel meaning "must be overridden by MCU-specific backend" */
#define NILE_MEMORY_KERNEL_ADDR           NILE_REQUIRED
#define  NILE_MEMORY_KERNEL_STACK_START   NILE_REQUIRED

#include "nile/stdtypes.h"
#if defined (NILE_MCU_STM32F746)
#include "nile/mcu/stm32f746/memlayout.h" //includes arch-specific
#endif

/* ========== Hardware memory address hooks ========== */

//dedicated 64-bit bus to the core
#define NILE_MCU_SRAM_DTCM RAMDTCM_BASE
#define NILE_MCU_SRAM_DTCM_LENGTH (0x00010000)

//share a single SRAM 32-bit bus to the core
#define NILE_MCU_SRAM1  SRAM1_BASE
#define NILE_MCU_SRAM2  SRAM2_BASE
#define NILE_MCU_SRAM1_LENGTH  (0x0003C000)
#define NILE_MCU_SRAM2_LENGTH  (0x00004000)

#define NILE_MCU_FLASH_AXI    FLASHAXI_BASE
#define NILE_MCU_INTERNAL_FLASH_ITCM  FLASHITCM_BASE
#define NILE_MCU_FLASH_LEN   (1 << 20)

#define NILE_MCU_RAM_START (0x20000000UL)
#define NILE_MCU_RAM_LEN   (0x00050000UL)

//Memory-mapped devices
#define NILE_QUADSPI_MEMMAPPED_ADDR   (0x90000000UL)

/* ========== Kernel memory config ========== */

#undef  NILE_MEMORY_KERNEL_ADDR
#define NILE_MEMORY_KERNEL_ADDR          NILE_MCU_SRAM2
#undef  NILE_MEMORY_KERNEL_STACK_START
#define NILE_MEMORY_KERNEL_STACK_START   (NILE_MCU_SRAM2 + NILE_MCU_SRAM2_LENGTH)

/* Additional */
//#define MCU_TRNG_HARDWARE				(RNG)
//#define MCU_QUADSPI_INTERFACE			(QUADSPI)
//#define MCU_QUADSPI_MEMMAP_ADDR			(0x90000000)


/* Validation: ensure backend overrode the sentinel */
#if NILE_MEMORY_KERNEL_ADDR == NILE_REQUIRED
#error "NILE_MEMORY_KERNEL_ADDR must be defined by MCU-specific config"
#endif
#if NILE_MEMORY_KERNEL_STACK_START == NILE_REQUIRED
#error "NILE_MEMORY_KERNEL_STACK_START must be defined by MCU-specific config"
#endif

#undef NILE_REQUIRED

#endif /* NILE_MEMLAYOUT_H_ */
