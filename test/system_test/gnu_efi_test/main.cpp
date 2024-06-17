
/**
 * @file main.cpp
 * @brief 用于测试 gnu-efi 启动
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

#include "cpu.hpp"
#include "cstdio"
#include "cstring"
#include "kernel.h"

extern "C" void _putchar(char character) {
  auto serial = cpu::Serial(cpu::kCom1);
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

uint32_t main(uint32_t argc, uint8_t *argv) {
  if (argc != 1) {
    printf("argc != 1 [%d]\n", argc);
    return -1;
  }

  BasicInfo basic_info = *reinterpret_cast<BasicInfo *>(argv);
  printf("basic_info.framebuffer.base: 0x%X\n", basic_info.framebuffer.base);
  Fillrect((uint8_t *)basic_info.framebuffer.base, 255, 0, 255, 100, 100);

  printf("Hello Test\n");

  return 0;
}

extern "C" void _start(uint32_t argc, uint8_t *argv) {
  main(argc, argv);

  // 进入死循环
  while (1) {
    ;
  }
}
