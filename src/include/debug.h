
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

/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))
#define ELF32_ST_TYPE(i) ((i)&0xf)


// void debug_init(uint64_t magic, multiboot_info_t * mb);
void debug_init(uint32_t magic, uint32_t addr);
void multiboot2_init(uint32_t magic, uint32_t addr);
void print_cur_status();
void print_MULTIBOOT_TAG_TYPE_CMDLINE(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_CMDLINE(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_MODULE(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_BASIC_MEMINFO(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_BOOTDEV(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_MMAP(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_APM(struct multiboot_tag *tag);
bool is_multiboot2_header(uint32_t magic, uint32_t addr);



#endif
