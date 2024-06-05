
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
#include "libcxx.h"

extern "C" void _putchar(char character) {
  auto serial = cpu::Serial(cpu::kCom1);
  serial.Write(character);
}

template <uint32_t V>
class TestStaticConstructDestruct {
 public:
  TestStaticConstructDestruct(unsigned int &v) : _v(v) { _v |= V; }
  ~TestStaticConstructDestruct() { _v &= ~V; }

 private:
  unsigned int &_v;
};

static int global_value_with_init = 42;
static uint32_t global_u32_value_with_init{0xa1a2a3a4ul};
static uint64_t global_u64_value_with_init{0xb1b2b3b4b5b6b7b8ull};
static float global_f32_value_with_init{3.14};
static double global_f64_value_with_init{1.44};
static uint16_t global_u16_value_with_init{0x1234};
static uint8_t global_u8a_value_with_init{0x42};
static uint8_t global_u8b_value_with_init{0x43};
static uint8_t global_u8c_value_with_init{0x44};
static uint8_t global_u8d_value_with_init{0x45};
static volatile bool global_bool_keep_running{true};

// static unsigned int global_value1_with_constructor = 1;
// static unsigned int global_value2_with_constructor = 2;

// static TestStaticConstructDestruct<0x200> constructor_destructor_1(
//     global_value1_with_constructor);
// static TestStaticConstructDestruct<0x200> constructor_destructor_2(
//     global_value2_with_constructor);
// static TestStaticConstructDestruct<0x100000> constructor_destructor_3{
//     global_value2_with_constructor};
// static TestStaticConstructDestruct<0x100000> constructor_destructor_4{
//     global_value1_with_constructor};

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

  global_u8c_value_with_init++;
  global_u32_value_with_init++;
  global_u64_value_with_init++;
  global_f32_value_with_init++;
  global_u8b_value_with_init++;
  global_f64_value_with_init++;
  global_u8d_value_with_init++;
  global_u16_value_with_init++;
  global_u8a_value_with_init++;
  global_value_with_init++;
  global_bool_keep_running = false;

  BootInfo boot_info = *reinterpret_cast<BootInfo *>(argv);
  printf("boot_info.framebuffer.base: 0x%X\n", boot_info.framebuffer.base);
  Fillrect((uint8_t *)boot_info.framebuffer.base, 255, 0, 255, 100, 100);
  printf("Hello Test\n");

  // 进入死循环
  while (true) {
    ;
  }
  return 0;
}

extern "C" void _start(uint32_t argc, uint8_t *argv) {
  CppInit();
  main(argc, argv);
  CppDeInit();

  // 进入死循环
  while (1) {
    ;
  }
}
