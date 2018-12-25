
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
// multiboot.h for MRNIU/SimpleKernel.


/**
 * 启动后，在32位内核进入点，机器状态如下：
 *   1. CS 指向基地址为 0x00000000，限长为4G – 1的代码段描述符。
 *   2. DS，SS，ES，FS 和 GS 指向基地址为0x00000000，限长为4G – 1的数据段描述符。
 *   3. A20 地址线已经打开。
 *   4. 页机制被禁止。
 *   5. 中断被禁止。
 *   6. EAX = 0x2BADB002
 *   7. 系统信息和启动信息块的线性地址保存在 EBX中（相当于一个指针）。
 *      以下即为这个信息块的结构
 */



// This is the source code in the file multiboot.h:

/* multiboot.h - Multiboot header file. */
/* Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _MULTIBOOT_H_
#define _MULTIBOOT_H_

#include "stdint.h"

/* How many bytes from the start of the file we search for the header. */
#define MULTIBOOT_SEARCH                        8192
#define MULTIBOOT_HEADER_ALIGN                  4

/* The magic field should contain this. */
#define MULTIBOOT_HEADER_MAGIC                  0x1BADB002

/* This should be in %eax. */
#define MULTIBOOT_BOOTLOADER_MAGIC              0x2BADB002

/* Alignment of multiboot modules. */
#define MULTIBOOT_MOD_ALIGN                     0x00001000

/* Alignment of the multiboot info structure. */
#define MULTIBOOT_INFO_ALIGN                    0x00000004

/* Flags set in the 'flags' member of the multiboot header. */

/* Align all boot modules on i386 page (4KB) boundaries. */
#define MULTIBOOT_PAGE_ALIGN                    0x00000001

/* Must pass memory information to OS. */
#define MULTIBOOT_MEMORY_INFO                   0x00000002

/* Must pass video information to OS. */
#define MULTIBOOT_VIDEO_MODE                    0x00000004

/* This flag indicates the use of the address fields in the header. */
#define MULTIBOOT_AOUT_KLUDGE                   0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure. */

/* is there basic lower/upper memory information? */
#define MULTIBOOT_INFO_MEMORY                   0x00000001
/* is there a boot device set? */
#define MULTIBOOT_INFO_BOOTDEV                  0x00000002
/* is the command-line defined? */
#define MULTIBOOT_INFO_CMDLINE                  0x00000004
/* are there modules to do something with? */
#define MULTIBOOT_INFO_MODS                     0x00000008

/* These next two are mutually exclusive */

/* is there a symbol table loaded? */
#define MULTIBOOT_INFO_AOUT_SYMS                0x00000010
/* is there an ELF section header table? */
#define MULTIBOOT_INFO_ELF_SHDR                 0X00000020

/* is there a full memory map? */
#define MULTIBOOT_INFO_MEM_MAP                  0x00000040

/* Is there drive info? */
#define MULTIBOOT_INFO_DRIVE_INFO               0x00000080

/* Is there a config table? */
#define MULTIBOOT_INFO_CONFIG_TABLE             0x00000100

/* Is there a boot loader name? */
#define MULTIBOOT_INFO_BOOT_LOADER_NAME         0x00000200

/* Is there a APM table? */
#define MULTIBOOT_INFO_APM_TABLE                0x00000400

/* Is there video information? */
#define MULTIBOOT_INFO_VBE_INFO                 0x00000800
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO         0x00001000


struct multiboot_header {
		/* Must be MULTIBOOT_MAGIC - see above. */
		uint32_t magic;

		/* Feature flags. */
		uint32_t flags;

		/* The above fields plus this one must equal 0 mod 2^32. */
		uint32_t checksum;

		/* These are only valid if MULTIBOOT_AOUT_KLUDGE is set. */
		uint32_t header_addr;
		uint32_t load_addr;
		uint32_t load_end_addr;
		uint32_t bss_end_addr;
		uint32_t entry_addr;

		/* These are only valid if MULTIBOOT_VIDEO_MODE is set. */
		uint32_t mode_type;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
};

/* The symbol table for a.out. */
typedef
  struct multiboot_aout_symbol_table {
		uint32_t tabsize;
		uint32_t strsize;
		uint32_t addr;
		uint32_t reserved;
} multiboot_aout_symbol_table_t;


