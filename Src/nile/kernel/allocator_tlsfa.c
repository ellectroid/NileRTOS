#include "nile/kernel/allocator_tlsfa.h"

static inline memsize_t header_is_used_block(memsize_t *block_header) {
	return !!(*block_header & TLSF_ALLOC_BLOCK_HEADER_USED_FLAG);
}

static inline memsize_t header_get_block_size(memsize_t *block_header) {
	return *block_header & TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK;
}

static inline memsize_t header_get_prev_block_size(memsize_t *block_header) {
	return (*block_header & TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_MASK)
			>> TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_SHIFT;
}

static inline memsize_t header_is_padding_set(memsize_t *block_header) {
	return !!(*block_header & TLSF_ALLOC_BLOCK_HEADER_HAS_PADDING_FLAG);
}

static inline memsize_t is_padding_header(memsize_t *header_or_padding_header) {
	return (*header_or_padding_header
			& TLSF_ALLOC_PADDING_HEADER_PADDING_FLAG_MASK)
			&& !(*header_or_padding_header & TLSF_ALLOC_BLOCK_HEADER_USED_FLAG);
}

static inline memsize_t padding_get_padding_size(memsize_t *padding_header) {
	return *padding_header & TLSF_ALLOC_PADDING_HEADER_PADDING_SIZE_MASK;
}

static inline memsize_t* header_get_prev_in_mem_block(memsize_t *block_header) {
	return (memsize_t*) ((unsigned char*) block_header
			- header_get_prev_block_size(block_header));
}

static inline memsize_t* header_get_next_in_mem_block(memsize_t *block_header) {
	return (memsize_t*) ((unsigned char*) block_header
			+ header_get_block_size(block_header));
}

static memsize_t* header_get_prev_free_block(memsize_t *block_header) {
	memsize_t *link = block_header + 1;
	memsize_t offset = (*link & TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_MASK)
			>> TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_SHIFT;
	memsize_t offset_negative = *link
			& TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_NEG;
	memsize_t *prev_free_block;
	if (offset_negative)
		prev_free_block = (memsize_t*) ((unsigned char*) block_header - offset);
	else
		prev_free_block = (memsize_t*) ((unsigned char*) block_header + offset);
	if (prev_free_block == block_header)
		prev_free_block = 0;
	return prev_free_block;
}

static memsize_t* header_get_next_free_block(memsize_t *block_header) {
	memsize_t *link = block_header + 1;
	memsize_t offset = (*link & TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_MASK)
			>> TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_SHIFT;
	memsize_t offset_negative = *link
			& TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_NEG;
	memsize_t *next_free_block;
	if (offset_negative)
		next_free_block = (memsize_t*) ((unsigned char*) block_header - offset);
	else
		next_free_block = (memsize_t*) ((unsigned char*) block_header + offset);
	if (next_free_block == block_header)
		next_free_block = 0;
	return next_free_block;
}

static void header_set_prev_free_block(memsize_t *current_block,
		memsize_t *new_prev) {
	memsize_t *current_link = current_block + 1;
	memsize_t *prev_link = new_prev + 1;
	*current_link =
			*current_link & ~TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_MASK;
	if (!new_prev || current_block == new_prev)
		return;

	memsize_t offset;
	memsize_t current_offset_negative;
	memsize_t prev_offset_negative;
	if (current_block > new_prev) {
		offset = (memsize_t) ((unsigned char*) current_block
				- (unsigned char*) new_prev);
		current_offset_negative = 1;
	} else {
		offset = (memsize_t) ((unsigned char*) new_prev
				- (unsigned char*) current_block);
		current_offset_negative = 0;
	}
	prev_offset_negative = !current_offset_negative;

	*current_link =
			*current_link & ~TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_MASK;
	*current_link = *current_link
			| (offset << TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_SHIFT);
	if (current_offset_negative)
		*current_link |= TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_NEG;
	else
		*current_link &= ~TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_NEG;

	*prev_link = *prev_link & ~TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_MASK;
	*prev_link = *prev_link
			| (offset << TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_SHIFT);
	if (prev_offset_negative)
		*prev_link |= TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_NEG;
	else
		*prev_link &= ~TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_NEG;

}

