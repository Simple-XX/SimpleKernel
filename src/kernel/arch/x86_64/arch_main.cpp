
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

#include "../../../../3rd/printf/370849c61557cb4086d69f22d7c2dd4972387583/src/include/printf.h"
#include "arch.h"
#include "cpu.hpp"

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

void _putchar(char character) {
  auto serial = CPU::Serial(CPU::COM1);
  serial.write(character);
}

int32_t arch_init(uint32_t _argc, uint8_t **_argv) {
  (void)_argc;
  (void)_argv;

  if (_argc == 1) {
    fillrect((uint8_t *)0x80000000, 0, 255, 0, 100, 100);
    boot_info_t boot_info = *reinterpret_cast<boot_info_t *>(_argv[0]);
    if (boot_info.framebuffer.base == 0x80000000) {
      fillrect((uint8_t *)0x80000000, 255, 0, 255, 100, 100);
    }
    _putchar('!');
    printf("hello world\n");
  } else {
    fillrect((uint8_t *)0x80000000, 255, 0, 0, 100, 100);
  }

  // 进入死循环
  while (1) {
    ;
  }

  return 0;
}
