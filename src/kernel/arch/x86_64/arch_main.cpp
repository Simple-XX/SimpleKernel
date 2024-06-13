
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

uint32_t ArchInit(uint32_t argc, uint8_t *argv) {
  if (argc != 1) {
    printf("argc != 1 [%d]\n", argc);
    return -1;
  }

  kBasicInfo = *reinterpret_cast<BasicInfo *>(argv);
  printf("kBasicInfo.framebuffer.base: 0x%p.\n", kBasicInfo.framebuffer.base);
  printf("kBasicInfo.framebuffer.size: %d.\n", kBasicInfo.framebuffer.size);
  printf("kBasicInfo.framebuffer.height: %d.\n", kBasicInfo.framebuffer.height);
  printf("kBasicInfo.framebuffer.width: %d.\n", kBasicInfo.framebuffer.width);
  printf("kBasicInfo.framebuffer.pitch: %d.\n", kBasicInfo.framebuffer.pitch);

  printf("kBasicInfo.elf_addr: 0x%X.\n", kBasicInfo.elf_addr);
  printf("kBasicInfo.elf_size: %d.\n", kBasicInfo.elf_size);

  // 在屏幕上绘制矩形
  Fillrect((uint8_t *)kBasicInfo.framebuffer.base, kBasicInfo.framebuffer.pitch,
           255, 0, 255, 100, 100);

  for (uint32_t i = 0; i < kBasicInfo.memory_map_count; i++) {
    printf(
        "kBasicInfo.memory_map[%d].base_addr: 0x%p, length: 0x%X, type: %d.\n",
        i, kBasicInfo.memory_map[i].base_addr, kBasicInfo.memory_map[i].length,
        kBasicInfo.memory_map[i].type);
  }

  // 解析内核 elf 信息
  kKernelElf = KernelElf(kBasicInfo.elf_addr, kBasicInfo.elf_size);

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
