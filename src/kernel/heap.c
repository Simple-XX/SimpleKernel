
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// heap.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdlib.h"
#include "mem/pmm.h"

void * kmalloc(size_t byte) {
	return (void *)pmm_alloc(byte);
}
void kfree(void * page) {
	pmm_free_page((ptr_t)page);
	return;
}

#ifdef __cplusplus
}
#endif