static void header_set_next_free_block(memsize_t *current_block,
		memsize_t *new_next) {
	memsize_t *current_link = current_block + 1;
	memsize_t *next_link = new_next + 1;
	*current_link =
			*current_link & ~TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_MASK;
	if (!new_next || current_block == new_next)
		return;
	memsize_t offset;
	memsize_t current_offset_negative;
	memsize_t next_offset_negative;
	if (current_block > new_next) {
		offset = (memsize_t) ((unsigned char*) current_block
				- (unsigned char*) new_next);
		current_offset_negative = 1;
	} else {
		offset = (memsize_t) ((unsigned char*) new_next
				- (unsigned char*) current_block);
		current_offset_negative = 0;
	}
	next_offset_negative = !current_offset_negative;

	*current_link = *current_link
			| (offset << TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_SHIFT);
	if (current_offset_negative)
		*current_link |= TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_NEG;
	else
		*current_link &= ~TLSF_ALLOC_FREE_BLOCK_LINK_NEXT_OFFSET_NEG;

	*next_link = *next_link & ~TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_MASK;
	*next_link = *next_link
			| (offset << TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_SHIFT);
	if (next_offset_negative)
		*next_link |= TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_NEG;
	else
		*next_link &= ~TLSF_ALLOC_FREE_BLOCK_LINK_PREV_OFFSET_NEG;

}

static inline unsigned trailing_zeroes_bitmap(unsigned x) {
#if TLSF_ALLOC_USE_COMPILER_INTRINSIC_TRAILING_ZEROES
	return (unsigned) __builtin_ctz(x);
#else
    unsigned int idx = 0;
    while ((x & 1u) == 0u) {
        x >>= 1;
        idx++;
    }
    return idx;
#endif
}

static inline unsigned highest_bit_index(memsize_t x) {
#if TLSF_ALLOC_USE_COMPILER_INTRINSIC_HIGHEST_BIT
#if defined(TLSF_ALLOC_MEMSIZE_TYPE_UL)
#if TLSF_ALLOC_MEMSIZE_T_SIZE == 4
	return (unsigned) (31 - __builtin_clzl(x));
#elif TLSF_ALLOC_MEMSIZE_T_SIZE == 8
            return (unsigned)(63 - __builtin_clzl(x));
        #else
            #error "Unsupported MEMSIZE_T_SIZE for MEMSIZE_TYPE_UL"
        #endif
#elif defined(TLSF_ALLOC_MEMSIZE_TYPE_ULL)
#if TLSF_ALLOC_MEMSIZE_T_SIZE == 8
	return (unsigned) (63 - __builtin_clzll(x));
#else
    #error "Unsupported MEMSIZE_T_SIZE for MEMSIZE_TYPE_ULL"
#endif
#endif
#else
    unsigned int idx = 0;
    while (x >>= 1)
        idx++;
    return idx;
#endif
}

static inline void size_to_bitmask_index(memsize_t size, l1_bitmap_t *out_l1,
		l2_bitmap_t *out_l2, unsigned short *out_l1i, unsigned short *out_l2i) {
	memsize_t s = size >> TLSF_ALLOC_CLASS_BITSHIFT;
	unsigned class_index;
	if (s == 0) {
		class_index = 0;
	} else {
		unsigned highest = highest_bit_index(s);
		class_index = highest;
		if (class_index >= TLSF_ALLOC_CLASS_COUNT)
			class_index = TLSF_ALLOC_CLASS_COUNT - 1;
	}
	*out_l1i = (unsigned short) class_index;
	*out_l1 = (l1_bitmap_t) (1U << class_index);
	memsize_t class_size = (1U << TLSF_ALLOC_CLASS_BITSHIFT) << class_index;
	memsize_t class_base = class_size;
	memsize_t subclass_span = class_size / TLSF_ALLOC_SUBCLASS_COUNT;
	memsize_t offset_in_class = (size > class_base) ? (size - class_base) : 0;
	unsigned subclass_index = (unsigned) (offset_in_class / subclass_span);
	if (subclass_index >= TLSF_ALLOC_SUBCLASS_COUNT)
		subclass_index = TLSF_ALLOC_SUBCLASS_COUNT - 1;
	*out_l2i = (unsigned short) subclass_index;
	*out_l2 = (l2_bitmap_t) (1U << subclass_index);
}

