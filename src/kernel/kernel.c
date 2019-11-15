
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif


#include "include/kernel.h"
#include "../test/include/test.h"

// 内核入口
// 指针是 32 位的
void kernel_main(uint32_t magic, uint32_t addr) {
	UNUSED(magic);
	UNUSED(addr);

	while (1);

	return;
}


#ifdef __cplusplus
}
#endif
