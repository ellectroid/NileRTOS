#ifndef NILE_COMPILER_GCC_H_
#define NILE_COMPILER_GCC_H_

/* Remove sentinel definitions from public compiler.h */
#undef NILE_INLINE
#undef NILE_FORCEINLINE
#undef NILE_NOINLINE

#undef NILE_NORETURN
#undef NILE_NAKED
#undef NILE_WEAK
#undef NILE_USED

#undef NILE_VAR_USED
#undef NILE_UNUSED

#undef NILE_LIKELY
#undef NILE_UNLIKELY

#undef NILE_PACKED
#undef NILE_ALIGN

#undef NILE_FALLTHROUGH

/* GCC-specific implementations */

#define NILE_INLINE        inline
#define NILE_FORCEINLINE   inline __attribute__((always_inline))
#define NILE_NOINLINE      __attribute__((noinline))

#define NILE_NORETURN      __attribute__((noreturn))
#define NILE_NAKED         __attribute__((naked))
#define NILE_WEAK          __attribute__((weak))
#define NILE_USED          __attribute__((used))

#define NILE_VAR_USED      __attribute__((used))
#define NILE_UNUSED        __attribute__((unused))

#define NILE_LIKELY(x)     __builtin_expect(!!(x), 1)
#define NILE_UNLIKELY(x)   __builtin_expect(!!(x), 0)

#define NILE_PACKED        __attribute__((packed))
#define NILE_ALIGN(x)      __attribute__((aligned(x)))

#if defined(__GNUC__) && __GNUC__ >= 7
#define NILE_FALLTHROUGH   __attribute__((fallthrough))
#else
#define NILE_FALLTHROUGH   /* fallthrough */
#endif

#endif /* NILE_COMPILER_GCC_H_ */
