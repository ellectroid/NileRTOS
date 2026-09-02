#include "nile/stdtypes.h"
#include "nile/kernel.h"
void kheap_init(){
	unsigned char* kheap_memory = (unsigned char*)(((NILE_MEMORY_KERNEL_ADDR + sizeof(nile_kernel) + 3) & ~(0x03)));
	((nile_kernel*) NILE_MEMORY_KERNEL_ADDR)->heap.init(&((nile_kernel*) NILE_MEMORY_KERNEL_ADDR)->heap.heap, kheap_memory, NILE_MEMORY_KHEAP_BYTESIZE);
}

void* kalloc(memsize_t size, memsize_t alignment){
	return ((nile_kernel*) NILE_MEMORY_KERNEL_ADDR)->heap.alloc(&((nile_kernel*) NILE_MEMORY_KERNEL_ADDR)->heap.heap, size, alignment);
}

void kfree(void* obj){
	((nile_kernel*) NILE_MEMORY_KERNEL_ADDR)->heap.free(&((nile_kernel*) NILE_MEMORY_KERNEL_ADDR)->heap.heap, obj);
}
