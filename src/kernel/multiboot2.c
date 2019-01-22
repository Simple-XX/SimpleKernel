
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
// debug.c for MRNIU/SimpleKernel.


#include "multiboot2.h"

void print_MULTIBOOT_TAG_TYPE_CMDLINE(struct multiboot_tag *tag){
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("Command line = %s\n",
		        ((struct multiboot_tag_string *) tag)->string);
		return;
}

void print_MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME(struct multiboot_tag *tag){
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("Boot loader name = %s\n",
		        ((struct multiboot_tag_string *) tag)->string);
		return;
}

void print_MULTIBOOT_TAG_TYPE_MODULE(struct multiboot_tag *tag){
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("Module at 0x%X-0x%X. Command line %s\n",
		        ((struct multiboot_tag_module *) tag)->mod_start,
		        ((struct multiboot_tag_module *) tag)->mod_end,
		        ((struct multiboot_tag_module *) tag)->cmdline);
		return;
}

void print_MULTIBOOT_TAG_TYPE_BASIC_MEMINFO(struct multiboot_tag *tag){
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk("meminfo: \n");
		printk ("mem_lower = %uKB, mem_upper = %uKB\n",
		        ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
		        ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
		return;
}

void print_MULTIBOOT_TAG_TYPE_BOOTDEV(struct multiboot_tag *tag){
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("Boot device 0x%X,%u,%u\n",
		        ((struct multiboot_tag_bootdev *) tag)->biosdev,
		        ((struct multiboot_tag_bootdev *) tag)->slice,
		        ((struct multiboot_tag_bootdev *) tag)->part);
		return;
}

void print_MULTIBOOT_TAG_TYPE_MMAP(struct multiboot_tag *tag){
		multiboot_memory_map_entry_t *mmap;
		mmap = ((struct multiboot_tag_mmap *) tag)->entries;
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("mmap\n");
		for (; (uint8_t *) mmap< (uint8_t *) tag + tag->size;
		     mmap = (multiboot_memory_map_entry_t *)((unsigned long) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size)) {
				printk_color(COL_DEBUG, "[DEBUG] ");
				printk ("base_addr = 0x%X%X, length = 0x%X%X, type = 0x%X\n",
				        (unsigned) (mmap->addr >> 32), // high
				        (unsigned) (mmap->addr & 0xffffffff), // low
				        (unsigned) (mmap->len >> 32), // high
				        (unsigned) (mmap->len & 0xffffffff), // low
				        (unsigned) mmap->type);
		}
		return;
}

void set_mem_info(struct multiboot_tag *tag){
		mmap_entries = ((struct multiboot_tag_mmap *) tag)->entries;
		return;
}

void print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(struct multiboot_tag *tag){
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("Elf type 0x%X, Size 0x%X, num 0x%X, entsize 0x%X, shndx 0x%X.\n",
		        ((struct multiboot_tag_elf_sections *) tag)->type,
		        ((struct multiboot_tag_elf_sections *) tag)->size,
		        ((struct multiboot_tag_elf_sections *) tag)->num,
		        ((struct multiboot_tag_elf_sections *) tag)->entsize,
		        ((struct multiboot_tag_elf_sections *) tag)->shndx);
		return;
}

void print_MULTIBOOT_TAG_TYPE_APM(struct multiboot_tag *tag){
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("APM type 0x%X, Size 0x%X, version 0x%X, cseg 0x%X, offset 0x%X, cseg_16 0x%X, "
		        "dseg 0x%X, flags 0x%X, cseg_len 0x%X, cseg_16_len 0x%X, dseg_len 0x%X\n",
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

void print_MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR(struct multiboot_tag *tag){
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("Image load base physical address type 0x%X, size 0x%X, addr 0x%X.\n",
		        ((struct multiboot_tag_load_base_addr *) tag)->type,
		        ((struct multiboot_tag_load_base_addr *) tag)->size,
		        ((struct multiboot_tag_load_base_addr *) tag)->load_base_addr);
		return;
}

bool is_multiboot2_header(uint32_t magic, uint32_t addr){
		if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
				printk_color(COL_ERROR, "[Error] ");
				printk ("Invalid magic number: %X\n", (unsigned) magic);
				return false;
		}
		if (addr & 7) {
				printk_color(COL_ERROR, "[Error] ");
				printk ("Unaligned addr: 0x%08x\n", addr);
				return false;
		}
		return true;
}


void multiboot2_init(uint32_t magic, uint32_t addr){
		// Am I booted by a Multiboot-compliant boot loader?
		is_multiboot2_header(magic, addr);

		uint32_t size=*(uint32_t *) addr;
		// addr+0 保存大小，下一字节开始为 tag 信息
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("Announced mbi size 0x%X\n", size);

		uint32_t tag_addr = (uint32_t)addr+8;
		struct multiboot_tag *tag;
		// tag = (struct multiboot_tag *) tag_addr;
		// printk("tag type: %X\n", tag->type);
		// printk("tag size: %X\n", tag->size);

		for (tag = (struct multiboot_tag *)tag_addr;
		     tag->type != MULTIBOOT_TAG_TYPE_END;
		     // (tag 低八位) + (tag->size 八位对齐)
		     tag = (struct multiboot_tag *) ((uint8_t *) tag + ((tag->size + 7) & ~7))) {
				// printk_color(COL_DEBUG, "[DEBUG] ");
				// printk ("Tag 0x%X, Size 0x%X\n", tag->type, tag->size);
				switch (tag->type) {
				case MULTIBOOT_TAG_TYPE_CMDLINE:
						// print_MULTIBOOT_TAG_TYPE_CMDLINE(tag);
						break;
				case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
						// print_MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME(tag);
						break;
				case MULTIBOOT_TAG_TYPE_MODULE:
						// print_MULTIBOOT_TAG_TYPE_MODULE(tag);
						break;
				case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
						// print_MULTIBOOT_TAG_TYPE_BASIC_MEMINFO(tag);
						break;
				case MULTIBOOT_TAG_TYPE_BOOTDEV:
						// print_MULTIBOOT_TAG_TYPE_BOOTDEV(tag);
						break;
				case MULTIBOOT_TAG_TYPE_MMAP:
						print_MULTIBOOT_TAG_TYPE_MMAP(tag);
						set_mem_info(tag);
						pmm_init(tag);
						break;
				case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
						// print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(tag);
						break;
				case MULTIBOOT_TAG_TYPE_APM:
						// print_MULTIBOOT_TAG_TYPE_APM(tag);
						break;
				case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
						print_MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR(tag);
						break;
				}
		}
		tag = (struct multiboot_tag *) ((uint8_t *) tag + ((tag->size + 7) & ~7));
		printk_color(COL_DEBUG, "[DEBUG] ");
		printk ("Total mbi size 0x%X\n", (unsigned) tag - addr);
		return;
}
