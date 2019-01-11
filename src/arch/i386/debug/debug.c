
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

void print_MULTIBOOT_TAG_TYPE_CMDLINE(struct multiboot_tag *tag){
		printk ("Command line = %s\n",
		        ((struct multiboot_tag_string *) tag)->string);
		return;
}

void print_MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME(struct multiboot_tag *tag){
		printk ("Boot loader name = %s\n",
		        ((struct multiboot_tag_string *) tag)->string);
		return;
}

void print_MULTIBOOT_TAG_TYPE_MODULE(struct multiboot_tag *tag){
		printk ("Module at 0x%x-0x%x. Command line %s\n",
		        ((struct multiboot_tag_module *) tag)->mod_start,
		        ((struct multiboot_tag_module *) tag)->mod_end,
		        ((struct multiboot_tag_module *) tag)->cmdline);
		return;
}

void print_MULTIBOOT_TAG_TYPE_BASIC_MEMINFO(struct multiboot_tag *tag){
		printk ("mem_lower = %uKB, mem_upper = %uKB\n",
		        ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
		        ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
		return;
}

void print_MULTIBOOT_TAG_TYPE_BOOTDEV(struct multiboot_tag *tag){
		printk ("Boot device 0x%x,%u,%u\n",
		        ((struct multiboot_tag_bootdev *) tag)->biosdev,
		        ((struct multiboot_tag_bootdev *) tag)->slice,
		        ((struct multiboot_tag_bootdev *) tag)->part);
		return;
}

void print_MULTIBOOT_TAG_TYPE_MMAP(struct multiboot_tag *tag){
		multiboot_memory_map_t *mmap;
		mmap = ((struct multiboot_tag_mmap *) tag)->entries;
		printk ("mmap\n");
		for (mmap; (uint8_t *) mmap< (uint8_t *) tag + tag->size;
		     mmap = (multiboot_memory_map_t *)((unsigned long) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size))
				printk ("base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
				        (unsigned) (mmap->addr >> 32),
				        (unsigned) (mmap->addr & 0xffffffff),
				        (unsigned) (mmap->len >> 32),
				        (unsigned) (mmap->len & 0xffffffff),
				        (unsigned) mmap->type);
		return;
}

void print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(struct multiboot_tag *tag){
		printk ("Elf type 0x%x, Size 0x%x, num 0x%x, entsize 0x%x, shndx 0x%x.\n",
		        ((struct multiboot_tag_elf_sections *) tag)->type,
		        ((struct multiboot_tag_elf_sections *) tag)->size,
		        ((struct multiboot_tag_elf_sections *) tag)->num,
		        ((struct multiboot_tag_elf_sections *) tag)->entsize,
		        ((struct multiboot_tag_elf_sections *) tag)->shndx);
		return;
}

void print_MULTIBOOT_TAG_TYPE_APM(struct multiboot_tag *tag){
		tag=(struct multiboot_tag_apm *) tag;
		printk ("APM type 0x%x, Size 0x%x, version 0x%x, cseg 0x%x, offset 0x%x, cseg_16 0x%x, \n"
		        "dseg 0x%x, flags 0x%x, cseg_len 0x%x, cseg_16_len 0x%x, dseg_len 0x%x\n",
		        ((struct multiboot_tag_apm *)tag)->type,
		        ((struct multiboot_tag_apm *)tag)->size,
		        ((struct multiboot_tag_apm *)tag)->version,
		        ((struct multiboot_tag_apm *)tag)->cseg,
		        ((struct multiboot_tag_apm *)tag)->offset,
		        ((struct multiboot_tag_apm *)tag)->cseg_16,
		        ((struct multiboot_tag_apm *)tag)->dseg,
		        ((struct multiboot_tag_apm *)tag)->flags,
		        ((struct multiboot_tag_apm *)tag)->cseg_len,
		        ((struct multiboot_tag_apm *)tag)->cseg_16_len,
		        ((struct multiboot_tag_apm *)tag)->dseg_len);
		return;
}

bool is_multiboot2_header(uint32_t magic, uint32_t addr){
		if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
				printk ("Invalid magic number: %x\n", (unsigned) magic);
				return false;
		}
		if (addr & 7) {
				printk ("Unaligned addr: 0x%08x\n", addr);
				return false;
		}
		return true;
}

void multiboot2_init(uint32_t magic, uint32_t addr){
		// Am I booted by a Multiboot-compliant boot loader?
		is_multiboot2_header(magic, addr);

		struct multiboot_tag *tag;
		uint64_t size;
		size = *(uint64_t *) addr;

		printk ("Announced mbi size 0x%x\n", size);

		tag = (struct multiboot_tag *) (addr + 8);
		printk("tag type: %X\n", tag->type);
		printk("tag size: %X\n", tag->size);

		for (tag = (struct multiboot_tag *) (addr + 8);
		     tag->type != MULTIBOOT_TAG_TYPE_END;
		     tag = (struct multiboot_tag *) ((uint8_t *) tag + ((tag->size + 7) & ~7))) {
				printk ("Tag 0x%x, Size 0x%x\n", tag->type, tag->size);
				switch (tag->type) {
				case MULTIBOOT_TAG_TYPE_CMDLINE:
						// print_MULTIBOOT_TAG_TYPE_CMDLINE(tag);
						break;
				case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
						// print_MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME(tag);
						break;
				case MULTIBOOT_TAG_TYPE_MODULE:
						print_MULTIBOOT_TAG_TYPE_MODULE(tag);
						break;
				case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
						print_MULTIBOOT_TAG_TYPE_BASIC_MEMINFO(tag);
						break;
				case MULTIBOOT_TAG_TYPE_BOOTDEV:
						print_MULTIBOOT_TAG_TYPE_BOOTDEV(tag);
						break;
				case MULTIBOOT_TAG_TYPE_MMAP:
						print_MULTIBOOT_TAG_TYPE_MMAP(tag);
						break;
				case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
						print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(tag);
						break;
				case MULTIBOOT_TAG_TYPE_APM:
						print_MULTIBOOT_TAG_TYPE_APM(tag);
						break;
				}
		}
		tag = (struct multiboot_tag *) ((uint8_t *) tag + ((tag->size + 7) & ~7));
		printk ("Total mbi size 0x%x\n", (unsigned) tag - addr);
}

void debug_init(uint32_t magic, uint32_t addr) {
		multiboot2_init(magic, addr);



		return;
}
