
/**
 * @file main.cpp
 * @brief 内核入口
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

#include <cstdint>

#include "arch.h"
#include "cpu.hpp"
#include "cstdio"
#include "kernel.h"

extern "C" void _start(int argc, char **argv) { main(argc, argv); }

extern "C" void _putchar(char character) {
  auto serial = Cpu::Serial(Cpu::kCom1);
  serial.Write(character);
}

static const int kPixelwidth = 4;
static const int kPitch = 800 * kPixelwidth;

static void Fillrect(uint8_t *vram, uint8_t r, uint8_t g, unsigned char b,
                     uint8_t w, uint8_t h) {
  unsigned char *where = vram;
  int i, j;

  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      // putpixel(vram, 64 + j, 64 + i, (r << 16) + (g << 8) + b);
      where[j * kPixelwidth] = r;
      where[j * kPixelwidth + 1] = g;
      where[j * kPixelwidth + 2] = b;
    }
    where += kPitch;
  }
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  if (argc != 1) {
    printf("argc != 1 [%d]\n", argc);
    return -1;
  }

  BootInfo boot_info = *reinterpret_cast<BootInfo *>(argv[0]);
  printf("boot_info.framebuffer.base: 0x%X\n", boot_info.framebuffer.base);
  Fillrect((uint8_t *)boot_info.framebuffer.base, 255, 0, 255, 100, 100);
  printf("Hello Test\n");

  // 进入死循环
  while (true) {
    ;
  }
  return 0;
}