static void free_block_get_usable_memory_padding_len(memsize_t *curr_block,
		memsize_t alignment, memsize_t *out_usable, memsize_t *out_padding) {
	memsize_t block_size = ((*curr_block
			& TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK)
			>> TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_SHIFT);
	unsigned char *block_ptr = (unsigned char*) curr_block;
	unsigned char *user_ptr = block_ptr + TLSF_ALLOC_BLOCK_HEADER_SIZE;
	if (alignment == 0) {
		*out_padding = 0;
		*out_usable = block_size - TLSF_ALLOC_BLOCK_HEADER_SIZE;
		return;
	}
	memsize_t misalign = (memsize_t) (user_ptr - (unsigned char*) 0)
			& (alignment - 1);
	memsize_t padding = (misalign == 0) ? 0 : (alignment - misalign);
	*out_padding = padding;
	*out_usable = block_size - TLSF_ALLOC_BLOCK_HEADER_SIZE - padding;
}

static void block_set_padding_header_footer(memsize_t *block_header,
		memsize_t padding_len) {
	if (padding_len == 0) {
		*block_header &= ~TLSF_ALLOC_PADDING_HEADER_PADDING_FLAG_MASK;
		return;
	}
	*block_header |= TLSF_ALLOC_PADDING_HEADER_PADDING_FLAG_MASK;
	memsize_t padding_header_value = (padding_len
			& TLSF_ALLOC_PADDING_HEADER_PADDING_SIZE_MASK) |
	TLSF_ALLOC_PADDING_HEADER_PADDING_FLAG_MASK;
	memsize_t *padding_header_ptr = block_header + 1;
	*padding_header_ptr = padding_header_value;
	if (padding_len > TLSF_ALLOC_MEMSIZE_T_SIZE) {
		memsize_t *padding_footer_ptr =
				(memsize_t*) ((unsigned char*) padding_header_ptr
						+ (padding_len - TLSF_ALLOC_MEMSIZE_T_SIZE));
		*padding_footer_ptr = padding_header_value;
	}
}

static unsigned char* used_block_get_user_data_ptr(memsize_t *block_header) {
	if (!header_is_padding_set(block_header)) {
		return (unsigned char*) (block_header + 1);
	}
	memsize_t *padding_header_ptr = block_header + 1;
	memsize_t padding_len = padding_get_padding_size(padding_header_ptr);
	return (unsigned char*) padding_header_ptr + padding_len;
}

static memsize_t* user_data_ptr_get_block_header(unsigned char *user_data_ptr) {
	memsize_t *block_header = (memsize_t*) (user_data_ptr
			- TLSF_ALLOC_BLOCK_HEADER_SIZE);
	if (is_padding_header(block_header)) {
		//is actually padding footer
		block_header = (memsize_t*) ((unsigned char*) block_header
				- padding_get_padding_size(block_header));
	}
	return block_header;
}

