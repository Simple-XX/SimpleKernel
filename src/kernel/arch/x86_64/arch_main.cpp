
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

#include "arch.h"
#include "cpu.hpp"
#include "cstdio"

// printf_bare_metal 基本输出实现
/// @note 这里要注意，保证在 serial 初始化之前不能使用 printf
/// 函数，否则会有全局对象依赖问题
static auto serial = cpu::Serial(cpu::kCom1);
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

  BootInfo boot_info = *reinterpret_cast<BootInfo *>(argv);
  printf("boot_info.framebuffer.base: 0x%p.\n", boot_info.framebuffer.base);
  printf("boot_info.framebuffer.size: %d.\n", boot_info.framebuffer.size);
  printf("boot_info.framebuffer.height: %d.\n", boot_info.framebuffer.height);
  printf("boot_info.framebuffer.width: %d.\n", boot_info.framebuffer.width);
  printf("boot_info.framebuffer.pitch: %d.\n", boot_info.framebuffer.pitch);

  // 在屏幕上绘制矩形
  Fillrect((uint8_t *)boot_info.framebuffer.base, boot_info.framebuffer.pitch,
           255, 0, 255, 100, 100);

  for (uint32_t i = 0; i < boot_info.memory_map_count; i++) {
    printf(
        "boot_info.memory_map[%d].base_addr: 0x%p, length: 0x%X, type: %d.\n",
        i, boot_info.memory_map[i].base_addr, boot_info.memory_map[i].length,
        boot_info.memory_map[i].type);
  }

  printf("hello ArchInit\n");

  return 0;
}

void DumpStack() {
  uint64_t *rbp = (uint64_t *)cpu::ReadRbp();
  uint64_t *rip;

  printf("------DumpStack------\n");
  while (rbp) {
    rip = rbp + 1;
    printf("[0x%p]\n", *rip);
    rbp = (uint64_t *)*rbp;
  }
  printf("----DumpStack End----\n");
}
