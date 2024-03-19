
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

#include "arch.h"
#include "cpu.hpp"
#include "cstdio"
#include "kernel.h"

extern "C" void _start(int _argc, char **_argv) { main(_argc, _argv); }

static const int pixelwidth = 4;
static const int pitch = 800 * pixelwidth;

static void fillrect(uint8_t *vram, uint8_t r, uint8_t g, unsigned char b,
                     uint8_t w, uint8_t h) {
  unsigned char *where = vram;
  int i, j;

  for (i = 0; i < w; i++) {
    for (j = 0; j < h; j++) {
      // putpixel(vram, 64 + j, 64 + i, (r << 16) + (g << 8) + b);
      where[j * pixelwidth] = r;
      where[j * pixelwidth + 1] = g;
      where[j * pixelwidth + 2] = b;
    }
    where += pitch;
  }
}

int main(int _argc, char **_argv) {
  (void)_argc;
  (void)_argv;

  if (_argc != 1) {
    printf("_argc != 1 [%d]\n", _argc);
    return -1;
  }

  boot_info_t boot_info = *reinterpret_cast<boot_info_t *>(_argv[0]);
  printf("boot_info.framebuffer.base: 0x%X\n", boot_info.framebuffer.base);
  fillrect((uint8_t *)boot_info.framebuffer.base, 255, 0, 255, 100, 100);
  printf("Hello Test\n");

  // 进入死循环
  while (true) {
    ;
  }
  return 0;
}
