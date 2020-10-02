
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.h for Simple-XX/SimpleKernel.

#ifndef _PMM_H_
#define _PMM_H_

#include "stdint.h"
#include "stdio.h"
#include "stddef.h"
#include "debug.h"
#include "multiboot2.h"

extern ptr_t kernel_init_start[];
extern ptr_t kernel_init_text_start[];
extern ptr_t kernel_init_text_end[];
extern ptr_t kernel_init_data_start[];
extern ptr_t kernel_init_data_end[];
extern ptr_t kernel_init_end[];

extern ptr_t kernel_start[];
extern ptr_t kernel_text_start[];
extern ptr_t kernel_text_end[];
extern ptr_t kernel_data_start[];
extern ptr_t kernel_data_end[];
extern ptr_t kernel_end[];

#endif /* _PMM_H_ */
