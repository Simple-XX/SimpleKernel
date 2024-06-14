
/**
 * @file main.cpp
 * @brief main cpp
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

#include <opensbi_interface.h>

#include <cstdint>

#include "cpu.hpp"
#include "cstdio"
#include "cstring"
#include "kernel.h"
#include "kernel_elf.hpp"
#include "libcxx.h"

// printf_bare_metal 基本输出实现
extern "C" void _putchar(char character) {
  sbi_debug_console_write_byte(character);
}

void DumpStack() {
  uint64_t *fp = (uint64_t *)cpu::kAllXreg.fp.Read();
  uint64_t *ra = nullptr;

  printf("------DumpStack------\n");
  while (fp && *fp) {
    ra = fp - 1;
    fp = (uint64_t *)*(fp - 2);
    printf("[0x%p]\n", *ra);
  }
  printf("----DumpStack End----\n");
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

static unsigned int global_value1_with_constructor = 1;
static unsigned int global_value2_with_constructor = 2;

static TestStaticConstructDestruct<0x200> constructor_destructor_1(
    global_value1_with_constructor);
static TestStaticConstructDestruct<0x200> constructor_destructor_2(
    global_value2_with_constructor);
static TestStaticConstructDestruct<0x100000> constructor_destructor_3{
    global_value2_with_constructor};
static TestStaticConstructDestruct<0x100000> constructor_destructor_4{
    global_value1_with_constructor};

class AbsClass {
 public:
  AbsClass() { val = 'B'; }
  virtual ~AbsClass() { ; }
  virtual void Func() = 0;
  char val = 'A';
};

class InsClass : public AbsClass {
 public:
  void Func() override { val = 'C'; }
};

uint32_t main(uint32_t, uint8_t *) {
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

  auto inst_class = InsClass();
  _putchar(inst_class.val);
  _putchar('\n');
  inst_class.Func();
  _putchar(inst_class.val);
  _putchar('\n');

  DumpStack();

  _putchar('H');
  _putchar('e');
  _putchar('l');
  _putchar('l');
  _putchar('o');
  _putchar('W');
  _putchar('o');
  _putchar('r');
  _putchar('l');
  _putchar('d');
  _putchar('!');
  _putchar('\n');

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
