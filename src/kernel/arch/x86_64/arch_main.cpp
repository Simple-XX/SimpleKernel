
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
extern "C" void _putchar(char character) {
  auto serial = Cpu::Serial(Cpu::kCom1);
  serial.write(character);
}

static const int kPixelWidth = 4;
static const int kPitch = 800 * kPixelWidth;

static void fillrect(uint8_t *vram, uint8_t r, uint8_t g, unsigned char b,
                     uint8_t w, uint8_t h) {
  unsigned char *where = vram;
  int i, j;

  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      // putpixel(vram, 64 + j, 64 + i, (r << 16) + (g << 8) + b);
      where[j * kPixelWidth] = r;
      where[j * kPixelWidth + 1] = g;
      where[j * kPixelWidth + 2] = b;
    }
    where += kPitch;
  }
}

int32_t arch_init(uint32_t argc, uint8_t **argv) {
  if (argc != 1) {
    printf("argc != 1 [%d]\n", argc);
    return -1;
  }

  BootInfo boot_info = *reinterpret_cast<BootInfo *>(argv[0]);
  printf("boot_info.framebuffer.base: 0x%X\n", boot_info.framebuffer.base);

  fillrect((uint8_t *)boot_info.framebuffer.base, 255, 0, 255, 100, 100);
  printf("hello arch_init\n");

  // static auto serial = Cpu::Serial(Cpu::COM1);
  // serial.write('!');

  _putchar('1');

  return 0;
}
