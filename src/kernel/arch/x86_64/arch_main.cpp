
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

#include "cpu.hpp"
#include "cstdio"
#include "cstring"
#include "kernel.h"
#include "kernel_elf.hpp"
#include "kernel_log.hpp"

// printf_bare_metal 基本输出实现
/// @note 这里要注意，保证在 serial 初始化之前不能使用 printf
/// 函数，否则会有全局对象依赖问题
static cpu::Serial kSerial(cpu::kCom1);
extern "C" void _putchar(char character) { kSerial.Write(character); }

/// gdt 描述符表
static cpu::reginfo::GdtrInfo::SegmentDescriptor
    segment_descriptors[cpu::reginfo::GdtrInfo::kGdtMaxCount] = {};

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

struct GDTEntry {
  uint16_t Limit0 = 0;
  uint16_t Base0 = 0;
  uint8_t Base1 = 0;
  uint8_t AccessByte = 0;
  uint8_t Limit1_Flags = 0;
  uint8_t Base2 = 0;
};

GDTEntry gdt[5];

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access,
                  uint8_t gran) {
  // gdt[num].Base0 = (base & 0xFFFF);
  // gdt[num].Base1 = (base >> 16) & 0xFF;
  // gdt[num].Base2 = (base >> 24) & 0xFF;

  // gdt[num].Limit0 = (limit & 0xFFFF);
  // gdt[num].Limit1_Flags = (limit >> 16) & 0x0F;

  gdt[num].Limit1_Flags |= gran & 0xF0;
  gdt[num].AccessByte = access;
}

uint32_t ArchInit(uint32_t argc, uint8_t *argv) {
  if (argc != 1) {
    Err("argc != 1 [%d]\n", argc);
    throw;
  }

  kBasicInfo.GetInstance() = *reinterpret_cast<BasicInfo *>(argv);
  printf("kBasicInfo.framebuffer.base: 0x%p.\n",
         kBasicInfo.GetInstance().framebuffer.base);
  printf("kBasicInfo.framebuffer.size: %d.\n",
         kBasicInfo.GetInstance().framebuffer.size);
  printf("kBasicInfo.framebuffer.height: %d.\n",
         kBasicInfo.GetInstance().framebuffer.height);
  printf("kBasicInfo.framebuffer.width: %d.\n",
         kBasicInfo.GetInstance().framebuffer.width);
  printf("kBasicInfo.framebuffer.pitch: %d.\n",
         kBasicInfo.GetInstance().framebuffer.pitch);

  printf("kBasicInfo.elf_addr: 0x%X.\n", kBasicInfo.GetInstance().elf_addr);
  printf("kBasicInfo.elf_size: %d.\n", kBasicInfo.GetInstance().elf_size);

  // 在屏幕上绘制矩形
  Fillrect((uint8_t *)kBasicInfo.GetInstance().framebuffer.base,
           kBasicInfo.GetInstance().framebuffer.pitch, 255, 0, 255, 100, 100);

  for (uint32_t i = 0; i < kBasicInfo.GetInstance().memory_map_count; i++) {
    printf(
        "kBasicInfo.memory_map[%d].base_addr: 0x%p, length: 0x%X, type: %d.\n",
        i, kBasicInfo.GetInstance().memory_map[i].base_addr,
        kBasicInfo.GetInstance().memory_map[i].length,
        kBasicInfo.GetInstance().memory_map[i].type);
  }

  // 解析内核 elf 信息
  kKernelElf.GetInstance() = KernelElf(kBasicInfo.GetInstance().elf_addr,
                                       kBasicInfo.GetInstance().elf_size);

  // 设置段描述符
  // 第一个全 0
  segment_descriptors[0] = cpu::reginfo::GdtrInfo::SegmentDescriptor();
  // 内核代码段描述符
  segment_descriptors[1] = cpu::reginfo::GdtrInfo::SegmentDescriptor(
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kCodeExecuteRead,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kCodeData,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kRing0,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kPresent,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kNotAvailable,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::k64Bit);
  // 内核数据段描述符
  segment_descriptors[2] = cpu::reginfo::GdtrInfo::SegmentDescriptor(
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kDataReadWrite,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kCodeData,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kRing0,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kPresent,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kNotAvailable,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::k64Bit);
  // 用户代码段描述符
  segment_descriptors[3] = cpu::reginfo::GdtrInfo::SegmentDescriptor(
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kCodeExecuteRead,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kCodeData,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kRing3,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kPresent,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kNotAvailable,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::k64Bit);
  // 用户数据段描述符
  segment_descriptors[4] = cpu::reginfo::GdtrInfo::SegmentDescriptor(
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kDataReadWrite,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kCodeData,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kRing3,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kPresent,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::kNotAvailable,
      cpu::reginfo::GdtrInfo::SegmentDescriptor::k64Bit);

  // std::cout << segment_descriptors[0] << std::endl;
  // std::cout << segment_descriptors[1] << std::endl;
  // std::cout << segment_descriptors[2] << std::endl;
  // std::cout << segment_descriptors[3] << std::endl;
  // std::cout << segment_descriptors[4] << std::endl;

  gdt_set_gate(0, 0, 0, 0, 0);
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xAF);
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xAF);
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xAF);
  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xAF);

  cpu::reginfo::GdtrInfo::Gdtr gdtr{};
  gdtr.limit = (sizeof(cpu::reginfo::GdtrInfo::SegmentDescriptor) *
                cpu::reginfo::GdtrInfo::kGdtMaxCount) -
               1;
  // gdtr.base = segment_descriptors;
  gdtr.base = (cpu::reginfo::GdtrInfo::SegmentDescriptor *)gdt;
  cpu::kAllCr.gdtr.Write(gdtr);

  cpu::kAllCr.ds.Write(0x10);
  cpu::kAllCr.es.Write(0x10);
  cpu::kAllCr.fs.Write(0x10);
  cpu::kAllCr.gs.Write(0x10);
  cpu::kAllCr.ss.Write(0x10);
  cpu::kAllCr.cs.Write(0x8);

  std::cout << cpu::kAllCr.es << std::endl;
  std::cout << cpu::kAllCr.cs << std::endl;
  std::cout << cpu::kAllCr.ss << std::endl;
  std::cout << cpu::kAllCr.ds << std::endl;
  std::cout << cpu::kAllCr.fs << std::endl;
  std::cout << cpu::kAllCr.gs << std::endl;

  for (auto i = 0; i < (cpu::kAllCr.gdtr.Read().limit + 1) /
                           sizeof(cpu::reginfo::GdtrInfo::SegmentDescriptor);
       i++) {
    printf("[0x%X]0x%p\n", i, cpu::kAllCr.gdtr.Read().base + i);
    std::cout << *(cpu::kAllCr.gdtr.Read().base + i) << std::endl;
  }

  Info("Hello x86_64 ArchInit\n");

  return 0;
}
