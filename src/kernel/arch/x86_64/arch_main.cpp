
/**
 * @file arch_main.cpp
 * @brief arch_main cpp
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#include <elf.h>

#include "cpu.hpp"
#include "cstdio"
#include "cstring"
#include "kernel.h"
#include "kernel_elf.hpp"
#include "kernel_log.hpp"

// printf_bare_metal 基本输出实现
/// @note 这里要注意，保证在 serial 初始化之前不能使用 printf
/// 函数，否则会有全局对象依赖问题
static cpu::Serial kSerial(cpu::kCom1);
extern "C" void _putchar(char character) { kSerial.Write(character); }

static void Fillrect(uint8_t *vram, uint32_t pitch, uint8_t r, uint8_t g,
                     uint8_t b, uint32_t w, uint32_t h) {
  static const int kPixelWidth = sizeof(uint32_t);
  for (uint32_t i = 0; i < w; i++) {
    for (uint32_t j = 0; j < h; j++) {
      vram[j * kPixelWidth] = r;
      vram[j * kPixelWidth + 1] = g;
      vram[j * kPixelWidth + 2] = b;
    }
    vram += pitch;
  }
}

// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define SEG_DESCTYPE(x) \
  ((x) << 0x04)  // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x) ((x) << 0x07)  // Present
#define SEG_SAVL(x) ((x) << 0x0C)  // Available for system use
#define SEG_LONG(x) ((x) << 0x0D)  // Long mode
#define SEG_SIZE(x) ((x) << 0x0E)  // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x) \
  ((x) << 0x0F)  // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x) (((x) & 0x03) << 0x05)  // Set privilege level (0 - 3)

#define SEG_DATA_RD 0x00         // Read-Only
#define SEG_DATA_RDA 0x01        // Read-Only, accessed
#define SEG_DATA_RDWR 0x02       // Read/Write
#define SEG_DATA_RDWRA 0x03      // Read/Write, accessed
#define SEG_DATA_RDEXPD 0x04     // Read-Only, expand-down
#define SEG_DATA_RDEXPDA 0x05    // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD 0x06   // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07  // Read/Write, expand-down, accessed
#define SEG_CODE_EX 0x08         // Execute-Only
#define SEG_CODE_EXA 0x09        // Execute-Only, accessed
#define SEG_CODE_EXRD 0x0A       // Execute/Read
#define SEG_CODE_EXRDA 0x0B      // Execute/Read, accessed
#define SEG_CODE_EXC 0x0C        // Execute-Only, conforming
#define SEG_CODE_EXCA 0x0D       // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC 0x0E      // Execute/Read, conforming
#define SEG_CODE_EXRDCA 0x0F     // Execute/Read, conforming, accessed

#define GDT_CODE_PL0                                                        \
  SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) | \
      SEG_GRAN(1) | SEG_PRIV(0) | SEG_CODE_EXRD

#define GDT_DATA_PL0                                                        \
  SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) | \
      SEG_GRAN(1) | SEG_PRIV(0) | SEG_DATA_RDWR

#define GDT_CODE_PL3                                                        \
  SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) | \
      SEG_GRAN(1) | SEG_PRIV(3) | SEG_CODE_EXRD

#define GDT_DATA_PL3                                                        \
  SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | SEG_LONG(0) | SEG_SIZE(1) | \
      SEG_GRAN(1) | SEG_PRIV(3) | SEG_DATA_RDWR

void create_descriptor(uint32_t base, uint32_t limit, uint16_t flag) {
  uint64_t descriptor;

  // Create the high 32 bit segment
  descriptor = limit & 0x000F0000;  // set limit bits 19:16
  descriptor |= (flag << 8) &
                0x00F0FF00;  // set type, p, dpl, s, g, d/b, l and avl fields
  descriptor |= (base >> 16) & 0x000000FF;  // set base bits 23:16
  descriptor |= base & 0xFF000000;          // set base bits 31:24

  // Shift by 32 to allow for low part of segment
  descriptor <<= 32;

  // Create the low 32 bit segment
  descriptor |= base << 16;          // set base bits 15:0
  descriptor |= limit & 0x0000FFFF;  // set limit bits 15:0

  printf("0x%.16llX\n", descriptor);
}

uint32_t ArchInit(uint32_t argc, uint8_t *argv) {
  if (argc != 1) {
    Err("argc != 1 [%d]\n", argc);
    throw;
  }

  kBasicInfo.GetInstance() = *reinterpret_cast<BasicInfo *>(argv);
  printf("kBasicInfo.framebuffer.base: 0x%p.\n",
         kBasicInfo.GetInstance().framebuffer.base);
  printf("kBasicInfo.framebuffer.size: %d.\n",
         kBasicInfo.GetInstance().framebuffer.size);
  printf("kBasicInfo.framebuffer.height: %d.\n",
         kBasicInfo.GetInstance().framebuffer.height);
  printf("kBasicInfo.framebuffer.width: %d.\n",
         kBasicInfo.GetInstance().framebuffer.width);
  printf("kBasicInfo.framebuffer.pitch: %d.\n",
         kBasicInfo.GetInstance().framebuffer.pitch);

  printf("kBasicInfo.elf_addr: 0x%X.\n", kBasicInfo.GetInstance().elf_addr);
  printf("kBasicInfo.elf_size: %d.\n", kBasicInfo.GetInstance().elf_size);

  // 在屏幕上绘制矩形
  Fillrect((uint8_t *)kBasicInfo.GetInstance().framebuffer.base,
           kBasicInfo.GetInstance().framebuffer.pitch, 255, 0, 255, 100, 100);

  for (uint32_t i = 0; i < kBasicInfo.GetInstance().memory_map_count; i++) {
    printf(
        "kBasicInfo.memory_map[%d].base_addr: 0x%p, length: 0x%X, type: %d.\n",
        i, kBasicInfo.GetInstance().memory_map[i].base_addr,
        kBasicInfo.GetInstance().memory_map[i].length,
        kBasicInfo.GetInstance().memory_map[i].type);
  }

  // 解析内核 elf 信息
  kKernelElf.GetInstance() = KernelElf(kBasicInfo.GetInstance().elf_addr,
                                       kBasicInfo.GetInstance().elf_size);

  create_descriptor(0, 0, 0);
  create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL0));
  create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL0));
  create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL3));
  create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL3));

  Info("Hello x8_64 ArchInit\n");

  return 0;
}
