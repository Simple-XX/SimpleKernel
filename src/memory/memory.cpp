/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include <algorithm>
#include <bmalloc.hpp>
#include <cstddef>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kernel_elf.hpp"
#include "kernel_log.hpp"
#include "virtual_memory.hpp"

namespace {

/// bmalloc 日志回调（空实现）
struct BmallocLogger {
  auto operator()(const char* format, ...) const -> int {
    (void)format;
    return 0;
  }
};

bmalloc::Bmalloc<BmallocLogger>* allocator = nullptr;
}  // namespace

extern "C" auto malloc(size_t size) -> void* {
  if (allocator) {
    return allocator->malloc(size);
  }
  return nullptr;
}

extern "C" auto free(void* ptr) -> void {
  if (allocator) {
    allocator->free(ptr);
  }
}

extern "C" auto calloc(size_t num, size_t size) -> void* {
  if (allocator) {
    return allocator->calloc(num, size);
  }
  return nullptr;
}

extern "C" auto realloc(void* ptr, size_t new_size) -> void* {
  if (allocator) {
    return allocator->realloc(ptr, new_size);
  }
  return nullptr;
}

extern "C" auto aligned_alloc(size_t alignment, size_t size) -> void* {
  if (allocator) {
    return allocator->aligned_alloc(alignment, size);
  }
  return nullptr;
}

extern "C" auto aligned_free(void* ptr) -> void {
  if (allocator) {
    allocator->aligned_free(ptr);
  }
}

auto MemoryInit() -> void {
  auto allocator_addr =
      reinterpret_cast<void*>(cpu_io::virtual_memory::PageAlignUp(
          BasicInfoSingleton::instance().elf_addr +
          KernelElfSingleton::instance().GetElfSize()));
  auto allocator_size = BasicInfoSingleton::instance().physical_memory_addr +
                        BasicInfoSingleton::instance().physical_memory_size -
                        reinterpret_cast<uint64_t>(allocator_addr);

  klog::Info("bmalloc address: {:#x}, size: {:#X}",
             static_cast<uint64_t>(reinterpret_cast<uintptr_t>(allocator_addr)),
             static_cast<uint64_t>(allocator_size));

  static bmalloc::Bmalloc<BmallocLogger> bmallocator(allocator_addr,
                                                     allocator_size);
  allocator = &bmallocator;

  // 初始化当前核心的虚拟内存
  VirtualMemorySingleton::create();
  VirtualMemorySingleton::instance().InitCurrentCore();

  // 重新映射早期控制台地址（如果有的话）
  if (SIMPLEKERNEL_EARLY_CONSOLE_BASE != 0) {
    VirtualMemorySingleton::instance()
        .MapMMIO(SIMPLEKERNEL_EARLY_CONSOLE_BASE,
                 cpu_io::virtual_memory::kPageSize)
        .or_else([](Error err) -> Expected<void*> {
          klog::Warn("Failed to remap early console MMIO: {}", err.message());
          return std::unexpected(err);
        });
  }

  klog::Info("Memory initialization completed");
}

auto MemoryInitSMP() -> void {
  VirtualMemorySingleton::instance().InitCurrentCore();
  klog::Info("SMP Memory initialization completed");
}
