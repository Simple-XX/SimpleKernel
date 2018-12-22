
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
// debug.c for MRNIU/SimpleKernel.

#include "debug.h"
#include "cpu.hpp"
#include "multiboot.h"
#include "stdio.h"

/* Am I booted by a Multiboot-compliant boot loader? */
bool is_magic(uint32_t magic){
		if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
				printk ("Invalid magic number: 0x%x\n", (unsigned) magic);
				return false;
		}
		return true;
}

/* Print out the flags. */
void show_multiboot_flags(){
		printk("flags = 0x%x\n", (unsigned) glb_mboot_ptr->flags);
		return;
}

/* Are mem_* valid? */
bool is_mem_vaild(){
		if (CHECK_FLAG (glb_mboot_ptr->flags, 0)) {
				printk ("mem_lower = %uKB, mem_upper = %uKB\n",
				        (unsigned) glb_mboot_ptr->mem_lower, (unsigned) glb_mboot_ptr->mem_upper);
		}
}

/* Is boot_device valid? */
bool is_boot_device_vaild(){
		if (CHECK_FLAG (glb_mboot_ptr->flags, 1)) {
				printk ("boot_device = 0x%x\n", (unsigned) glb_mboot_ptr->boot_device);
				return false;
		}
		return true;
}


/* Is the command line passed? */
bool is_command_line_passed(){
		if (CHECK_FLAG (glb_mboot_ptr->flags, 2)) {
				printk ("cmdline = %s\n", (char *) glb_mboot_ptr->cmdline);
				return false;
		}
		return true;
}

/* Are mods_* valid? */
bool is_mods_valid(){
		if (CHECK_FLAG (glb_mboot_ptr->flags, 3)) {
				multiboot_module_list_t *mod;
				int i;

				printk ("mods_count = %d, mods_addr = 0x%x\n",
				        (int) glb_mboot_ptr->mods_count, (int) glb_mboot_ptr->mods_addr);
				for (i = 0, mod = (multiboot_module_list_t *) glb_mboot_ptr->mods_addr;
				     i < glb_mboot_ptr->mods_count;
				     i++, mod++)
						printk (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
						        (unsigned) mod->mod_start,
						        (unsigned) mod->mod_end,
						        (char *) mod->cmdline);
				return false;
		}
		return true;
}

/* Bits 4 and 5 are mutually exclusive! */
bool is_bits4_bits5_mutually_exclusive(){
		if (CHECK_FLAG (glb_mboot_ptr->flags, 4) && CHECK_FLAG (glb_mboot_ptr->flags, 5)) {
				printk ("Both bits 4 and 5 are set.\n");
				return true;
		}
		return false;
}


/* Is the symbol table of a.out valid? */
bool is_symbol_table_aout_valid(){
		if (CHECK_FLAG (glb_mboot_ptr->flags, 4)) {
				multiboot_aout_symbol_table_t *multiboot_aout_sym = &(glb_mboot_ptr->u.aout_sym);
				printk ("multiboot_aout_symbol_table: tabsize = 0x%0x, "
				        "strsize = 0x%x, addr = 0x%x\n",
				        (unsigned) multiboot_aout_sym->tabsize,
				        (unsigned) multiboot_aout_sym->strsize,
				        (unsigned) multiboot_aout_sym->addr);
				return true;
		}
		return false;
}


/* Is the section header table of ELF valid? */
bool is_section_header_table_elf_valid(){
		if (CHECK_FLAG (glb_mboot_ptr->flags, 5)) {
				multiboot_elf_section_header_table_t *multiboot_elf_sec = &(glb_mboot_ptr->u.elf_sec);
				printk ("multiboot_elf_sec: num = %u, size = 0x%x,"
				        " addr = 0x%x, shndx = 0x%x\n",
				        (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
				        (unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
				return true;
		}
		return false;
}

/* Are mmap_* valid? */
bool is_mmap_valid(){
		if (CHECK_FLAG (glb_mboot_ptr->flags, 6)) {
				multiboot_mmap_entry_t *mmap;
				printk ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
				        (unsigned) glb_mboot_ptr->mmap_addr, (unsigned) glb_mboot_ptr->mmap_length);
				for (mmap = (multiboot_mmap_entry_t *) glb_mboot_ptr->mmap_addr;
				     (uint64_t) mmap < glb_mboot_ptr->mmap_addr + glb_mboot_ptr->mmap_length;
				     mmap = (multiboot_mmap_entry_t *) ((uint64_t) mmap + mmap->size + sizeof (mmap->size))
				     )
						printk (" size = 0x%x, base_addr = 0x%x%08x,"
						        " length = 0x%x%08x, type = 0x%x\n",
						        (unsigned) mmap->size,
						        (unsigned) (mmap->addr >> 32),
						        (unsigned) (mmap->addr & 0xffffffff),
						        (unsigned) (mmap->len >> 32),
						        (unsigned) (mmap->len & 0xffffffff),
						        (unsigned) mmap->type);
				return true;
		}
		return false;
}

/* Draw diagonal blue line. */
void draw_blue_line(){
		if (CHECK_FLAG (glb_mboot_ptr->flags, 12)) {
				uint32_t color;
				unsigned i;
				void *fb = (void *) (uint64_t) glb_mboot_ptr->framebuffer_addr;

				switch (glb_mboot_ptr->framebuffer_type) {
				case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED: {
						unsigned best_distance, distance;
						struct multiboot_color *palette;

						palette = (struct multiboot_color *) glb_mboot_ptr->framebuffer_palette_addr;

						color = 0;
						best_distance = 4*256*256;

						for (i = 0; i < glb_mboot_ptr->framebuffer_palette_num_colors; i++) {
								distance = (0xff - palette[i].blue) * (0xff - palette[i].blue)
								           + palette[i].red * palette[i].red
								           + palette[i].green * palette[i].green;
								if (distance < best_distance) {
										color = i;
										best_distance = distance;
								}
						}
				}
				break;

				case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
						color = ((1 << glb_mboot_ptr->framebuffer_blue_mask_size) - 1)
						        << glb_mboot_ptr->framebuffer_blue_field_position;
						break;

				case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
						color = '\\' | 0x0100;
						break;

				default:
						color = 0xffffffff;
						break;
				}
				for (i = 0;
				     i < glb_mboot_ptr->framebuffer_width
				     && i < glb_mboot_ptr->framebuffer_height;
				     i++) {
						switch (glb_mboot_ptr->framebuffer_bpp) {
						case 8: {
								uint8_t *pixel = fb + glb_mboot_ptr->framebuffer_pitch * i + i;
								*pixel = color;
						}
						break;
						case 15:
						case 16: {
								uint16_t *pixel
								  = fb + glb_mboot_ptr->framebuffer_pitch * i + 2 * i;
								*pixel = color;
						}
						break;
						case 24: {
								uint32_t *pixel
								  = fb + glb_mboot_ptr->framebuffer_pitch * i + 3 * i;
								*pixel = (color & 0xffffff) | (*pixel & 0xff000000);
						}
						break;

						case 32: {
								uint32_t *pixel
								  = fb + glb_mboot_ptr->framebuffer_pitch * i + 4 * i;
								*pixel = color;
						}
						break;
						}
				}
		}
}

void debug_init(uint32_t magic, uint32_t addr) {
		is_magic(magic);
		show_multiboot_flags();
		is_mem_vaild();
		is_boot_device_vaild();
		is_command_line_passed();
		is_mods_valid();
		is_bits4_bits5_mutually_exclusive();
		is_symbol_table_aout_valid();
		is_section_header_table_elf_valid();
		is_mmap_valid();
		draw_blue_line();

		return;
}
