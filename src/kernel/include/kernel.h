
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

extern "C" void kernel_main(uint32_t size, void *addr);

int  test_pmm(void);
int  test_vmm(void);
int  test_heap(void);
int  test_vfs(void);
void show_info(void);

#endif /* _KERNEL_H_ */
