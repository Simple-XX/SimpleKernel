
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
// debug.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "multiboot2.h"
#include "string.h"

void print_MULTIBOOT_TAG_TYPE_CMDLINE(multiboot_tag_t * tag) {
	printk_info("Command line = %s\n",
		( (struct multiboot_tag_string *)tag)->string);
	return;
}

void print_MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME(multiboot_tag_t * tag) {
	printk_info("Boot loader name = %s\n",
		( (struct multiboot_tag_string *)tag)->string);
	return;
}

void print_MULTIBOOT_TAG_TYPE_MODULE(multiboot_tag_t * tag) {
	printk_info("Module at 0x%X-0x%X. Command line %s\n",
		( (struct multiboot_tag_module *)tag)->mod_start,
		( (struct multiboot_tag_module *)tag)->mod_end,
		( (struct multiboot_tag_module *)tag)->cmdline);
	return;
}

void print_MULTIBOOT_TAG_TYPE_BASIC_MEMINFO(multiboot_tag_t * tag) {
	printk_info("mem_lower = %uKB, mem_upper = %uKB\n",
		( (struct multiboot_tag_basic_meminfo *)tag)->mem_lower,
		( (struct multiboot_tag_basic_meminfo *)tag)->mem_upper);
	return;
}

void print_MULTIBOOT_TAG_TYPE_BOOTDEV(multiboot_tag_t * tag) {
	printk_info("Boot device 0x%X,%u,%u\n",
		( (struct multiboot_tag_bootdev *)tag)->biosdev,
		( (struct multiboot_tag_bootdev *)tag)->slice,
		( (struct multiboot_tag_bootdev *)tag)->part);
	return;
}

void print_MULTIBOOT_TAG_TYPE_MMAP(multiboot_tag_t * tag) {
	mmap_entries = ( (struct multiboot_tag_mmap *)tag)->entries;
	mmap_tag = tag;
	multiboot_memory_map_entry_t * mmap;
	mmap = ( (struct multiboot_tag_mmap *)tag)->entries;
#if DEBUG
	printk_info("mmap\n");
	for( ; (uint8_t *)mmap < (uint8_t *)tag + tag->size ;
	    mmap = (multiboot_memory_map_entry_t *)( (uint32_t)mmap + ( (struct multiboot_tag_mmap *)tag)->entry_size) ) {
		printk_info("base_addr = 0x%X%X, length = 0x%X%X, type = 0x%X\n",
			(unsigned)(mmap->addr >> 32),// high
			(unsigned)(mmap->addr & 0xffffffff),// low
			(unsigned)(mmap->len >> 32),// high
			(unsigned)(mmap->len & 0xffffffff),// low
			(unsigned)mmap->type);
	}
#endif
	return;
}

void print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(multiboot_tag_t * tag) {
#if DEBUG
	printk_info("Elf type 0x%X, Size 0x%X, num 0x%X, entsize 0x%X, shndx 0x%X.\n",
		( (struct multiboot_tag_elf_sections *)tag)->type,
		( (struct multiboot_tag_elf_sections *)tag)->size,
		( (struct multiboot_tag_elf_sections *)tag)->num,
		( (struct multiboot_tag_elf_sections *)tag)->entsize,
		( (struct multiboot_tag_elf_sections *)tag)->shndx);
#endif
	return;
}

void print_MULTIBOOT_TAG_TYPE_APM(multiboot_tag_t * tag) {
	printk_info("APM type 0x%X, Size 0x%X, version 0x%X, cseg 0x%X, offset 0x%X, cseg_16 0x%X, "
		"dseg 0x%X, flags 0x%X, cseg_len 0x%X, cseg_16_len 0x%X, dseg_len 0x%X\n",
		( (struct multiboot_tag_apm *)tag)->type,
		( (struct multiboot_tag_apm *)tag)->size,
		( (struct multiboot_tag_apm *)tag)->version,
		( (struct multiboot_tag_apm *)tag)->cseg,
		( (struct multiboot_tag_apm *)tag)->offset,
		( (struct multiboot_tag_apm *)tag)->cseg_16,
		( (struct multiboot_tag_apm *)tag)->dseg,
		( (struct multiboot_tag_apm *)tag)->flags,
		( (struct multiboot_tag_apm *)tag)->cseg_len,
		( (struct multiboot_tag_apm *)tag)->cseg_16_len,
		( (struct multiboot_tag_apm *)tag)->dseg_len);
	return;
}

void print_MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR(multiboot_tag_t * tag) {
	printk_info("Image load base physical address type 0x%X, size 0x%X, addr 0x%X.\n",
		( (struct multiboot_tag_load_base_addr *)tag)->type,
		( (struct multiboot_tag_load_base_addr *)tag)->size,
		( (struct multiboot_tag_load_base_addr *)tag)->load_base_addr);
	return;
}

