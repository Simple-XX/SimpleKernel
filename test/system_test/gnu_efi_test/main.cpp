
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

#include <elf.h>

#include <cstdint>

#include "arch.h"
#include "cpu.hpp"
#include "cstdio"
#include "cstring"
#include "libcxx.h"

static auto serial = cpu::Serial(cpu::kCom1);
extern "C" void _putchar(char character) { serial.Write(character); }

/// 符号表地址
static Elf64_Sym *symtab = nullptr;
/// 符号个数
static size_t sym_count = 0;
/// 字符串表地址
static uint8_t *strtab = nullptr;

/**
 * 解析 elf 信息
 * @param ehdr elf 头
 */
static uint32_t ParseElf(uint64_t elf_addr) {
  auto ehdr = (Elf64_Ehdr *)elf_addr;
  if (memcmp(ehdr->e_ident,
             "\x7f"
             "ELF",
             4) != 0) {
    printf("Invalid ELF file\n");
    return -1;
  }

  auto shdr = (Elf64_Shdr *)((uint8_t *)ehdr + ehdr->e_shoff);
  auto shstrtab = (const char *)ehdr + shdr[ehdr->e_shstrndx].sh_offset;

  // 查找符号表和字符串表
  for (size_t i = 0; i < ehdr->e_shnum; ++i) {
    if (strcmp(shstrtab + shdr[i].sh_name, ".symtab") == 0) {
      symtab = (Elf64_Sym *)((uint8_t *)ehdr + shdr[i].sh_offset);
      sym_count = shdr[i].sh_size / sizeof(Elf64_Sym);

    } else if (strcmp(shstrtab + shdr[i].sh_name, ".strtab") == 0) {
      strtab = (uint8_t *)ehdr + shdr[i].sh_offset;
    }
  }

  if (!symtab || !strtab) {
    printf("Symbol table or string table not found\n");
    return -1;
  }

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
    for (size_t i = 0; i < sym_count; i++) {
      if (ELF64_ST_TYPE(symtab[i].st_info) == STT_FUNC) {
        if (*rip >= symtab[i].st_value &&
            *rip <= symtab[i].st_value + symtab[i].st_size) {
          printf("[%s] 0x%p\n", strtab + symtab[i].st_name, *rip);
        }
      }
    }
  }
  printf("----DumpStack End----\n");
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

  auto inst_class = InsClass();
  printf("%c\n", inst_class.val);
  inst_class.Func();
  printf("%c\n", inst_class.val);

  // 解析内核 elf 信息
  auto ret = ParseElf(boot_info.elf_addr);
  if (ret != 0) {
    printf("ParseElf failed!\n");
    return -1;
  }

  printf("Hello Test\n");

  DumpStack();

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
