#ifndef NILE_MCU_STM32F746_EXTERNAL_CACHE_ART_H_
#define NILE_MCU_STM32F746_EXTERNAL_CACHE_ART_H_

#include "nile/stdtypes.h"
#include "nile/compiler.h"
#include "nile/barriers.h"
#include "nile/memlayout.h"

static NILE_FORCEINLINE void nile_external_cache_disable(void){
	FLASH->ACR &= ~FLASH_ACR_ARTEN;
}

static NILE_FORCEINLINE void nile_external_cache_enable(void){
	FLASH->ACR |= FLASH_ACR_ARTEN;
}

static NILE_FORCEINLINE void nile_external_cache_prefetch_enable(void){
	FLASH->ACR |= FLASH_ACR_PRFTEN;
}

static NILE_FORCEINLINE void nile_external_cache_prefetch_disable(void){
	FLASH->ACR &= ~FLASH_ACR_PRFTEN;
}

static NILE_FORCEINLINE void nile_external_cache_reset(void)
{
    nile_external_cache_disable();
    FLASH->ACR |= FLASH_ACR_ARTRST;
    nile_dsb();
    nile_isb();
    FLASH->ACR &= ~FLASH_ACR_ARTRST;
    nile_external_cache_enable();
    nile_dsb();
    nile_isb();
}

#endif /* NILE_MCU_STM32F746_EXTERNAL_CACHE_ART_H_ */