/* The section header table for ELF. */
// ELF 格式内核映像的 section 头表。
// 包括每项的大小、一共有几项以及作为名字索引的字符串表。
typedef
  struct multiboot_elf_section_header_table {
		uint32_t num;
		uint32_t size;
		uint32_t addr;
		uint32_t shndx;
} multiboot_elf_section_header_table_t;

typedef
  struct multiboot_info {
		/* Multiboot info version number */
		uint32_t flags;

		/* Available memory from BIOS */
		/*
		 * 从 BIOS 获知的可用内存
		 *
		 * mem_lower 和 mem_upper 分别指出了低端和高端内存的大小，单位是K。
		 * 低端内存的首地址是 0 ，高端内存的首地址是 1M 。
		 * 低端内存的最大可能值是 640K
		 * 高端内存的最大可能值是最大值减去 1M 。但并不保证是这个值。
		 */
		uint32_t mem_lower;
		uint32_t mem_upper;

		/* "root" partition */
		// 指出引导程序从哪个BIOS磁盘设备载入的OS映像
		uint32_t boot_device;

		/* Kernel command line */
		// 内核命令行
		uint32_t cmdline;

		/* Boot-Module list */
		// boot 模块列表
		uint32_t mods_count;
		uint32_t mods_addr;

		union {
				multiboot_aout_symbol_table_t aout_sym;
				multiboot_elf_section_header_table_t elf_sec;
		} u;

		/* Memory Mapping buffer */
		/**
		 * 以下两项指出保存由 BIOS 提供的内存分布的缓冲区的地址和长度
		 * mmap_addr 是缓冲区的地址， mmap_length 是缓冲区的总大小
		 * 缓冲区由一个或者多个下面的 mmap_entry_t 组成
		 */
		uint32_t mmap_length;
		uint32_t mmap_addr;

		/* Drive Info buffer */
		uint32_t drives_length; // 指出第一个驱动器这个结构的大小
		uint32_t drives_addr; // 指出第一个驱动器结构的物理地址

		/* ROM configuration table */
		// ROM 配置表
		uint32_t config_table;

		/* Boot Loader Name */
		// boot loader 的名字
		uint32_t boot_loader_name;

		/* APM table */
		// APM 表
		uint32_t apm_table;

		/* Video */
		uint32_t vbe_control_info;
		uint32_t vbe_mode_info;
		uint16_t vbe_mode;
		uint16_t vbe_interface_seg;
		uint16_t vbe_interface_off;
		uint16_t vbe_interface_len;

		uint64_t framebuffer_addr;
		uint32_t framebuffer_pitch;
		uint32_t framebuffer_width;
		uint32_t framebuffer_height;
		uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED      0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB          1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
		uint8_t framebuffer_type;
		union {
				struct {
						uint32_t framebuffer_palette_addr;
						uint16_t framebuffer_palette_num_colors;
				};
				struct {
						uint8_t framebuffer_red_field_position;
						uint8_t framebuffer_red_mask_size;
						uint8_t framebuffer_green_field_position;
						uint8_t framebuffer_green_mask_size;
						uint8_t framebuffer_blue_field_position;
						uint8_t framebuffer_blue_mask_size;
				};
		};
} multiboot_info_t;


struct multiboot_color {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
};


/*
 * size 是相关结构的大小，单位是字节，它可能大于最小值 20
 * base_addr_low 是启动地址的低 32 位，base_addr_high 是高 32 位，启动地址总共有 64 位
 * length_low 是内存区域大小的低 32 位，length_high 是内存区域大小的高 32 位，总共是 64 位
 * type 是相应地址区间的类型，1 代表可用 RAM，所有其它的值代表保留区域
 */
typedef
  struct multiboot_mmap_entry {
		uint32_t size;  // size 是不含 size 自身变量的大小
		uint64_t addr;
		uint64_t len;
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
		uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

typedef
  struct multiboot_mod_list {
		/* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
		uint32_t mod_start;
		uint32_t mod_end;

		/* Module command line */
		uint32_t cmdline;

		/* padding to take it to 16 bytes (must be zero) */
		uint32_t pad;
} multiboot_module_list_t;


/* APM BIOS info. */
struct multiboot_apm_info {
		uint16_t version;
		uint16_t cseg;
		uint32_t offset;
		uint16_t cseg_16;
		uint16_t dseg;
		uint16_t flags;
		uint16_t cseg_len;
		uint16_t cseg_16_len;
		uint16_t dseg_len;
};

extern multiboot_info_t * glb_mboot_ptr;


#endif /* ! MULTIBOOT_HEADER */
