
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
// multiboot2.cpp for Simple-XX/SimpleKernel.

#include "assert.h"
#include "stdio.h"
#include "multiboot2.h"

namespace MULTIBOOT2 {

    void print_MULTIBOOT_TAG_TYPE_CMDLINE(multiboot_tag_t *tag) {
        printf("Command line = %s\n",
               ((struct multiboot_tag_string *)tag)->string);
        return;
    }

    void print_MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME(multiboot_tag_t *tag) {
        printf("Boot loader name = %s\n",
               ((struct multiboot_tag_string *)tag)->string);
        return;
    }

    void print_MULTIBOOT_TAG_TYPE_MODULE(multiboot_tag_t *tag) {
        printf("Module at 0x%X-0x%X. Command line %s\n",
               ((struct multiboot_tag_module *)tag)->mod_start,
               ((struct multiboot_tag_module *)tag)->mod_end,
               ((struct multiboot_tag_module *)tag)->cmdline);
        return;
    }

    void print_MULTIBOOT_TAG_TYPE_BASIC_MEMINFO(multiboot_tag_t *tag) {
        printf("mem_lower = %uKB, mem_upper = %uKB\n",
               ((struct multiboot_tag_basic_meminfo *)tag)->mem_lower,
               ((struct multiboot_tag_basic_meminfo *)tag)->mem_upper);
        return;
    }

    void print_MULTIBOOT_TAG_TYPE_BOOTDEV(multiboot_tag_t *tag) {
        printf("Boot device 0x%X,%u,%u\n",
               ((struct multiboot_tag_bootdev *)tag)->biosdev,
               ((struct multiboot_tag_bootdev *)tag)->slice,
               ((struct multiboot_tag_bootdev *)tag)->part);
        return;
    }

    void print_MULTIBOOT_TAG_TYPE_MMAP(multiboot_tag_t *tag) {
        multiboot_memory_map_entry_t *mmap;
        mmap = ((multiboot_tag_mmap_t *)tag)->entries;
        printf("mmap\n");
        for (; (uint8_t *)mmap < (uint8_t *)tag + tag->size;
             mmap = (multiboot_memory_map_entry_t
                         *)((uintptr_t)mmap +
                            ((multiboot_tag_mmap_t *)tag)->entry_size)) {
            printf("base_addr = 0x%X%X, length = 0x%X%X, type = 0x%X\n",
                   (unsigned)(mmap->addr >> 32),        // high
                   (unsigned)(mmap->addr & 0xffffffff), // low
                   (unsigned)(mmap->len >> 32),         // high
                   (unsigned)(mmap->len & 0xffffffff),  // low
                   (unsigned)mmap->type);
        }
        return;
    }

    void print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(multiboot_tag_t *tag
                                               __attribute__((unused))) {
#if DEBUG_LOCAL
        printf(
            "Elf type 0x%X, Size 0x%X, num 0x%X, entsize 0x%X, shndx 0x%X.\n",
            ((struct multiboot_tag_elf_sections *)tag)->type,
            ((struct multiboot_tag_elf_sections *)tag)->size,
            ((struct multiboot_tag_elf_sections *)tag)->num,
            ((struct multiboot_tag_elf_sections *)tag)->entsize,
            ((struct multiboot_tag_elf_sections *)tag)->shndx);
#endif
        return;
    }