static void insert_free_block(alloc_header_t *alloc, memsize_t *block_header) {
	*block_header &= ~(TLSF_ALLOC_BLOCK_HEADER_USED_FLAG
			| TLSF_ALLOC_PADDING_HEADER_PADDING_FLAG_MASK);
	memsize_t size = header_get_block_size(block_header);
	size -= TLSF_ALLOC_BLOCK_HEADER_SIZE;
	l1_bitmap_t class_mask;
	l2_bitmap_t subclass_mask;
	unsigned short class_index;
	unsigned short subclass_index;
	size_to_bitmask_index(size, &class_mask, &subclass_mask, &class_index,
			&subclass_index);
	memsize_t *head = 0;
	if(alloc->free_block[class_index][subclass_index] & ~TLSF_ALLOC_FREE_BLOCK_OFFSET_VALID_FLAG)
	head =
			(memsize_t*) (alloc->managed_memory + (alloc->free_block[class_index][subclass_index] & ~TLSF_ALLOC_FREE_BLOCK_OFFSET_VALID_FLAG));
	header_set_prev_free_block(block_header, 0);
	header_set_next_free_block(block_header, head);
	alloc->free_block[class_index][subclass_index] =
			(memsize_t)((unsigned char*)block_header - alloc->managed_memory) | TLSF_ALLOC_FREE_BLOCK_OFFSET_VALID_FLAG;
	alloc->l1_bitmap |= class_mask;
	alloc->l2_bitmap[class_index] |= subclass_mask;
}

static void remove_free_block(alloc_header_t *alloc, memsize_t *block_header) {
	memsize_t size = header_get_block_size(block_header);
	size -= TLSF_ALLOC_BLOCK_HEADER_SIZE;
	l1_bitmap_t class_mask;
	l2_bitmap_t subclass_mask;
	unsigned short class_index;
	unsigned short subclass_index;
	size_to_bitmask_index(size, &class_mask, &subclass_mask, &class_index,
			&subclass_index);
	memsize_t *prev = header_get_prev_free_block(block_header);
	memsize_t *next = header_get_next_free_block(block_header);
	if (prev)
		header_set_next_free_block(prev, next);
	else
		if(next)
		alloc->free_block[class_index][subclass_index] =
				(memsize_t)((unsigned char*)next - alloc->managed_memory) | TLSF_ALLOC_FREE_BLOCK_OFFSET_VALID_FLAG;
		else alloc->free_block[class_index][subclass_index] = 0;
	if (next)
		header_set_prev_free_block(next, prev);
	if (alloc->free_block[class_index][subclass_index] == 0)
		alloc->l2_bitmap[class_index] &= ~subclass_mask;
	if (alloc->l2_bitmap[class_index] == 0)
		alloc->l1_bitmap &= ~class_mask;
	*block_header |= TLSF_ALLOC_BLOCK_HEADER_USED_FLAG;
}

static void recalculate_neighbor_sizes(alloc_header_t *alloc,
		memsize_t *prev_block_header, memsize_t *curr_block_header,
		memsize_t *next_block_header) {
	(void) alloc;
	if (prev_block_header) {
		memsize_t prev_size = (memsize_t) ((unsigned char*) curr_block_header
				- (unsigned char*) prev_block_header);
		{
			memsize_t old = *prev_block_header;
			memsize_t new = (old & ~TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK)
					| (prev_size & TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK);
			*(volatile memsize_t*) prev_block_header = new; //volatile for -O2, -Os, -O3 ([-Wstringop-overflow=])
		}
		{
			memsize_t old = *curr_block_header;
			memsize_t new = (old & ~TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_MASK)
					| (prev_size << TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_SHIFT);
			*(volatile memsize_t*) curr_block_header = new; //volatile for -O2, -Os, -O3 ([-Wstringop-overflow=])
		}
	} else {
		*curr_block_header &= ~TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_MASK;
	}
	if (next_block_header) {
		memsize_t curr_size = (memsize_t) ((unsigned char*) next_block_header
				- (unsigned char*) curr_block_header);
		{
			memsize_t old = *curr_block_header;
			memsize_t new = (old & ~TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK)
					| (curr_size & TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK);
			*(volatile memsize_t*) curr_block_header = new; //volatile for -O2, -Os, -O3 ([-Wstringop-overflow=])
		}
		{
			memsize_t old = *next_block_header;
			memsize_t new = (old & ~TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_MASK)
					| (curr_size << TLSF_ALLOC_BLOCK_HEADER_PREV_SIZE_SHIFT);
			*(volatile memsize_t*) next_block_header = new; //volatile for -O2, -Os, -O3 ([-Wstringop-overflow=])
		}
	}
}

