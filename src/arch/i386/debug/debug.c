
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-5.html
// debug.c for MRNIU/SimpleKernel.

#include "debug.h"
#include "cpu.hpp"
#include "multiboot.h"
#include "stdio.h"

void is_magic(uint32_t magic){
		if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
				printk ("Invalid magic number: 0x%x\n", (unsigned) magic);
		return;
}

void debug_init(uint32_t magic, uint32_t addr){
		multiboot_info = (multiboot_info_t *) addr;
		mag = magic;


		return;
}
