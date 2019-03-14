
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// debug.h for MRNIU/SimpleKernel.


#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "stdarg.h"
#include "stddef.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "multiboot2.h"
#include "elf.h"
#include "intr/intr.h"

void debug_init(uint32_t magic, uint32_t addr);
void print_cur_status(void);
void panic(const char *msg);
const char * elf_lookup_symbol(uint32_t addr, elf_t *elf);
void print_stack_trace(void);

#endif