static memsize_t* split_block_padding(alloc_header_t *alloc, memsize_t *block) {
	if (!header_is_padding_set(block))
		return block;
	memsize_t padding_len = padding_get_padding_size(block + 1); //from padding header
	memsize_t *prev_block = header_get_prev_in_mem_block(block);
	memsize_t *next_block = header_get_next_in_mem_block(block);
	memsize_t block_size = header_get_block_size(block);
	memsize_t *new_block_header = block;
	if (prev_block == block)
		prev_block = 0;
	if ((unsigned char*) next_block
			>= (alloc->managed_memory + alloc->managed_memory_bytelen))
		next_block = 0;

	if ((!prev_block || header_is_used_block(prev_block))) {
		if (padding_len >= TLSF_ALLOC_MIN_BLOCK_SIZE) {
			memsize_t *new_current_header_ptr =
					(memsize_t*) (used_block_get_user_data_ptr(block)
							- TLSF_ALLOC_BLOCK_HEADER_SIZE);
			memsize_t *new_padding_block = block;
			memsize_t new_current_block_size = block_size - padding_len;
			//volatile for -O2, -Os, -O3 ([-Wstringop-overflow=])
			*(volatile memsize_t*) new_current_header_ptr =
			TLSF_ALLOC_BLOCK_HEADER_USED_FLAG | new_current_block_size;
			recalculate_neighbor_sizes(alloc, prev_block, new_padding_block,
					new_current_header_ptr);
			recalculate_neighbor_sizes(alloc, new_padding_block,
					new_current_header_ptr, next_block);
			insert_free_block(alloc, new_padding_block);
			new_block_header = new_current_header_ptr;
		}
	} else {
		remove_free_block(alloc, prev_block);
		memsize_t *new_current_header_ptr =
				(memsize_t*) (used_block_get_user_data_ptr(block)
						- TLSF_ALLOC_BLOCK_HEADER_SIZE);
		memsize_t new_current_block_size = block_size - padding_len;
		*new_current_header_ptr = TLSF_ALLOC_BLOCK_HEADER_USED_FLAG
				| new_current_block_size;
		recalculate_neighbor_sizes(alloc, prev_block, block, next_block);
		insert_free_block(alloc, prev_block);
		new_block_header = new_current_header_ptr;
	}
	return new_block_header;
}

static void split_block_trailing(alloc_header_t *alloc, memsize_t *block,
		memsize_t user_data_len) {
	unsigned char *trailing_start = used_block_get_user_data_ptr(block)
			+ user_data_len;
	memsize_t align_mask = TLSF_ALLOC_BLOCK_HEADER_SIZE - 1;
	memsize_t aligned_offset = ((memsize_t) (trailing_start
			- alloc->managed_memory) + align_mask) & ~align_mask;
	trailing_start = alloc->managed_memory + aligned_offset;
	memsize_t *next_block = header_get_next_in_mem_block(block);
	unsigned char *block_end = (unsigned char*) next_block;
	if ((unsigned char*) next_block
			>= (alloc->managed_memory + alloc->managed_memory_bytelen))
		next_block = 0;

	memsize_t trailing_len = (memsize_t) (block_end - trailing_start);
	if (trailing_len > 0) {
		if (!next_block || header_is_used_block(next_block)) {
			if (trailing_len >= TLSF_ALLOC_MIN_BLOCK_SIZE) {
				memsize_t *new_trailing_block = (memsize_t*) trailing_start;
				//if next block doesn't exist, size won't be rebuilt
				*new_trailing_block = (memsize_t) (block_end - trailing_start);
				recalculate_neighbor_sizes(alloc, block, new_trailing_block,
						next_block);
				insert_free_block(alloc, new_trailing_block);
			}
		} else {
			//next block exists and is unused
			remove_free_block(alloc, next_block);
			memsize_t *new_trailing_block = (memsize_t*) trailing_start;
			memsize_t *after_next = header_get_next_in_mem_block(next_block);
			*new_trailing_block = (memsize_t) (block_end - trailing_start
					+ header_get_block_size(next_block));
			if ((unsigned char*) after_next
					>= (alloc->managed_memory + alloc->managed_memory_bytelen))
				after_next = 0;
			recalculate_neighbor_sizes(alloc, block, new_trailing_block,
					after_next);
			insert_free_block(alloc, new_trailing_block);

		}
	}
}

