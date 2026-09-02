#ifndef NILE_EXTERNAL_CACHE_H_
#define NILE_EXTERNAL_CACHE_H_

#include "nile/compiler.h"
#include "mcu.h"

static NILE_FORCEINLINE void nile_external_cache_disable(void);
static NILE_FORCEINLINE void nile_external_cache_enable(void);
static NILE_FORCEINLINE void nile_external_cache_reset(void);
static NILE_FORCEINLINE void nile_external_cache_prefetch_enable(void);
static NILE_FORCEINLINE void nile_external_cache_prefetch_disable(void);

#if defined(NILE_MCU_STM32F746)
#include "nile/mcu/stm32f746/external_cache_art.h"
#endif


#endif /* NILE_EXTERNAL_CACHE_H_ */
