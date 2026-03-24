/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>
#include <elf.h>

#include <array>
#include <cstdint>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kernel_elf.hpp"
#include "kernel_log.hpp"

__always_inline auto backtrace(std::array<uint64_t, kMaxFrameCount>& buffer)
    -> int {
  auto fp_value = cpu_io::Fp::Read();
  size_t count = 0;

  // RISC-V 栈帧布局 (使用 -fno-omit-frame-pointer):
  // fp[-1] (fp - 8):  保存的返回地址 (ra)
  // fp[-2] (fp - 16): 保存的上一个帧指针 (saved fp)

  while ((fp_value & 0x07) == 0 && count < buffer.max_size()) {
    auto* fp = reinterpret_cast<uint64_t*>(fp_value);
    auto ra = *(fp - 1);
    auto saved_fp = *(fp - 2);

    // 检查返回地址是否在代码段范围内
    if (ra < reinterpret_cast<uint64_t>(__executable_start) ||
        ra > reinterpret_cast<uint64_t>(__etext)) {
      break;
    }

    buffer[count++] = ra;

    // 如果 saved_fp 为 0 或无效，停止遍历
    if (saved_fp == 0) {
      break;
    }
    fp_value = saved_fp;
  }
  return static_cast<int>(count);
}

auto DumpStack() -> void {
  std::array<uint64_t, kMaxFrameCount> buffer{};

  // 获取调用栈中的地址
  auto num_frames = backtrace(buffer);

  // 打印函数名
  for (auto current_frame_idx = 0; current_frame_idx < num_frames;
       current_frame_idx++) {
    for (auto symtab : KernelElfSingleton::instance().symtab) {
      if ((ELF64_ST_TYPE(symtab.st_info) == STT_FUNC) &&
          (buffer[current_frame_idx] >= symtab.st_value) &&
          (buffer[current_frame_idx] <= symtab.st_value + symtab.st_size)) {
        klog::Err("[{}] {:#x}",
                  reinterpret_cast<const char*>(
                      KernelElfSingleton::instance().strtab + symtab.st_name),
                  buffer[current_frame_idx]);
      }
    }
  }
}

auto RawDumpStack() -> void {
  std::array<uint64_t, kMaxFrameCount> buffer{};
  auto num_frames = backtrace(buffer);

  klog::RawPut("\n--- Raw Stack Trace ---\n");

  char line_buf[256];
  for (auto i = 0; i < num_frames; i++) {
    bool found = false;
    for (auto symtab : KernelElfSingleton::instance().symtab) {
      if ((ELF64_ST_TYPE(symtab.st_info) == STT_FUNC) &&
          (buffer[i] >= symtab.st_value) &&
          (buffer[i] <= symtab.st_value + symtab.st_size)) {
        auto* end = etl::format_to_n(
            line_buf, sizeof(line_buf) - 1, "  #{} {:#018x} [{}]+{:#x}", i,
            buffer[i],
            reinterpret_cast<const char*>(
                KernelElfSingleton::instance().strtab + symtab.st_name),
            buffer[i] - symtab.st_value);
        *end = '\0';
        klog::RawPut(line_buf);
        etl_putchar('\n');
        found = true;
        break;
      }
    }
    if (!found) {
      auto* end = etl::format_to_n(line_buf, sizeof(line_buf) - 1,
                                   "  #{} {:#018x} [???]", i, buffer[i]);
      *end = '\0';
      klog::RawPut(line_buf);
      etl_putchar('\n');
    }
  }

  klog::RawPut("--- End Stack Trace ---\n");
}
