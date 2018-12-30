
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


/*  kernel.c - the C part of the kernel */
/*  Copyright (C) 1999, 2010  Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/*  Macros. */

/*  Some screen stuff. */
/*  The number of columns. */
#define COLUMNS                 80
/*  The number of lines. */
#define LINES                   24
/*  The attribute of an character. */
#define ATTRIBUTE               7
/*  The video memory address. */
#define VIDEO                   0xB8000

/*  Variables. */
/*  Save the X position. */
static int xpos;
/*  Save the Y position. */
static int ypos;
/*  Point to the video memory. */
static volatile unsigned char *video;

/*  Forward declarations. */
void debug233 (unsigned long magic, unsigned long addr);

/*  Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
debug233 (unsigned long magic, unsigned long addr)
{
		struct multiboot_tag *tag;
		unsigned size;

		printk("debug233\n");
		/*  Am I booted by a Multiboot-compliant boot loader? */
		if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
		{
				printk ("Invalid magic number: 0x%x\n", (unsigned) magic);
				return;
		}

		if (addr & 7)
		{
				printk ("Unaligned mbi: 0x%x\n", addr);
				return;
		}

		size = *(unsigned *) addr;
		printk ("Announced mbi size 0x%x\n", size);


		tag = (struct multiboot_tag *) (addr + 8);
		printk("tag type: %x\n", tag->type);
		printk("tag size: %x\n", tag->size);


		// for (tag = (struct multiboot_tag *) (addr + 8);
		//      tag->type != MULTIBOOT_TAG_TYPE_END;
		//      tag = (struct multiboot_tag *) ((uint8_t *) tag
		//                                      + ((tag->size + 7) & ~7)))
		// {
		//   printk ("Tag 0x%x, Size 0x%x\n", tag->type, tag->size);
		//   switch (tag->type)
		//   {
		//   case MULTIBOOT_TAG_TYPE_CMDLINE:
		//     printk ("Command line = %s\n",
		//             ((struct multiboot_tag_string *) tag)->string);
		//     break;
		//   case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
		//     printk ("Boot loader name = %s\n",
		//             ((struct multiboot_tag_string *) tag)->string);
		//     break;
		//   case MULTIBOOT_TAG_TYPE_MODULE:
		//     printk ("Module at 0x%x-0x%x. Command line %s\n",
		//             ((struct multiboot_tag_module *) tag)->mod_start,
		//             ((struct multiboot_tag_module *) tag)->mod_end,
		//             ((struct multiboot_tag_module *) tag)->cmdline);
		//     break;
		//   case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
		//     printk ("mem_lower = %uKB, mem_upper = %uKB\n",
		//             ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
		//             ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
		//     break;
		//   case MULTIBOOT_TAG_TYPE_BOOTDEV:
		//     printk ("Boot device 0x%x,%u,%u\n",
		//             ((struct multiboot_tag_bootdev *) tag)->biosdev,
		//             ((struct multiboot_tag_bootdev *) tag)->slice,
		//             ((struct multiboot_tag_bootdev *) tag)->part);
		//     break;
		//   case MULTIBOOT_TAG_TYPE_MMAP:
		//   {
		//     multiboot_memory_map_t *mmap;
		//
		//     printk ("mmap\n");
		//
		//     for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
		//          (uint8_t *) mmap
		//          < (uint8_t *) tag + tag->size;
		//          mmap = (multiboot_memory_map_t *)
		//                 ((unsigned long) mmap
		//                  + ((struct multiboot_tag_mmap *) tag)->entry_size))
		//       printk (" base_addr = 0x%x%x,"
		//               " length = 0x%x%x, type = 0x%x\n",
		//               (unsigned) (mmap->addr >> 32),
		//               (unsigned) (mmap->addr & 0xffffffff),
		//               (unsigned) (mmap->len >> 32),
		//               (unsigned) (mmap->len & 0xffffffff),
		//               (unsigned) mmap->type);
		//   }
		//   break;
		//   case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
		//   {
		//     uint32_t color;
		//     unsigned i;
		//     struct multiboot_tag_framebuffer *tagfb
		//       = (struct multiboot_tag_framebuffer *) tag;
		//     void *fb = (void *) (unsigned long) tagfb->common.framebuffer_addr;
		//
		//     switch (tagfb->common.framebuffer_type)
		//     {
		//     case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
		//     {
		//       unsigned best_distance, distance;
		//       struct multiboot_color *palette;
		//
		//       palette = tagfb->framebuffer_palette;
		//
		//       color = 0;
		//       best_distance = 4*256*256;
		//
		//       for (i = 0; i < tagfb->framebuffer_palette_num_colors; i++)
		//       {
		//         distance = (0xff - palette[i].blue)
		//                    * (0xff - palette[i].blue)
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
		//       color = ((1 << tagfb->framebuffer_blue_mask_size) - 1)
		//               << tagfb->framebuffer_blue_field_position;
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
		//
		//     for (i = 0; i < tagfb->common.framebuffer_width
		//          && i < tagfb->common.framebuffer_height; i++)
		//     {
		//       switch (tagfb->common.framebuffer_bpp)
		//       {
		//       case 8:
		//       {
		//         uint8_t *pixel = fb
		//                          + tagfb->common.framebuffer_pitch * i + i;
		//         *pixel = color;
		//       }
		//       break;
		//       case 15:
		//       case 16:
		//       {
		//         uint16_t *pixel
		//           = fb + tagfb->common.framebuffer_pitch * i + 2 * i;
		//         *pixel = color;
		//       }
		//       break;
		//       case 24:
		//       {
		//         uint32_t *pixel
		//           = fb + tagfb->common.framebuffer_pitch * i + 3 * i;
		//         *pixel = (color & 0xffffff) | (*pixel & 0xff000000);
		//       }
		//       break;
		//
		//       case 32:
		//       {
		//         uint32_t *pixel
		//           = fb + tagfb->common.framebuffer_pitch * i + 4 * i;
		//         *pixel = color;
		//       }
		//       break;
		//       }
		//     }
		//     break;
		//   }
		//
		//   }
		// }
		tag = (struct multiboot_tag *) ((uint8_t *) tag
		                                + ((tag->size + 7) & ~7));
		printk ("Total mbi size 0x%x\n", (unsigned) tag - addr);
}



void debug_init(uint64_t magic, unsigned long addr) {
		debug233(magic, addr);

		// 从 GRUB 提供的信息中获取到内核符号表和代码地址信息

		return;
}
