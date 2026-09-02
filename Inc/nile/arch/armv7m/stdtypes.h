#ifndef NILE_ARCH_ARMV7M_STDTYPES_H_
#define NILE_ARCH_ARMV7M_STDTYPES_H_

typedef unsigned char      uint8_t;
typedef signed   char      int8_t;

typedef unsigned short     uint16_t;
typedef signed   short     int16_t;

typedef unsigned int       uint32_t;
typedef signed   int       int32_t;

typedef unsigned long long uint64_t;
typedef signed   long long int64_t;


typedef unsigned int       size_t;
typedef int                ptrdiff_t;

typedef unsigned int       uintptr_t;
typedef signed   int       intptr_t;


typedef unsigned char      bool;
#define true  1
#define false 0


#define NULL    ((void*)0)
#define NULLPTR ((void*)0)

/* Compile-time size checks */
#define NILE_STATIC_ASSERT(cond, msg) \
    typedef char static_assert_##msg[(cond) ? 1 : -1]

NILE_STATIC_ASSERT(sizeof(uint8_t)  == 1, uint8_t_wrong_size);
NILE_STATIC_ASSERT(sizeof(uint16_t) == 2, uint16_t_wrong_size);
NILE_STATIC_ASSERT(sizeof(uint32_t) == 4, uint32_t_wrong_size);
NILE_STATIC_ASSERT(sizeof(uint64_t) == 8, uint64_t_wrong_size);

#endif
