
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
// debug.c for MRNIU/SimpleKernel.

#include "debug.h"
#include "cpu.hpp"


void print_cur_status(){
		static int round = 0;
		uint16_t reg1, reg2, reg3, reg4;
		asm volatile (  "mov %%cs, %0;"
		                "mov %%ds, %1;"
		                "mov %%es, %2;"
		                "mov %%ss, %3;"
		                : "=m" (reg1), "=m" (reg2), "=m" (reg3), "=m" (reg4));

		// 打印当前的运行级别
		printk("%d:  @ring %d\n", round, reg1 & 0x3);
		printk("%d:  cs = %x\n", round, reg1);
		printk("%d:  ds = %x\n", round, reg2);
		printk("%d:  es = %x\n", round, reg3);
		printk("%d:  ss = %x\n", round, reg4);
		++round;
}

/*  Forward declarations. */
void debug233 (uint64_t magic, uint64_t addr);

/*  Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */

void debug233 (uint64_t magic, uint64_t addr){
		struct multiboot_tag *tag;
		unsigned long size;
		size = *(unsigned long *) addr;
		uint32_t tmp233=addr;
		printk ("addr: %X\n", addr);
		printk ("size: %X\n", size);
		printk ("tmp233: %X\n", tmp233);
		/*  Am I booted by a Multiboot-compliant boot loader? */
		if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
				printk ("Invalid magic number: 0x%x\n", (unsigned) magic);
				printk("%X\n", MULTIBOOT2_BOOTLOADER_MAGIC);
				return;
		}
		unsigned long tmp=addr;
		if (tmp & 7) {
				printk ("Unaligned addr: 0x%08x\n", addr);
				printk ("Unaligned tmp: 0x%08x\n", tmp);
				return;
		}

		printk ("Announced mbi size 0x%x\n", size);


		tag = (struct multiboot_tag *) (addr + 8);
		printk("tag type: %X\n", tag->type);
		printk("tag size: %X\n", tag->size);


		for (tag = (struct multiboot_tag *) (addr + 8);
		     tag->type != MULTIBOOT_TAG_TYPE_END;
		     tag = (struct multiboot_tag *) ((uint8_t *) tag
		                                     + ((tag->size + 7) & ~7)))
		{
				printk ("Tag 0x%x, Size 0x%x\n", tag->type, tag->size);
				switch (tag->type) {
				case MULTIBOOT_TAG_TYPE_CMDLINE:
						printk ("Command line = %s\n",
						        ((struct multiboot_tag_string *) tag)->string);
						break;
				case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
						printk ("Boot loader name = %s\n",
						        ((struct multiboot_tag_string *) tag)->string);
						break;
				case MULTIBOOT_TAG_TYPE_MODULE:
						printk ("Module at 0x%x-0x%x. Command line %s\n",
						        ((struct multiboot_tag_module *) tag)->mod_start,
						        ((struct multiboot_tag_module *) tag)->mod_end,
						        ((struct multiboot_tag_module *) tag)->cmdline);
						break;
				case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
						printk ("mem_lower = %uKB, mem_upper = %uKB\n",
						        ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
						        ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
						break;
				case MULTIBOOT_TAG_TYPE_BOOTDEV:
						printk ("Boot device 0x%x,%u,%u\n",
						        ((struct multiboot_tag_bootdev *) tag)->biosdev,
						        ((struct multiboot_tag_bootdev *) tag)->slice,
						        ((struct multiboot_tag_bootdev *) tag)->part);
						break;
				case MULTIBOOT_TAG_TYPE_MMAP: {
						multiboot_memory_map_t *mmap;

						printk ("mmap\n");

						for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
						     (uint8_t *) mmap
						     < (uint8_t *) tag + tag->size;
						     mmap = (multiboot_memory_map_t *)
						            ((unsigned long) mmap
						             + ((struct multiboot_tag_mmap *) tag)->entry_size))
								printk (" base_addr = 0x%x%x,"
								        " length = 0x%x%x, type = 0x%x\n",
								        (unsigned) (mmap->addr >> 32),
								        (unsigned) (mmap->addr & 0xffffffff),
								        (unsigned) (mmap->len >> 32),
								        (unsigned) (mmap->len & 0xffffffff),
								        (unsigned) mmap->type);
				}
				break;
				}
		}
		tag = (struct multiboot_tag *) ((uint8_t *) tag
		                                + ((tag->size + 7) & ~7));
		printk ("Total mbi size 0x%x\n", (unsigned) tag - addr);
}



void debug_init(uint64_t magic, unsigned long addr) {
		debug233(magic, addr);

		// 从 GRUB 提供的信息中获取到内核符号表和代码地址信息

		return;
}
