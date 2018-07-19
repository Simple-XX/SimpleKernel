
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-5.html
// multiboot.h for MRNIU/SimpleKernel.

#ifndef _MULTIBOOT_H_
#define _MULTIBOOT_H_

#include "stdint.h"

typedef
struct multiboot_t{
  uint32_t flags;     // Multiboot 的版本信息
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

  uint32_t boot_device;   // 指出引导程序从哪个BIOS磁盘设备载入的OS映像
  uint32_t cmdline;   // 内核命令行
  uint32_t mods_count;    // boot 模块列表
  uint32_t mods_addr;

  // ELF 格式内核映像的 section 头表。包括每项的大小、一共有几项以及作为名字索引的字符串。
  uint32_t num;
  uint32_t size;
  uint32_t addr;
  uint32_t shndx;

  /**
   * 以下两项指出保存由 BIOS 提供的内存分布的缓冲区的地址和长度
   * mmap_addr 是缓冲区的地址， mmap_length 是缓冲区的总大小
   * 缓冲区由一个或者多个下面的 mmap_entry_t 组成
   */
  uint32_t mmap_length;
  uint32_t mmap_addr;

  uint32_t drives_length;     // 指出第一个驱动器这个结构的大小
  uint32_t drives_addr;       // 指出第一个驱动器结构的物理地址
  uint32_t config_table;      // ROM 配置表
  uint32_t boot_loader_name;  // boot loader 的名字
  uint32_t apm_table;             // APM 表
  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint32_t vbe_mode;
  uint32_t vbe_interface_seg;
  uint32_t vbe_interface_off;
  uint32_t vbe_interface_len;
}__attribute__((packed)) multiboot_t;

/*
 * size 是相关结构的大小，单位是字节，它可能大于最小值 20
 * base_addr_low 是启动地址的低 32 位，base_addr_high 是高 32 位，启动地址总共有 64 位
 * length_low 是内存区域大小的低 32 位，length_high 是内存区域大小的高 32 位，总共是 64 位
 * type 是相应地址区间的类型，1 代表可用 RAM，所有其它的值代表保留区域
 */
typedef
struct mmap_entry_t{
  uint32_t size;      // size 是不含 size 自身变量的大小
  uint32_t base_addr_low;
  uint32_t base_addr_high;
  uint32_t length_low;
  uint32_t length_high;
  uint32_t type;
}__attribute__((packed)) mmap_entry_t;

// 声明全局的 multiboot_t * 指针
extern multiboot_t *glb_mboot_ptr;

#endif