static memsize_t* coalesce_free_blocks(alloc_header_t *alloc, memsize_t *block) {
	memsize_t *unified = block;
	memsize_t unified_block_size = header_get_block_size(block);
	memsize_t *prev_block = header_get_prev_in_mem_block(block);
	memsize_t *next_block = header_get_next_in_mem_block(block);

	if (prev_block == block)
		prev_block = 0;
	if ((unsigned char*) next_block
			>= (alloc->managed_memory + alloc->managed_memory_bytelen))
		next_block = 0;

	memsize_t prev_free = (prev_block && !header_is_used_block(prev_block));
	memsize_t next_free = (next_block && !header_is_used_block(next_block));

	if (!prev_free && !next_free)
		return unified;
	remove_free_block(alloc, block);
	if (prev_free) {
		remove_free_block(alloc, prev_block);
		unified = prev_block;
		unified_block_size += header_get_block_size(prev_block);
	}

	if (next_free) {
		remove_free_block(alloc, next_block);
		unified_block_size += header_get_block_size(next_block);
	}

	*unified = (*unified & ~TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_MASK)
			| (unified_block_size) << TLSF_ALLOC_BLOCK_HEADER_CURR_SIZE_SHIFT;

	memsize_t *block_after_unified;
	if (next_free) {
		block_after_unified = header_get_next_in_mem_block(next_block);
		if ((unsigned char*) block_after_unified
				>= (alloc->managed_memory + alloc->managed_memory_bytelen))
			block_after_unified = 0;
	} else {
		block_after_unified = next_block;
	}

	memsize_t *block_before_unified = header_get_prev_in_mem_block(unified);
	if (block_before_unified == unified)
		block_before_unified = 0;
	recalculate_neighbor_sizes(alloc, block_before_unified, unified,
			block_after_unified);
	insert_free_block(alloc, unified);
	return unified;
}

static void coalesce_free_block_with_next_padding(alloc_header_t *alloc,
		memsize_t *block) {
	memsize_t *next_block = header_get_next_in_mem_block(block);
	if ((unsigned char*) next_block
			>= (alloc->managed_memory + alloc->managed_memory_bytelen))
		next_block = 0;

	if (!next_block || !header_is_padding_set(next_block)
			|| !header_is_used_block(next_block)) {
		return;
	}
	remove_free_block(alloc, block);
	memsize_t *after_next_block = header_get_next_in_mem_block(next_block);
	if ((unsigned char*) after_next_block
			>= (alloc->managed_memory + alloc->managed_memory_bytelen))
		after_next_block = 0;

	memsize_t *new_next_header = (memsize_t*) (used_block_get_user_data_ptr(
			next_block) - TLSF_ALLOC_BLOCK_HEADER_SIZE);
	*new_next_header = TLSF_ALLOC_BLOCK_HEADER_USED_FLAG;
	recalculate_neighbor_sizes(alloc, block, new_next_header, after_next_block);
	insert_free_block(alloc, block);
}

void tlsf_allocator_init(alloc_header_t *alloc_header, unsigned char *memory,
		memsize_t memory_bytelen) {
	alloc_header->managed_memory = memory;
	if (memory_bytelen > TLSF_ALLOC_MAX_BLOCK_SIZE)
		memory_bytelen = TLSF_ALLOC_MAX_BLOCK_SIZE;
	alloc_header->managed_memory_bytelen = memory_bytelen;

	alloc_header->l1_bitmap = 0;
	for (unsigned i = 0; i < TLSF_ALLOC_CLASS_COUNT; ++i) {
		alloc_header->l2_bitmap[i] = 0;
		for (unsigned j = 0; j < TLSF_ALLOC_SUBCLASS_COUNT; ++j)
			alloc_header->free_block[i][j] = 0;
	}

	free_block_header_t *block = (free_block_header_t*) memory;
	block->header = memory_bytelen;
	block->free_block_link = 0;

	insert_free_block(alloc_header, &block->header);
}

