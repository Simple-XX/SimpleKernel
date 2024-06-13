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

// 禁用 GCC/Clang 的警告
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include <libfdt.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "cpu.hpp"
#include "cstdio"
#include "kernel_elf.hpp"
#include "ns16550a.h"
#include "opensbi_interface.h"

// printf_bare_metal 基本输出实现
extern "C" void _putchar(char character) {
  sbi_debug_console_write_byte(character);
}

void parse_memory_node(const void *fdt) {
  int offset;
  const struct fdt_property *prop;
  const uint64_t *reg;
  int len;

  // 找到 /memory 节点
  offset = fdt_path_offset(fdt, "/memory");
  if (offset < 0) {
    printf("Error finding /memory node: %s\n", fdt_strerror(offset));
    return;
  }

  // 获取 reg 属性
  prop = fdt_get_property(fdt, offset, "reg", &len);
  if (!prop) {
    printf("Error finding reg property: %s\n", fdt_strerror(len));
    return;
  }

  // 解析 reg 属性，通常包含基地址和大小
  reg = (const uint64_t *)prop->data;
  for (size_t i = 0; i < len / sizeof(uint64_t); i += 2) {
    uint64_t base = fdt64_to_cpu(reg[i]);
    uint64_t size = fdt64_to_cpu(reg[i + 1]);
    printf("Memory region: base address = 0x%llx, size = 0x%llx\n", base, size);
  }
}

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
uint32_t ArchInit(uint32_t argc, uint8_t *argv) {
  printf("boot hart id: %d\n", argc);
  printf("dtb info addr: %p\n", argv);

  // auto dtb_info = FDT_PARSER::fdt_parser((uintptr_t)argv);

  // auto resource_mem = FDT_PARSER::resource_t();
  // dtb_info.find_via_prefix("serial@", &resource_mem);
  // auto uart = Ns16550a(resource_mem.mem.addr);
  // uart.PutChar('H');
  // uart.PutChar('e');
  // uart.PutChar('l');
  // uart.PutChar('l');
  // uart.PutChar('o');
  // uart.PutChar(' ');
  // uart.PutChar('u');
  // uart.PutChar('a');
  // uart.PutChar('r');
  // uart.PutChar('t');
  // uart.PutChar('!');
  // uart.PutChar('\n');

  if (fdt_check_header((void *)argv) != 0) {
    printf("Invalid device tree blob\n");
  }

  parse_memory_node((void *)argv);

  // 解析内核 elf 信息
  kernel_elf = KernelElf();

  printf("hello ArchInit\n");

  return 0;
}

void DumpStack() {
  uint64_t *fp = (uint64_t *)cpu::ReadFp();
  uint64_t *ra = nullptr;

  printf("------DumpStack------\n");
  while (fp && *fp) {
    ra = fp - 1;
    fp = (uint64_t *)*(fp - 2);
    printf("[0x%p]\n", *ra);
    // 打印函数名
    // for (auto i : kernel_elf.symtab_) {
    //   if ((ELF64_ST_TYPE(i.st_info) == STT_FUNC) && (*ra >= i.st_value) &&
    //       (*ra <= i.st_value + i.st_size)) {
    //     printf("[%s] 0x%p\n", kernel_elf.strtab_ + i.st_name, *ra);
    //   }
    // }
  }
  printf("----DumpStack End----\n");
}
