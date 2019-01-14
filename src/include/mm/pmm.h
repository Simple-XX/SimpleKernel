
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pmm.h for MRNIU/SimpleKernel.


#ifndef _PMM_H_
#define _PMM_H_

#include "stdint.h"
#include "stdio.h"
#include "multiboot2.h"

extern uint8_t kernel_start[];
extern uint8_t kernel_end[];





void show_memory_map();

#endif
