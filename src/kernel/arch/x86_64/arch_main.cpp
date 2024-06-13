
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

#include "arch.h"
#include "cpu.hpp"
#include "cstdio"
#include "cstring"
#include "kernel_elf.hpp"

// printf_bare_metal 基本输出实现
/// @note 这里要注意，保证在 serial 初始化之前不能使用 printf
/// 函数，否则会有全局对象依赖问题
static cpu::Serial serial(cpu::kCom1);
extern "C" void _putchar(char character) { serial.Write(character); }

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

uint32_t ArchInit(uint32_t argc, uint8_t *argv) {
  if (argc != 1) {
    printf("argc != 1 [%d]\n", argc);
    return -1;
  }

  kBootInfo = *reinterpret_cast<BootInfo *>(argv);
  printf("kBootInfo.framebuffer.base: 0x%p.\n", kBootInfo.framebuffer.base);
  printf("kBootInfo.framebuffer.size: %d.\n", kBootInfo.framebuffer.size);
  printf("kBootInfo.framebuffer.height: %d.\n", kBootInfo.framebuffer.height);
  printf("kBootInfo.framebuffer.width: %d.\n", kBootInfo.framebuffer.width);
  printf("kBootInfo.framebuffer.pitch: %d.\n", kBootInfo.framebuffer.pitch);

  printf("kBootInfo.elf_addr: 0x%X.\n", kBootInfo.elf_addr);
  printf("kBootInfo.elf_size: %d.\n", kBootInfo.elf_size);

  // 在屏幕上绘制矩形
  Fillrect((uint8_t *)kBootInfo.framebuffer.base, kBootInfo.framebuffer.pitch,
           255, 0, 255, 100, 100);

  for (uint32_t i = 0; i < kBootInfo.memory_map_count; i++) {
    printf(
        "kBootInfo.memory_map[%d].base_addr: 0x%p, length: 0x%X, type: %d.\n",
        i, kBootInfo.memory_map[i].base_addr, kBootInfo.memory_map[i].length,
        kBootInfo.memory_map[i].type);
  }

  // 解析内核 elf 信息
  kKernelElf = KernelElf(kBootInfo.elf_addr, kBootInfo.elf_size);

  printf("hello ArchInit\n");

  return 0;
}

void DumpStack() {
  uint64_t *rbp = (uint64_t *)cpu::ReadRbp();
  uint64_t *rip = nullptr;

  printf("------DumpStack------\n");
  while (rbp && *rbp) {
    rip = rbp + 1;
    rbp = (uint64_t *)*rbp;

    // 打印函数名
    for (auto i : kKernelElf.symtab_) {
      if ((ELF64_ST_TYPE(i.st_info) == STT_FUNC) && (*rip >= i.st_value) &&
          (*rip <= i.st_value + i.st_size)) {
        printf("[%s] 0x%p\n", kKernelElf.strtab_ + i.st_name, *rip);
      }
    }
  }
  printf("----DumpStack End----\n");
}
