/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "virtual_memory.hpp"

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

// 从 Singleton 获取已初始化的 VirtualMemory 实例
// VirtualMemorySingleton is defined in virtual_memory.hpp

auto virtual_memory_test() -> bool {
  klog::Info("virtual_memory_test: start");

  auto& vm = VirtualMemorySingleton::instance();

  // Test 1: 创建用户页表
  void* user_page_dir = aligned_alloc(cpu_io::virtual_memory::kPageSize,
                                      cpu_io::virtual_memory::kPageSize);
  EXPECT_TRUE(user_page_dir != nullptr,
              "virtual_memory_test: failed to create user page directory");
  memset(user_page_dir, 0, cpu_io::virtual_memory::kPageSize);
  klog::Info("virtual_memory_test: user page directory created at {}",
             user_page_dir);

  // Test 2: 映射页面
  void* virt_addr = reinterpret_cast<void*>(0x200000);
  void* phys_addr = reinterpret_cast<void*>(0x90000000);

  auto map_result =
      vm.MapPage(user_page_dir, virt_addr, phys_addr,
                 cpu_io::virtual_memory::GetUserPagePermissions());
  EXPECT_TRUE(map_result.has_value(),
              "virtual_memory_test: failed to map page");
  klog::Info("virtual_memory_test: mapped va={} to pa={}", virt_addr,
             phys_addr);

  // Test 3: 获取映射
  auto mapped = vm.GetMapping(user_page_dir, virt_addr);
  EXPECT_TRUE(mapped.has_value(), "virtual_memory_test: failed to get mapping");
  if (mapped) {
    EXPECT_EQ(*mapped, phys_addr,
              "virtual_memory_test: mapping address mismatch");
    klog::Info("virtual_memory_test: verified mapping pa={}", *mapped);
  }

  // Test 4: 映射多个页面
  constexpr size_t kNumPages = 5;
  for (size_t i = 0; i < kNumPages; ++i) {
    void* va = reinterpret_cast<void*>(0x300000 +
                                       i * cpu_io::virtual_memory::kPageSize);
    void* pa = reinterpret_cast<void*>(0x91000000 +
                                       i * cpu_io::virtual_memory::kPageSize);

    auto result = vm.MapPage(user_page_dir, va, pa,
                             cpu_io::virtual_memory::GetUserPagePermissions());
    EXPECT_TRUE(result.has_value(),
                "virtual_memory_test: failed to map multiple pages");
  }
  klog::Info("virtual_memory_test: mapped {} pages", kNumPages);

  // Test 5: 验证多页映射
  for (size_t i = 0; i < kNumPages; ++i) {
    void* va = reinterpret_cast<void*>(0x300000 +
                                       i * cpu_io::virtual_memory::kPageSize);
    void* pa = reinterpret_cast<void*>(0x91000000 +
                                       i * cpu_io::virtual_memory::kPageSize);

    auto m = vm.GetMapping(user_page_dir, va);
    EXPECT_TRUE(m.has_value(),
                "virtual_memory_test: multiple page mapping not found");
    if (m) {
      EXPECT_EQ(*m, pa, "virtual_memory_test: multiple page mapping mismatch");
    }
  }
  klog::Info("virtual_memory_test: verified {} page mappings", kNumPages);

  // Test 6: 取消映射
  auto unmap_result = vm.UnmapPage(user_page_dir, virt_addr);
  EXPECT_TRUE(unmap_result.has_value(),
              "virtual_memory_test: failed to unmap page");

  auto unmapped = vm.GetMapping(user_page_dir, virt_addr);
  EXPECT_TRUE(!unmapped.has_value(),
              "virtual_memory_test: page still mapped after unmap");
  klog::Info("virtual_memory_test: unmapped va={}", virt_addr);

  // Test 7: 克隆页表（复制映射）
  auto clone_result = vm.ClonePageDirectory(user_page_dir, true);
  EXPECT_TRUE(clone_result.has_value(),
              "virtual_memory_test: failed to clone page directory");
  void* cloned_page_dir = clone_result.value();
  EXPECT_TRUE(cloned_page_dir != user_page_dir,
              "virtual_memory_test: cloned page dir same as source");
  klog::Info("virtual_memory_test: cloned page directory to {}",
             cloned_page_dir);

  // Test 8: 验证克隆的映射
  for (size_t i = 0; i < kNumPages; ++i) {
    void* va = reinterpret_cast<void*>(0x300000 +
                                       i * cpu_io::virtual_memory::kPageSize);
    void* pa = reinterpret_cast<void*>(0x91000000 +
                                       i * cpu_io::virtual_memory::kPageSize);

    auto src_m = vm.GetMapping(user_page_dir, va);
    auto dst_m = vm.GetMapping(cloned_page_dir, va);

    EXPECT_TRUE(src_m.has_value(),
                "virtual_memory_test: source mapping lost after clone");
    EXPECT_TRUE(dst_m.has_value(),
                "virtual_memory_test: cloned mapping not found");

    if (src_m && dst_m) {
      EXPECT_EQ(*src_m, pa,
                "virtual_memory_test: source mapping changed after clone");
      EXPECT_EQ(*dst_m, pa, "virtual_memory_test: cloned mapping incorrect");
      EXPECT_EQ(*src_m, *dst_m,
                "virtual_memory_test: source and clone mappings differ");
    }
  }
  klog::Info("virtual_memory_test: verified cloned mappings");

  // Test 9: 克隆页表（不复制映射）
  auto clone_no_map_result = vm.ClonePageDirectory(user_page_dir, false);
  EXPECT_TRUE(clone_no_map_result.has_value(),
              "virtual_memory_test: failed to clone page dir (no mappings)");
  void* cloned_no_map = clone_no_map_result.value();
  klog::Info("virtual_memory_test: cloned page directory (no mappings) to {}",
             cloned_no_map);

  // 验证新页表没有映射
  for (size_t i = 0; i < kNumPages; ++i) {
    void* va = reinterpret_cast<void*>(0x300000 +
                                       i * cpu_io::virtual_memory::kPageSize);

    auto m = vm.GetMapping(cloned_no_map, va);
    EXPECT_TRUE(!m.has_value(),
                "virtual_memory_test: cloned (no map) should have no mappings");
  }
  klog::Info("virtual_memory_test: verified no mappings in cloned page dir");

  // Test 10: 在克隆的页表中添加新映射
  void* new_va = reinterpret_cast<void*>(0x400000);
  void* new_pa = reinterpret_cast<void*>(0x92000000);

  auto clone_map_result =
      vm.MapPage(cloned_no_map, new_va, new_pa,
                 cpu_io::virtual_memory::GetUserPagePermissions());
  EXPECT_TRUE(clone_map_result.has_value(),
              "virtual_memory_test: failed to map in cloned page dir");

  // 验证只在克隆的页表中有映射
  auto user_m = vm.GetMapping(user_page_dir, new_va);
  auto clone_m = vm.GetMapping(cloned_no_map, new_va);

  EXPECT_TRUE(!user_m.has_value(),
              "virtual_memory_test: mapping leaked to original page dir");
  EXPECT_TRUE(clone_m.has_value(),
              "virtual_memory_test: new mapping not in cloned page dir");
  if (clone_m) {
    EXPECT_EQ(*clone_m, new_pa,
              "virtual_memory_test: new mapping address incorrect");
  }
  klog::Info("virtual_memory_test: verified independent mappings");

  // Test 11: 销毁页表
  vm.DestroyPageDirectory(user_page_dir, false);
  klog::Info("virtual_memory_test: destroyed user page directory");

  vm.DestroyPageDirectory(cloned_page_dir, false);
  klog::Info("virtual_memory_test: destroyed cloned page directory");

  vm.DestroyPageDirectory(cloned_no_map, false);
  klog::Info("virtual_memory_test: destroyed cloned (no map) page directory");

  // Test 12: 重新映射测试
  void* test_page_dir = aligned_alloc(cpu_io::virtual_memory::kPageSize,
                                      cpu_io::virtual_memory::kPageSize);
  EXPECT_TRUE(test_page_dir != nullptr,
              "virtual_memory_test: failed to create test page dir");
  memset(test_page_dir, 0, cpu_io::virtual_memory::kPageSize);

  void* test_va = reinterpret_cast<void*>(0x500000);
  void* test_pa1 = reinterpret_cast<void*>(0x93000000);
  void* test_pa2 = reinterpret_cast<void*>(0x94000000);

  // 第一次映射
  (void)vm.MapPage(test_page_dir, test_va, test_pa1,
                   cpu_io::virtual_memory::GetUserPagePermissions());

  auto m1 = vm.GetMapping(test_page_dir, test_va);
  EXPECT_TRUE(m1.has_value(), "virtual_memory_test: first mapping failed");
  if (m1) {
    EXPECT_EQ(*m1, test_pa1, "virtual_memory_test: first mapping incorrect");
  }

  // 重新映射到不同物理地址
  (void)vm.MapPage(test_page_dir, test_va, test_pa2,
                   cpu_io::virtual_memory::GetUserPagePermissions());

  auto m2 = vm.GetMapping(test_page_dir, test_va);
  EXPECT_TRUE(m2.has_value(), "virtual_memory_test: remap failed");
  if (m2) {
    EXPECT_EQ(*m2, test_pa2, "virtual_memory_test: remap address incorrect");
  }
  klog::Info("virtual_memory_test: verified remap from {} to {}", test_pa1,
             test_pa2);

  // 清理
  vm.DestroyPageDirectory(test_page_dir, false);

  klog::Info("virtual_memory_test: all tests passed");
  return true;
}