void* tlsf_alloc(alloc_header_t *alloc_header, memsize_t size,
		memsize_t alignment) {
	if ((size == 0) || ((alignment != 0) && (alignment & (alignment - 1))))
		return 0;
	if (size < TLSF_ALLOC_MIN_USER_DATA_SIZE)
		size = TLSF_ALLOC_MIN_USER_DATA_SIZE;
	size = (size + TLSF_ALLOC_BLOCK_HEADER_SIZE - 1)
			& ~(TLSF_ALLOC_BLOCK_HEADER_SIZE - 1);

	l1_bitmap_t class_mask;
	l2_bitmap_t subclass_mask;
	unsigned short class_index;
	unsigned short subclass_index;
	size_to_bitmask_index(size, &class_mask, &subclass_mask, &class_index,
			&subclass_index);
	l1_bitmap_t l1 = alloc_header->l1_bitmap
			& (~((unsigned int) 0) << class_index);
	if (!l1)
		return 0;
	unsigned short found_class = trailing_zeroes_bitmap(l1);
	for (unsigned short ci = found_class; ci < TLSF_ALLOC_CLASS_COUNT; ++ci) {
		l2_bitmap_t l2 = alloc_header->l2_bitmap[ci];
		if (ci == class_index)
			l2 &= (~((unsigned int) 0) << subclass_index);
		if (!l2)
			continue;
		unsigned short start_sub = trailing_zeroes_bitmap(l2);

		unsigned short si = start_sub;
		l2_bitmap_t remaining = l2 >> si;   // shift so bit 0 corresponds to si

		while (remaining) {
			unsigned short offset = trailing_zeroes_bitmap(remaining);
			unsigned short subclass = si + offset;
			free_block_header_t *free_block = 0;
			if((alloc_header->free_block[ci][subclass] & TLSF_ALLOC_FREE_BLOCK_OFFSET_VALID_FLAG))
			free_block = (free_block_header_t*)
					(alloc_header->managed_memory +
					(alloc_header->free_block[ci][subclass] & ~TLSF_ALLOC_FREE_BLOCK_OFFSET_VALID_FLAG));

			if (free_block) {
				memsize_t *block_header = &free_block->header;
				memsize_t block_size = header_get_block_size(block_header);

				memsize_t block_usable;
				memsize_t block_padding;
				free_block_get_usable_memory_padding_len(block_header,
						alignment, &block_usable, &block_padding);

				unsigned char *aligned_user_ptr =
						(unsigned char*) used_block_get_user_data_ptr(
								block_header) + block_padding;

				unsigned char *block_end = (unsigned char*) block_header
						+ block_size;

				if (aligned_user_ptr + size <= block_end) {
					remove_free_block(alloc_header, block_header);
					block_set_padding_header_footer(block_header,
							block_padding);
					split_block_trailing(alloc_header, block_header, size);
					block_header = split_block_padding(alloc_header,
							block_header);
					return used_block_get_user_data_ptr(block_header);
				}
			}

			remaining &= (remaining - 1);
		}

	}
	return 0;
}

void tlsf_free(alloc_header_t *alloc_header, void *obj) {
	memsize_t *block_header = user_data_ptr_get_block_header(
			(unsigned char*) obj);
	insert_free_block(alloc_header, block_header);
	block_header = coalesce_free_blocks(alloc_header, block_header);
	coalesce_free_block_with_next_padding(alloc_header, block_header);
}

#if (TLSF_ALLOC_ENABLE_DEBUG_MODE == 1)
//debugging functions
#include <stdio.h>
static void tlsf_get_bin_usable_range(int l1, int l2, memsize_t *min_usable,
		memsize_t *max_usable) {
	const memsize_t class_range = (1U << TLSF_ALLOC_CLASS_BITSHIFT) << l1;
	const memsize_t class_base = class_range;
	const memsize_t subclass_span = class_range / TLSF_ALLOC_SUBCLASS_COUNT;
	const memsize_t subclass_base = class_base + (l2 * subclass_span);
	const memsize_t subclass_end = subclass_base + subclass_span - 1;
	*min_usable = subclass_base;
	*max_usable = subclass_end;
}