    void print_MULTIBOOT_TAG_TYPE_APM(multiboot_tag_t *tag) {
        printf("APM type 0x%X, Size 0x%X, version 0x%X, cseg 0x%X, offset "
               "0x%X, cseg_16 0x%X, "
               "dseg 0x%X, flags 0x%X, cseg_len 0x%X, cseg_16_len 0x%X, "
               "dseg_len 0x%X\n",
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

    void print_MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR(multiboot_tag_t *tag) {
        printf("Image load base physical address type 0x%X, size 0x%X, addr "
               "0x%X.\n",
               ((struct multiboot_tag_load_base_addr *)tag)->type,
               ((struct multiboot_tag_load_base_addr *)tag)->size,
               ((struct multiboot_tag_load_base_addr *)tag)->load_base_addr);
        return;
    }

    multiboot_memory_map_entry_t *mmap_entries;
    multiboot_mmap_tag_t *        mmap_tag;

    // 处理 multiboot 信息
    void multiboot2_init(uint32_t magic, void *addr) {
        // Am I booted by a Multiboot-compliant boot loader?
        assert(magic == MULTIBOOT2_BOOTLOADER_MAGIC);
        assert((reinterpret_cast<uintptr_t>(addr) & 7) == 0);
        // uint32_t size = *(uint32_t *)addr;
        // addr+0 保存大小，下一字节开始为 tag 信息
        // printf("Announced mbi size 0x%X\n", size);
        void *tag_addr =
            reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(addr) + 8);
        multiboot_tag_t *tag = (multiboot_tag_t *)tag_addr;
        // printk("tag type: %X\n", tag->type);
        // printk("tag size: %X\n", tag->size);
        for (tag = (multiboot_tag_t *)tag_addr;
             tag->type != MULTIBOOT_TAG_TYPE_END;
             // (tag 低八位) + (tag->size 八位对齐)
             tag =
                 (multiboot_tag_t *)((uint8_t *)tag + ((tag->size + 7) & ~7))) {
            // printf ("Tag 0x%X, Size 0x%X\n", tag->type, tag->size);
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
                    mmap_entries = ((multiboot_tag_mmap_t *)tag)->entries;
                    mmap_tag     = tag;
                    // print_MULTIBOOT_TAG_TYPE_MMAP(tag);
                    break;
                case MULTIBOOT_TAG_TYPE_ELF_SECTIONS: {
                    // print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(tag);
                    // printk("!!!!!!!!!!!!!!!!!!!\n");
                    // 获取 shdr 的地址：tag 的地址加上
                    // multiboot_tag_elf_sections 结构体大小即是第一项符号表 BUG
                    // !!! 获取到的 size 和 name 均正确，然而 symtab 的成员全为
                    // 0x00
                    // TODO
                    // Elf64_Shdr * shdr = (Elf64_Shdr*)( (uint32_t)tag +
                    // sizeof( struct multiboot_tag_elf_sections ) ); uint32_t
                    // shstrtab = shdr[( (struct multiboot_tag_elf_sections*)tag
                    // )->shndx].sh_addr; printk("shstrtabsz: 0x%X\n",
                    // shdr[((struct
                    // multiboot_tag_elf_sections*)tag)->shndx].sh_size); //
                    // correct printk("shstrtab addr: 0x%X\n", shstrtab); for
                    // (uint32_t i = 0; i < ( (struct
                    // multiboot_tag_elf_sections*)tag )->num; i++) { 	const
                    // char *name = (const char *)(shstrtab + shdr[i].sh_name);
                    // 	// printk(" sh_name: %s ", name); // correct
                    // 	// printk("shaddr: 0x%X\t", shdr[i].sh_addr); // correct
                    // 	// 在 GRUB 提供的 multiboot 信息中寻找内核 ELF
                    // 格式所提取的字符串表和符号表 	if (strcmp(name,
                    // ".strtab")
                    // == 0) { 		kernel_elf.strtab = (const char
                    // *)shdr[i].sh_addr;
                    // 		// printk("strtab: 0x%X\n", kernel_elf.strtab);
                    // 		// printk("strtab: %s\n", kernel_elf.strtab);
                    // 		kernel_elf.strtabsz = shdr[i].sh_size;
                    // 		// printk("strtabsz: 0x%X\n",
                    // kernel_elf.strtabsz);
                    // // correct
                    // 	}
                    // 	if (strcmp(name, ".symtab") == 0) {
                    // 		kernel_elf.symtab = (Elf64_Sym
                    // *)(shdr[i].sh_addr);
                    // 		// printk("symtab addr: 0x%X\n",
                    // kernel_elf.symtab); 		kernel_elf.symtabsz =
                    // shdr[i].sh_size;
                    // 		// printk("symtab->: 0x%X\n",
                    // (kernel_elf.symtab->st_value));
                    // 		// printk("symtabsz: 0x%X\n",
                    // kernel_elf.symtabsz);
                    // // correct
                    // 	}
                    // }
                    // // printk("!!!!!!!!!!!!!!!!!!!\n");
                } break;
                case MULTIBOOT_TAG_TYPE_APM: {
                    // print_MULTIBOOT_TAG_TYPE_APM(tag);
                } break;
                case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: {
                    // print_MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR(tag);
                } break;
            }
        }
        // tag = (multiboot_tag_t *) ((uint8_t *) tag + ((tag->size + 7) & ~7));
        // printf ("Total mbi size 0x%X\n", (unsigned) tag - addr);
        printf("multiboot2 init.\n");
        return;
    }

    int get_e820(e820map_t &_e820map) {
        for (; (uint8_t *)mmap_entries < (uint8_t *)mmap_tag + mmap_tag->size;
             mmap_entries =
                 (multiboot_memory_map_entry_t
                      *)((uintptr_t)mmap_entries +
                         ((multiboot_tag_mmap_t *)mmap_tag)->entry_size)) {
            // 如果是可用内存
            if (mmap_entries->type == MULTIBOOT_MEMORY_AVAILABLE) {
                _e820map.map[_e820map.nr_map].addr =
                    reinterpret_cast<uint8_t *>(mmap_entries->addr);
                _e820map.map[_e820map.nr_map].length = mmap_entries->len;
                _e820map.map[_e820map.nr_map].type   = mmap_entries->type;
                _e820map.nr_map++;
            }
        }
        return 0;
    }

};
