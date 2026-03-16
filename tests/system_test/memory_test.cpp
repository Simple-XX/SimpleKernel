/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include <cstddef>
#include <cstdint>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kstd_cstring"
#include "kstd_libcxx.h"
#include "sk_stdlib.h"
#include "system_test.h"

extern "C" {
void* malloc(size_t size);
void free(void* ptr);
void* aligned_alloc(size_t alignment, size_t size);
}

auto memory_test() -> bool {
  klog::Info("memory_test: start");

  // Test 1: malloc & free
  size_t size = 1024;
  void* ptr = malloc(size);
  EXPECT_TRUE(ptr != nullptr, "memory_test: malloc failed");

  // Write and read verification
  auto* byte_ptr = static_cast<uint8_t*>(ptr);
  for (size_t i = 0; i < size; ++i) {
    byte_ptr[i] = static_cast<uint8_t>(i & 0xFF);
  }

  for (size_t i = 0; i < size; ++i) {
    EXPECT_EQ(byte_ptr[i], static_cast<uint8_t>(i & 0xFF),
              "memory_test: verify failed");
  }

  free(ptr);
  klog::Info("memory_test: malloc/free passed");

  // Test 2: aligned_alloc
  size_t alignment = 256;
  size_t aligned_size = 512;
  void* aligned_ptr = aligned_alloc(alignment, aligned_size);
  EXPECT_TRUE(aligned_ptr != nullptr, "memory_test: aligned_alloc failed");

  EXPECT_EQ(reinterpret_cast<uintptr_t>(aligned_ptr) & (alignment - 1), 0,
            "memory_test: aligned_alloc alignment failed");

  free(aligned_ptr);
  klog::Info("memory_test: aligned_alloc passed");

  // Test 3: Multiple small allocations
  const int count = 10;
  void* ptrs[count];

  for (int i = 0; i < count; ++i) {
    ptrs[i] = malloc(128);
    EXPECT_TRUE(ptrs[i] != nullptr, "memory_test: multi alloc failed");
    // Fill
    kstd::memset(ptrs[i], i, 128);
  }

  for (int i = 0; i < count; ++i) {
    auto* p = static_cast<uint8_t*>(ptrs[i]);
    for (int j = 0; j < 128; ++j) {
      EXPECT_EQ(p[j], i, "memory_test: multi alloc verify failed");
    }
    free(ptrs[i]);
  }
  klog::Info("memory_test: multi alloc passed");

  return true;
}
