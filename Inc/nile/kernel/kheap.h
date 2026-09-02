#ifndef NILE_KHEAP_H_
#define NILE_KHEAP_H_

#include "nile/stdtypes.h"
#include "allocator_tlsfa.h"

typedef void (*nile_kheap_init_fn)(
    alloc_header_t *alloc_header,
    unsigned char *memory,
    memsize_t memory_bytelen);

typedef void* (*nile_kheap_alloc_fn)(
    alloc_header_t *alloc_header,
    memsize_t size,
    memsize_t alignment);

typedef void (*nile_kheap_free_fn)(
    alloc_header_t *alloc_header,
    void *obj);

typedef struct nile_kernel_kheap{
	alloc_header_t heap;
	nile_kheap_init_fn  init;
	nile_kheap_alloc_fn alloc;
	nile_kheap_free_fn  free;
}nile_kernel_kheap;

void kheap_init();
void* kalloc(memsize_t size, memsize_t alignment);
void kfree(void* obj);

#endif /* NILE_KHEAP_H_ */
