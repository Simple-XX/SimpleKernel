
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// kernel.c for MRNIU/SimpleKernel.

#include "kernel.h"
#include "assert.h"

extern void tmp();

/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

// 内核入口
void kernel_main(uint64_t magic, uint64_t addr);
void kernel_main(uint64_t magic, uint64_t addr){
		// // 系统初始化
		// debug_init();
		// terminal_init();       // 控制台初始化
		// gdt_inuint64_t/ GDT 初始化
		// idt_init();       // IDT 初始化
		//
		// clock_init();       // 时钟初始化
		// keyboard_init();       // 键盘初始化
		// showinfo();
		//
		//
		// while(1);
		//
		// // printk_color(white, "\nEnd.\n");
		// return;
		printk_color(red, "0\n");
		multiboot_info_t *mbi;


		/* Am I booted by a Multiboot-compliant boot loader? */
		if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
		{
				printk ("Invalid magic number: 0x%x\n", (unsigned) magic);
				return;
		}
		printk_color(red, "1\n");

		/* Set MBI to the address of the Multiboot information structure. */
		mbi = (multiboot_info_t *) addr;

		/* Print out the flags. */
		printk ("flags = 0x%x\n", (unsigned) mbi->flags);

		/* Are mem_* valid? */
		if (CHECK_FLAG (mbi->flags, 0))
				printk ("mem_lower = %uKB, mem_upper = %uKB\n",
				        (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

		/* Is boot_device valid? */
		if (CHECK_FLAG (mbi->flags, 1))
				printk ("boot_device = 0x%x\n", (unsigned) mbi->boot_device);

		/* Is the command line passed? */
		if (CHECK_FLAG (mbi->flags, 2))
				printk ("cmdline = %s\n", (char *) mbi->cmdline);

		/* Are mods_* valid? */
		if (CHECK_FLAG (mbi->flags, 3))
		{
				multiboot_module_t *mod;
				int i;

				printk ("mods_count = %d, mods_addr = 0x%x\n",
				        (int) mbi->mods_count, (int) mbi->mods_addr);
				for (i = 0, mod = (multiboot_module_t *) mbi->mods_addr;
				     i < mbi->mods_count;
				     i++, mod++)
						printk (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
						        (unsigned) mod->mod_start,
						        (unsigned) mod->mod_end,
						        (char *) mod->cmdline);
		}

		/* Bits 4 and 5 are mutually exclusive! */
		if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
		{
				printk ("Both bits 4 and 5 are set.\n");
				return;
		}

		/* Is the symbol table of a.out valid? */
		if (CHECK_FLAG (mbi->flags, 4))
		{
				multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);

				printk ("multiboot_aout_symbol_table: tabsize = 0x%0x, "
				        "strsize = 0x%x, addr = 0x%x\n",
				        (unsigned) multiboot_aout_sym->tabsize,
				        (unsigned) multiboot_aout_sym->strsize,
				        (unsigned) multiboot_aout_sym->addr);
		}

		/* Is the section header table of ELF valid? */
		if (CHECK_FLAG (mbi->flags, 5))
		{
				multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);

				printk ("multiboot_elf_sec: num = %u, size = 0x%x,"
				        " addr = 0x%x, shndx = 0x%x\n",
				        (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
				        (unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
		}

		/* Are mmap_* valid? */
		if (CHECK_FLAG (mbi->flags, 6))
		{
				multiboot_memory_map_t *mmap;

				printk ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
				        (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
				for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
				     (uint64_t) mmap < mbi->mmap_addr + mbi->mmap_length;
				     mmap = (multiboot_memory_map_t *) ((uint64_t) mmap
				                                        + mmap->size + sizeof (mmap->size)))
						printk (" size = 0x%x, base_addr = 0x%x%08x,"
						        " length = 0x%x%08x, type = 0x%x\n",
						        (unsigned) mmap->size,
						        (unsigned) (mmap->addr >> 32),
						        (unsigned) (mmap->addr & 0xffffffff),
						        (unsigned) (mmap->len >> 32),
						        (unsigned) (mmap->len & 0xffffffff),
						        (unsigned) mmap->type);
		}

		printk_color(red, "2\n");
		/* Draw diagonal blue line. */
		if (CHECK_FLAG (mbi->flags, 12))
		{
				uint32_t color;
				unsigned i;
				void *fb = (void *) (uint64_t) mbi->framebuffer_addr;

				switch (mbi->framebuffer_type)
				{
				case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
				{
						unsigned best_distance, distance;
						struct multiboot_color *palette;

						palette = (struct multiboot_color *) mbi->framebuffer_palette_addr;

						color = 0;
						best_distance = 4*256*256;

						for (i = 0; i < mbi->framebuffer_palette_num_colors; i++)
						{
								distance = (0xff - palette[i].blue) * (0xff - palette[i].blue)
								           + palette[i].red * palette[i].red
								           + palette[i].green * palette[i].green;
								if (distance < best_distance)
								{
										color = i;
										best_distance = distance;
								}
						}
				}
				break;

				case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
						color = ((1 << mbi->framebuffer_blue_mask_size) - 1)
						        << mbi->framebuffer_blue_field_position;
						break;

				case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
						color = '\\' | 0x0100;
						break;

				default:
						color = 0xffffffff;
						break;
				}
				for (i = 0; i < mbi->framebuffer_width
				     && i < mbi->framebuffer_height; i++)
				{
						switch (mbi->framebuffer_bpp)
						{
						case 8:
						{
								uint8_t *pixel = fb + mbi->framebuffer_pitch * i + i;
								*pixel = color;
						}
						break;
						case 15:
						case 16:
						{
								uint16_t *pixel
								  = fb + mbi->framebuffer_pitch * i + 2 * i;
								*pixel = color;
						}
						break;
						case 24:
						{
								uint32_t *pixel
								  = fb + mbi->framebuffer_pitch * i + 3 * i;
								*pixel = (color & 0xffffff) | (*pixel & 0xff000000);
						}
						break;

						case 32:
						{
								uint32_t *pixel
								  = fb + mbi->framebuffer_pitch * i + 4 * i;
								*pixel = color;
						}
						break;
						}
				}
		}
		printk_color(red, "END\n");
}



/* Macros. */



/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
// void
// cmain (uint64_t magic, uint64_t addr)
// {
//   multiboot_info_t *mbi;
//
//
//   /* Am I booted by a Multiboot-compliant boot loader? */
//   if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
//   {
//     printk ("Invalid magic number: 0x%x\n", (unsigned) magic);
//     return;
//   }
//
//   /* Set MBI to the address of the Multiboot information structure. */
//   mbi = (multiboot_info_t *) addr;
//
//   /* Print out the flags. */
//   printk ("flags = 0x%x\n", (unsigned) mbi->flags);
//
//   /* Are mem_* valid? */
//   if (CHECK_FLAG (mbi->flags, 0))
//     printk ("mem_lower = %uKB, mem_upper = %uKB\n",
//             (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
//
//   /* Is boot_device valid? */
//   if (CHECK_FLAG (mbi->flags, 1))
//     printk ("boot_device = 0x%x\n", (unsigned) mbi->boot_device);
//
//   /* Is the command line passed? */
//   if (CHECK_FLAG (mbi->flags, 2))
//     printk ("cmdline = %s\n", (char *) mbi->cmdline);
//
//   /* Are mods_* valid? */
//   if (CHECK_FLAG (mbi->flags, 3))
//   {
//     multiboot_module_t *mod;
//     int i;
//
//     printk ("mods_count = %d, mods_addr = 0x%x\n",
//             (int) mbi->mods_count, (int) mbi->mods_addr);
//     for (i = 0, mod = (multiboot_module_t *) mbi->mods_addr;
//          i < mbi->mods_count;
//          i++, mod++)
//       printk (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
//               (unsigned) mod->mod_start,
//               (unsigned) mod->mod_end,
//               (char *) mod->cmdline);
//   }
//
//   /* Bits 4 and 5 are mutually exclusive! */
//   if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
//   {
//     printk ("Both bits 4 and 5 are set.\n");
//     return;
//   }
//
//   /* Is the symbol table of a.out valid? */
//   if (CHECK_FLAG (mbi->flags, 4))
//   {
//     multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mbi->u.aout_sym);
//
//     printk ("multiboot_aout_symbol_table: tabsize = 0x%0x, "
//             "strsize = 0x%x, addr = 0x%x\n",
//             (unsigned) multiboot_aout_sym->tabsize,
//             (unsigned) multiboot_aout_sym->strsize,
//             (unsigned) multiboot_aout_sym->addr);
//   }
//
//   /* Is the section header table of ELF valid? */
//   if (CHECK_FLAG (mbi->flags, 5))
//   {
//     multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mbi->u.elf_sec);
//
//     printk ("multiboot_elf_sec: num = %u, size = 0x%x,"
//             " addr = 0x%x, shndx = 0x%x\n",
//             (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
//             (unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
//   }
//
//   /* Are mmap_* valid? */
//   if (CHECK_FLAG (mbi->flags, 6))
//   {
//     multiboot_memory_map_t *mmap;
//
//     printk ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
//             (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
//     for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
//          (uint64_t) mmap < mbi->mmap_addr + mbi->mmap_length;
//          mmap = (multiboot_memory_map_t *) ((uint64_t) mmap
//                                             + mmap->size + sizeof (mmap->size)))
//       printk (" size = 0x%x, base_addr = 0x%x%08x,"
//               " length = 0x%x%08x, type = 0x%x\n",
//               (unsigned) mmap->size,
//               (unsigned) (mmap->addr >> 32),
//               (unsigned) (mmap->addr & 0xffffffff),
//               (unsigned) (mmap->len >> 32),
//               (unsigned) (mmap->len & 0xffffffff),
//               (unsigned) mmap->type);
//   }
//
//   /* Draw diagonal blue line. */
//   if (CHECK_FLAG (mbi->flags, 12))
//   {
//     uint32_t color;
//     unsigned i;
//     void *fb = (void *) (uint64_t) mbi->framebuffer_addr;
//
//     switch (mbi->framebuffer_type)
//     {
//     case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
//     {
//       unsigned best_distance, distance;
//       struct multiboot_color *palette;
//
//       palette = (struct multiboot_color *) mbi->framebuffer_palette_addr;
//
//       color = 0;
//       best_distance = 4*256*256;
//
//       for (i = 0; i < mbi->framebuffer_palette_num_colors; i++)
//       {
//         distance = (0xff - palette[i].blue) * (0xff - palette[i].blue)
//                    + palette[i].red * palette[i].red
//                    + palette[i].green * palette[i].green;
//         if (distance < best_distance)
//         {
//           color = i;
//           best_distance = distance;
//         }
//       }
//     }
//     break;
//
//     case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
//       color = ((1 << mbi->framebuffer_blue_mask_size) - 1)
//               << mbi->framebuffer_blue_field_position;
//       break;
//
//     case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
//       color = '\\' | 0x0100;
//       break;
//
//     default:
//       color = 0xffffffff;
//       break;
//     }
//     for (i = 0; i < mbi->framebuffer_width
//          && i < mbi->framebuffer_height; i++)
//     {
//       switch (mbi->framebuffer_bpp)
//       {
//       case 8:
//       {
//         uint8_t *pixel = fb + mbi->framebuffer_pitch * i + i;
//         *pixel = color;
//       }
//       break;
//       case 15:
//       case 16:
//       {
//         uint16_t *pixel
//           = fb + mbi->framebuffer_pitch * i + 2 * i;
//         *pixel = color;
//       }
//       break;
//       case 24:
//       {
//         uint32_t *pixel
//           = fb + mbi->framebuffer_pitch * i + 3 * i;
//         *pixel = (color & 0xffffff) | (*pixel & 0xff000000);
//       }
//       break;
//
//       case 32:
//       {
//         uint32_t *pixel
//           = fb + mbi->framebuffer_pitch * i + 4 * i;
//         *pixel = color;
//       }
//       break;
//       }
//     }
//   }
//
// }
