#ifndef SRC_ALLOCATOR_TLSFA_H_
#define SRC_ALLOCATOR_TLSFA_H_

//========== USER CONFIGURATION ==========
//Pick a type for header
#define TLSF_ALLOC_MEMSIZE_TYPE_UL
//#define TLSF_ALLOC_MEMSIZE_TYPE_ULL

//Set its sizeof accordingly
#define TLSF_ALLOC_MEMSIZE_T_SIZE  4
//#define TLSF_ALLOC_MEMSIZE_T_SIZE  8

//Binning config
#define TLSF_ALLOC_CLASS_COUNT  7
#define TLSF_ALLOC_CLASS_BITSHIFT  4
#define TLSF_ALLOC_SUBCLASS_BITWIDTH  2

#define TLSF_ALLOC_USE_COMPILER_INTRINSIC_TRAILING_ZEROES  1
#define TLSF_ALLOC_USE_COMPILER_INTRINSIC_HIGHEST_BIT  1

//Printing memory
#define TLSF_ALLOC_ENABLE_DEBUG_MODE  0

//========== END OF USER CONFIGURATION ===

//========== Configuration verification ==
#if (TLSF_ALLOC_CLASS_COUNT > 16)
#error "TLSFPC Allocator: unsupported size class count"
#endif
#if (TLSF_ALLOC_SUBCLASS_COUNT > 16)
#error "TLSFPC Allocator: unsupported size subclass count"
#endif
#if (TLSF_ALLOC_SUBCLASS_BITWIDTH >= TLSF_ALLOC_CLASS_BITSHIFT)
#error "TLSFPC Allocator: bad class/subclass bit shift"
#endif
#if (TLSF_ALLOC_MEMSIZE_T_SIZE != 4) && (TLSF_ALLOC_MEMSIZE_T_SIZE != 8)
#error "TLSFPC Allocator: bad memsize_t size configuration"
#endif

//========== Internal configuration ======
//config memsize_t type
#if defined(TLSF_ALLOC_MEMSIZE_TYPE_UL)
typedef unsigned long memsize_t;
#elif defined(TLSF_ALLOC_MEMSIZE_TYPE_ULL)
typedef unsigned long long memsize_t;
#else
#error "TLSFPC Allocator: invalid configuration for memsize_t type"
#endif

#define TLSF_ALLOC_SUBCLASS_COUNT  (1 << TLSF_ALLOC_SUBCLASS_BITWIDTH)
#define TLSF_ALLOC_SUBCLASS_BITSHIFT (TLSF_ALLOC_CLASS_BITSHIFT - TLSF_ALLOC_SUBCLASS_BITWIDTH)

//Config block header
#define TLSF_ALLOC_BLOCK_HEADER_SIZE                  TLSF_ALLOC_MEMSIZE_T_SIZE
#define TLSF_ALLOC_BLOCK_HEADER_USED_FLAG             ~((~(memsize_t)0 << 1) >> 1)
#define TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_SHIFT       0
#if(TLSF_ALLOC_MEMSIZE_T_SIZE == 4)
#define TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_SHIFT       16
#define TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_MASK        (((memsize_t)0x7FFF) << TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_SHIFT)
#define TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK        (((memsize_t)0x7FFF) << TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_SHIFT)
#else
#define TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_SHIFT       32
#define TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_MASK        (((memsize_t)0x7FFFFFFF) << TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_SHIFT)
#define TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK        (((memsize_t)0x7FFFFFFF) << TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_SHIFT)
#endif
#define TLSF_ALLOC_BLOCK_HEADER_HAS_PADDING_FLAG      ((memsize_t)1 << (TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_SHIFT - 1))

//Config padding header/footer
#define TLSF_ALLOC_PADDING_HEADER_SIZE                TLSF_ALLOC_MEMSIZE_T_SIZE
#define TLSF_ALLOC_PADDING_HEADER_PADDING_FLAG_MASK   TLSF_ALLOC_BLOCK_HEADER_HAS_PADDING_FLAG
#define TLSF_ALLOC_PADDING_HEADER_PADDING_SIZE_MASK   TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK

//Config free block links
#define TLSF_ALLOC_FREE_BLOCK_LINK_SIZE               TLSF_ALLOC_MEMSIZE_T_SIZE
#define TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_SHIFT  0
#if(TLSF_ALLOC_MEMSIZE_T_SIZE == 4)
#define TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_MASK   (((memsize_t)0x7FFF) << TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_SHIFT)
#define TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_SHIFT  16
#define TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_MASK   (((memsize_t)0x7FFF) << TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_SHIFT)
#else
#define TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_MASK   (((memsize_t)0x7FFFFFFF) << TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_SHIFT)
#define TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_SHIFT  32
#define TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_MASK   (((memsize_t)0x7FFFFFFF) << TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_SHIFT)
#endif
#define TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_NEG    ((memsize_t)1 << (TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_SHIFT - 1))
#define TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_NEG    ~((~(memsize_t)0 << 1) >> 1)

#define TLSF_ALLOC_FREE_BLOCK_OFFSET_VALID_FLAG       TLSF_ALLOC_BLOCK_HEADER_USED_FLAG

#if (((1 << TLSF_ALLOC_CLASS_BITSHIFT) + TLSF_ALLOC_MEMSIZE_T_SIZE) > 2*TLSF_ALLOC_MEMSIZE_T_SIZE)
#define TLSF_ALLOC_MIN_BLOCK_SIZE ((1 << TLSF_ALLOC_CLASS_BITSHIFT) + TLSF_ALLOC_MEMSIZE_T_SIZE)
#else
#define TLSF_ALLOC_MIN_BLOCK_SIZE (2*TLSF_ALLOC_MEMSIZE_T_SIZE)
#endif

#define TLSF_ALLOC_MIN_USER_DATA_SIZE (TLSF_ALLOC_MIN_BLOCK_SIZE - TLSF_ALLOC_MEMSIZE_T_SIZE)
#define TLSF_ALLOC_MAX_BLOCK_SIZE  (TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK & ~(TLSF_ALLOC_MEMSIZE_T_SIZE - 1))

typedef unsigned short l1_bitmap_t;
#if (TLSF_ALLOC_SUBCLASS_COUNT < 8)
typedef unsigned char l2_bitmap_t;
#else
typedef unsigned short l2_bitmap_t;
#endif
typedef struct free_block_header_t {
	memsize_t header;
	memsize_t free_block_link;
}free_block_header_t;
typedef struct{
	unsigned char* managed_memory;
	memsize_t managed_memory_bytelen;
	l1_bitmap_t l1_bitmap;
	l2_bitmap_t l2_bitmap[TLSF_ALLOC_CLASS_COUNT];
	//free_block_header_t* free_block[TLSF_ALLOC_CLASS_COUNT][TLSF_ALLOC_SUBCLASS_COUNT];
	memsize_t free_block[TLSF_ALLOC_CLASS_COUNT][TLSF_ALLOC_SUBCLASS_COUNT];
}alloc_header_t;



void tlsf_allocator_init(alloc_header_t *alloc_header, unsigned char *memory,
		memsize_t memory_bytelen);
void* tlsf_alloc(alloc_header_t *alloc_header, memsize_t size, memsize_t alignment);
void tlsf_free(alloc_header_t *alloc_header, void *obj);

#if (TLSF_ALLOC_ENABLE_DEBUG_MODE == 1)
void tlsf_print_bins(alloc_header_t *alloc_header);
void tlsf_print_memory(alloc_header_t *alloc_header);
#endif

#endif //end of header
