
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
bool is_multiboot2_header(uint32_t magic, uint32_t addr);
void multiboot2_init(uint32_t magic, uint32_t addr);
void print_MULTIBOOT_TAG_TYPE_CMDLINE(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_CMDLINE(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_MODULE(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_BASIC_MEMINFO(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_BOOTDEV(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_MMAP(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(struct multiboot_tag *tag);
void print_MULTIBOOT_TAG_TYPE_APM(struct multiboot_tag *tag);
void print_cur_status(void);

#endif
