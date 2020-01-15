
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// debug.h for MRNIU/SimpleKernel.

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "intr/include/intr.h"
#include "elf.h"

#define DEBUG 1

void debug_init(ptr_t magic, ptr_t addr);
void print_cur_status(void);
void panic(const char * msg);
const char * elf_lookup_symbol(ptr_t addr, elf_t * elf);
void print_stack_trace(void);

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_H_ */
