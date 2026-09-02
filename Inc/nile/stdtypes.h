#ifndef NILE_STDTYPES_H_
#define NILE_STDTYPES_H_

#include "arch.h"

#if (NILE_CPU_ARCH_ARMV7M)
#include "nile/arch/armv7m/stdtypes.h"
#else
#error "Unsupported architecture: no stdtypes defined"
#endif

#endif
