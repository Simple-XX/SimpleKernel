
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

#include "arch.h"
#include "cpu.hpp"
#include "cstdio"
#include "cstring"

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

  printf("boot_info.elf_addr: 0x%X.\n", boot_info.elf_addr);
  printf("boot_info.elf_size: %d.\n", boot_info.elf_size);

  // 在屏幕上绘制矩形
  Fillrect((uint8_t *)boot_info.framebuffer.base, boot_info.framebuffer.pitch,
           255, 0, 255, 100, 100);

  for (uint32_t i = 0; i < boot_info.memory_map_count; i++) {
    printf(
        "boot_info.memory_map[%d].base_addr: 0x%p, length: 0x%X, type: %d.\n",
        i, boot_info.memory_map[i].base_addr, boot_info.memory_map[i].length,
        boot_info.memory_map[i].type);
  }

  // 解析内核 elf 信息
  auto ret = ParseElf(boot_info.elf_addr);
  if (ret != 0) {
    printf("ParseElf failed!\n");
    return -1;
  }

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
