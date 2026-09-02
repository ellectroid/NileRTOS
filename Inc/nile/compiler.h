#ifndef NILE_COMPILER_H_
#define NILE_COMPILER_H_

#if defined(__GNUC__) && !defined(__clang__)
#define NILE_COMPILER_GCC 1
#endif

#if defined(__clang__)
#define NILE_COMPILER_CLANG 1
#endif

#if defined(_MSC_VER)
#define NILE_COMPILER_MSVC 1
#endif

/* Sentinel meaning "must be overridden by compiler-specific backend" */
#define NILE_REQUIRED 0x87654321U

/* Public compiler-facing macros, initially set to sentinel */
#define NILE_INLINE        NILE_REQUIRED
#define NILE_FORCEINLINE   NILE_REQUIRED
#define NILE_NOINLINE      NILE_REQUIRED

#define NILE_NORETURN      NILE_REQUIRED
#define NILE_NAKED         NILE_REQUIRED
#define NILE_WEAK          NILE_REQUIRED
#define NILE_USED          NILE_REQUIRED

#define NILE_VAR_USED      NILE_REQUIRED
#define NILE_UNUSED        NILE_REQUIRED

#define NILE_LIKELY(x)     NILE_REQUIRED
#define NILE_UNLIKELY(x)   NILE_REQUIRED

#define NILE_PACKED        NILE_REQUIRED
#define NILE_ALIGN(x)      NILE_REQUIRED

#define NILE_FALLTHROUGH   NILE_REQUIRED

/* Include compiler backend */
#if NILE_COMPILER_GCC
#include "nile/compiler/gcc.h"
#endif

#if NILE_COMPILER_CLANG
#include "nile/compiler/clang.h"
#endif

#if NILE_COMPILER_MSVC
#include "nile/compiler/msvc.h"
#endif

#undef NILE_REQUIRED

#endif
