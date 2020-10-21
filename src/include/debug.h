
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// debug.h for SimpleXX/SimpleKernel.

#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"

void debug_init(ptr_t magic, ptr_t addr);
void print_cur_status(void);
void panic(const char *msg);
void print_stack(size_t count);

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_H_ */
