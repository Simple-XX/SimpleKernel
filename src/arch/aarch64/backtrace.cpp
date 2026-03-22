/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>
#include <elf.h>

#include <array>
#include <cerrno>
#include <cstdint>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kernel_elf.hpp"
#include "kernel_log.hpp"

__always_inline auto backtrace(std::array<uint64_t, kMaxFrameCount>& buffer)
    -> int {
  auto* x29 = reinterpret_cast<uint64_t*>(cpu_io::X29::Read());
  size_t count = 0;
  while ((x29 != nullptr) && (x29[0] != 0U) &&
         x29[0] >= reinterpret_cast<uint64_t>(__executable_start) &&
         x29[0] <= reinterpret_cast<uint64_t>(__etext) &&
         count < buffer.max_size()) {
    auto lr = x29[1];
    x29 = reinterpret_cast<uint64_t*>(x29[0]);
    buffer[count++] = lr;
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