bool is_multiboot2_header(ptr_t magic, ptr_t addr) {
	if(magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printk_err("Invalid magic number: %X\n", (unsigned)magic);
		return false;
	}
	if(addr & 7) {
		printk_err("Unaligned addr: 0x%08x\n", addr);
		return false;
	}
	return true;
}

// 处理 multiboot 信息
void multiboot2_init(ptr_t magic, ptr_t addr) {
	// Am I booted by a Multiboot-compliant boot loader?
	is_multiboot2_header(magic, addr);
	uint32_t size = *(uint32_t *)addr;
	// addr+0 保存大小，下一字节开始为 tag 信息
	printk_info("Announced mbi size 0x%X\n", size);
	ptr_t tag_addr = (ptr_t)addr + 8;
	multiboot_tag_t * tag;
	tag = (multiboot_tag_t *)tag_addr;
	printk("tag type: %X\n", tag->type);
	printk("tag size: %X\n", tag->size);
	for(tag = (multiboot_tag_t *)tag_addr ;
	    tag->type != MULTIBOOT_TAG_TYPE_END ;
	    // (tag 低八位) + (tag->size 八位对齐)
	    tag = (multiboot_tag_t *)( (uint8_t *)tag + ( (tag->size + 7) & ~7) ) ) {
		// printk_info ("Tag 0x%X, Size 0x%X\n", tag->type, tag->size);
		switch(tag->type) {
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
			    break;
		    case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: {
			    // print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(tag);
			    // printk("!!!!!!!!!!!!!!!!!!!\n");
			    // 获取 shdr 的地址：tag 的地址加上 multiboot_tag_elf_sections 结构体大小即是第一项符号表
			    // BUG !!!
			    // 获取到的 size 和 name 均正确，然而 symtab 的成员全为 0x00
			    // TODO
			    // Elf64_Shdr * shdr = (Elf64_Shdr*)( (uint32_t)tag + sizeof( struct multiboot_tag_elf_sections ) );
			    // uint32_t shstrtab = shdr[( (struct multiboot_tag_elf_sections*)tag )->shndx].sh_addr;
			    // printk("shstrtabsz: 0x%X\n", shdr[((struct multiboot_tag_elf_sections*)tag)->shndx].sh_size); // correct
			    // printk("shstrtab addr: 0x%X\n", shstrtab);
			    // for (uint32_t i = 0; i < ( (struct multiboot_tag_elf_sections*)tag )->num; i++) {
			    // 	const char *name = (const char *)(shstrtab + shdr[i].sh_name);
			    // 	// printk(" sh_name: %s ", name); // correct
			    // 	// printk("shaddr: 0x%X\t", shdr[i].sh_addr); // correct
			    // 	// 在 GRUB 提供的 multiboot 信息中寻找内核 ELF 格式所提取的字符串表和符号表
			    // 	if (strcmp(name, ".strtab") == 0) {
			    // 		kernel_elf.strtab = (const char *)shdr[i].sh_addr;
			    // 		// printk("strtab: 0x%X\n", kernel_elf.strtab);
			    // 		// printk("strtab: %s\n", kernel_elf.strtab);
			    // 		kernel_elf.strtabsz = shdr[i].sh_size;
			    // 		// printk("strtabsz: 0x%X\n", kernel_elf.strtabsz); // correct
			    // 	}
			    // 	if (strcmp(name, ".symtab") == 0) {
			    // 		kernel_elf.symtab = (Elf64_Sym *)(shdr[i].sh_addr);
			    // 		// printk("symtab addr: 0x%X\n", kernel_elf.symtab);
			    // 		kernel_elf.symtabsz = shdr[i].sh_size;
			    // 		// printk("symtab->: 0x%X\n", (kernel_elf.symtab->st_value));
			    // 		// printk("symtabsz: 0x%X\n", kernel_elf.symtabsz); // correct
			    // 	}
			    // }
			    // // printk("!!!!!!!!!!!!!!!!!!!\n");
		    }
		    break;
		    case MULTIBOOT_TAG_TYPE_APM: {
			    // print_MULTIBOOT_TAG_TYPE_APM(tag);
		    }
		    break;
		    case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: {
			    // print_MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR(tag);
		    }
		    break;
		}
	}
	// tag = (multiboot_tag_t *) ((uint8_t *) tag + ((tag->size + 7) & ~7));
	// printk_info ("Total mbi size 0x%X\n", (unsigned) tag - addr);
	// printk_info("multiboot2_init\n");
	return;
}

#ifdef __cplusplus
}
#endif
