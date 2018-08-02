
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pmm.h for MRNIU/SimpleKernel.


#ifndef _PMM_H_
#define _PMM_H_

#include "stdint.h"
#include "stdio.h"
#include "multiboot.h"

extern uint8_t kern_start[];
extern uint8_t kern_end[];

void show_memory_map();

#endif