static inline memsize_t get_block_offset(alloc_header_t *alloc,
		memsize_t *block) {
	return (memsize_t) ((unsigned char*) block - alloc->managed_memory);
}

void tlsf_print_bins(alloc_header_t *alloc_header) {
	printf("=== TLSF Bins ===\n");

	for (int l1 = 0; l1 < TLSF_ALLOC_CLASS_COUNT; l1++) {
		l1_bitmap_t l1bm = alloc_header->l1_bitmap;
		int l1_set = (l1bm >> l1) & 1;

		printf("L1 %02d: %s\n", l1, l1_set ? "SET" : "clr");

		if (!l1_set)
			continue;

		l2_bitmap_t l2bm = alloc_header->l2_bitmap[l1];

		for (int l2 = 0; l2 < TLSF_ALLOC_SUBCLASS_COUNT; l2++) {
			int l2_set = (l2bm >> l2) & 1;

			memsize_t min_usable, max_usable;
			tlsf_get_bin_usable_range(l1, l2, &min_usable, &max_usable);

			printf("  L2 %02d: %s  size: %08llu - %08llu bytes", l2,
					l2_set ? "SET" : "clr", (unsigned long long) min_usable,
					(unsigned long long) max_usable);

			free_block_header_t *block = 0;
			if((alloc_header->free_block[l1][l2] & TLSF_ALLOC_FREE_BLOCK_OFFSET_VALID_FLAG))
			block = (free_block_header_t *)(alloc_header->managed_memory + (alloc_header->free_block[l1][l2] & ~TLSF_ALLOC_FREE_BLOCK_OFFSET_VALID_FLAG));

			memsize_t count = 0;
			free_block_header_t *b_next = 0;
			for (free_block_header_t *b = block; b != 0;) {
				count++;
				b_next = (free_block_header_t*) header_get_next_free_block(
						(memsize_t*) b);
				if (b_next == b || b_next == 0) {
					b = 0;
				} else {
					b = b_next;
				}
			}

			printf("  count=%llu\n", (unsigned long long) count);

			/* Print offsets and sizes of blocks in this bin */
			for (free_block_header_t *b = block; b != 0;
					b = (free_block_header_t*) header_get_next_free_block(
							(memsize_t*) b)) {
				memsize_t off = get_block_offset(alloc_header, (memsize_t*) b);
				memsize_t size = header_get_block_size((memsize_t*) b);
				memsize_t usable = size - sizeof(memsize_t);

				printf("    - offset=%08llu usable=%08llu size=%08llu\n",
						(unsigned long long) off, (unsigned long long) usable,
						(unsigned long long) size);
			}

		}
	}

	printf("=================\n");
}

void tlsf_print_memory(alloc_header_t *alloc_header) {
	unsigned char *mem_start = alloc_header->managed_memory;
	unsigned char *mem_end = alloc_header->managed_memory
			+ alloc_header->managed_memory_bytelen;

	memsize_t *block = (memsize_t*) mem_start;
	memsize_t block_index = 0;

	while ((unsigned char*) block < mem_end) {
		memsize_t size = header_get_block_size(block);
		memsize_t used = header_is_used_block(block);
		memsize_t padding = header_is_padding_set(block);

		printf("Block %04llu ", (unsigned long long) block_index);
		printf("S=%08llu ", (unsigned long long) size);

		if (!used) {
			printf("F\n");
		} else {
			if (padding) {
				memsize_t pad_size = padding_get_padding_size(block + 1);
				printf("U P=%08llu\n", (unsigned long long) pad_size);
			} else {
				printf("U P=00000000\n");
			}
		}

		block = header_get_next_in_mem_block(block);
		block_index++;
	}
}

#endif
