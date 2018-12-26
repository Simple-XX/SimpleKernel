
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-5.html
// debug.h for MRNIU/SimpleKernel.


#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "stdarg.h"
#include "stddef.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "multiboot.h"
#include "elf.h"
#include "intr/intr.h"

/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))
#define ELF32_ST_TYPE(i) ((i)&0xf)


// void debug_init(uint64_t magic, multiboot_info_t * mb);


#endif
