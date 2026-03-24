/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "virtual_memory.hpp"

#include <cpu_io.h>

#include <bmalloc.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "basic_info.hpp"
#include "expected.hpp"
#include "kernel.h"
#include "kernel_log.hpp"
#include "sk_stdlib.h"

VirtualMemory::VirtualMemory() {
  // 分配根页表目录
  kernel_page_dir_ = aligned_alloc(cpu_io::virtual_memory::kPageSize,
                                   cpu_io::virtual_memory::kPageSize);
  assert(kernel_page_dir_ != nullptr &&
         "Failed to allocate kernel page directory");

  // 清零页表目录
  std::memset(kernel_page_dir_, 0, cpu_io::virtual_memory::kPageSize);

  // 获取内核基本信息
  const auto& basic_info = BasicInfoSingleton::instance();

  // 映射全部物理内存
  MapMMIO(basic_info.physical_memory_addr, basic_info.physical_memory_size)
      .or_else([](auto&& err) -> Expected<void*> {
        klog::Err("Failed to map kernel memory: {}", err.message());
        while (true) {
          cpu_io::Pause();
        }
        return {};
      });

  klog::Info("Kernel memory mapped from {:#X} to {:#X}",
             basic_info.physical_memory_addr,
             basic_info.physical_memory_addr + basic_info.physical_memory_size);
}

auto VirtualMemory::InitCurrentCore() const -> void {
  cpu_io::virtual_memory::SetPageDirectory(
      reinterpret_cast<uint64_t>(kernel_page_dir_));
  // 开启分页
  cpu_io::virtual_memory::EnablePage();
}

auto VirtualMemory::MapMMIO(uint64_t phys_addr, size_t size, uint32_t flags)
    -> Expected<void*> {
  // 计算对齐后的起始和结束页
  auto start_page = cpu_io::virtual_memory::PageAlign(phys_addr);
  auto end_page = cpu_io::virtual_memory::PageAlignUp(phys_addr + size);

  // 遍历并映射
  for (uint64_t addr = start_page; addr < end_page;
       addr += cpu_io::virtual_memory::kPageSize) {
    auto result = MapPage(kernel_page_dir_, reinterpret_cast<void*>(addr),
                          reinterpret_cast<void*>(addr), flags);
    if (!result.has_value()) {
      return std::unexpected(result.error());
    }
  }
  return reinterpret_cast<void*>(phys_addr);
}

auto VirtualMemory::MapPage(void* page_dir, void* virtual_addr,
                            void* physical_addr, uint32_t flags)
    -> Expected<void> {
  assert(page_dir != nullptr && "MapPage: page_dir is null");

  // 查找页表项，如果不存在则分配
  auto pte_result = FindPageTableEntry(page_dir, virtual_addr, true);
  if (!pte_result.has_value()) {
    return std::unexpected(Error(ErrorCode::kVmMapFailed));
  }

  auto pte = pte_result.value();

  // 检查是否已经映射且标志位相同
  if (cpu_io::virtual_memory::IsPageTableEntryValid(*pte)) {
    // 如果物理地址和标志位都相同，则认为是重复映射（警告但不失败）
    auto existing_pa = cpu_io::virtual_memory::PageTableEntryToPhysical(*pte);
    if (existing_pa == reinterpret_cast<uint64_t>(physical_addr) &&
        (*pte & ((1ULL << cpu_io::virtual_memory::kPteAttributeBits) - 1)) ==
            flags) {
      klog::Debug(
          "MapPage: duplicate va = {:#x}, pa = {:#X}, flags = {:#X}, skip",
          static_cast<uint64_t>(reinterpret_cast<uintptr_t>(virtual_addr)),
          static_cast<uint64_t>(existing_pa), static_cast<uint64_t>(flags));
      // 重复映射，但不是错误
      return {};
    }
    klog::Warn(
        "MapPage: remap va = {:#x} from pa = {:#X} to pa = {:#x}",
        static_cast<uint64_t>(reinterpret_cast<uintptr_t>(virtual_addr)),
        static_cast<uint64_t>(existing_pa),
        static_cast<uint64_t>(reinterpret_cast<uintptr_t>(physical_addr)));
  }

  // 设置页表项
  *pte = cpu_io::virtual_memory::PhysicalToPageTableEntry(
      reinterpret_cast<uint64_t>(physical_addr), flags);
  // 刷新 TLB
  cpu_io::virtual_memory::FlushTLBAll();

  return {};
}

auto VirtualMemory::UnmapPage(void* page_dir, void* virtual_addr)
    -> Expected<void> {
  assert(page_dir != nullptr && "UnmapPage: page_dir is null");

  auto pte_result = FindPageTableEntry(page_dir, virtual_addr, false);
  if (!pte_result.has_value()) {
    return std::unexpected(Error(ErrorCode::kVmPageNotMapped));
  }

  auto pte = pte_result.value();

  if (!cpu_io::virtual_memory::IsPageTableEntryValid(*pte)) {
    return std::unexpected(Error(ErrorCode::kVmPageNotMapped));
  }

  // 清除页表项
  *pte = 0;

  // 刷新 TLB
  cpu_io::virtual_memory::FlushTLBAll();

  return {};
}

auto VirtualMemory::GetMapping(void* page_dir, void* virtual_addr)
    -> Expected<void*> {
  assert(page_dir != nullptr && "GetMapping: page_dir is null");

  auto pte_result = FindPageTableEntry(page_dir, virtual_addr, false);
  if (!pte_result.has_value()) {
    return std::unexpected(Error(ErrorCode::kVmPageNotMapped));
  }

  auto pte = pte_result.value();

  if (!cpu_io::virtual_memory::IsPageTableEntryValid(*pte)) {
    return std::unexpected(Error(ErrorCode::kVmPageNotMapped));
  }

  return reinterpret_cast<void*>(
      cpu_io::virtual_memory::PageTableEntryToPhysical(*pte));
}

auto VirtualMemory::DestroyPageDirectory(void* page_dir, bool free_pages)
    -> void {
  if (page_dir == nullptr) {
    return;
  }

  // 递归释放所有层级的页表
  RecursiveFreePageTable(reinterpret_cast<uint64_t*>(page_dir),
                         cpu_io::virtual_memory::kPageTableLevels - 1,
                         free_pages);

  // 释放根页表目录本身
  aligned_free(page_dir);

  klog::Debug("Destroyed page directory at address: {:#x}",
              static_cast<uint64_t>(reinterpret_cast<uintptr_t>(page_dir)));
}

auto VirtualMemory::ClonePageDirectory(void* src_page_dir, bool copy_mappings)
    -> Expected<void*> {
  assert(src_page_dir != nullptr &&
         "ClonePageDirectory: source page directory is nullptr");

  // 创建新的页表目录
  auto dst_page_dir = aligned_alloc(cpu_io::virtual_memory::kPageSize,
                                    cpu_io::virtual_memory::kPageSize);
  if (dst_page_dir == nullptr) {
    return std::unexpected(Error(ErrorCode::kVmAllocationFailed));
  }

  // 清零新页表
  std::memset(dst_page_dir, 0, cpu_io::virtual_memory::kPageSize);

  // 递归复制页表
  auto result = RecursiveClonePageTable(
      reinterpret_cast<uint64_t*>(src_page_dir),
      reinterpret_cast<uint64_t*>(dst_page_dir),
      cpu_io::virtual_memory::kPageTableLevels - 1, copy_mappings);
  if (!result.has_value()) {
    // 复制失败，清理已分配的页表
    DestroyPageDirectory(dst_page_dir, false);
    return std::unexpected(result.error());
  }

  klog::Debug("Cloned page directory from {:#x} to {:#x}",
              static_cast<uint64_t>(reinterpret_cast<uintptr_t>(src_page_dir)),
              static_cast<uint64_t>(reinterpret_cast<uintptr_t>(dst_page_dir)));
  return dst_page_dir;
}

auto VirtualMemory::RecursiveFreePageTable(uint64_t* table, size_t level,
                                           bool free_pages) -> void {
  if (table == nullptr) {
    return;
  }

  // 遍历页表中的所有条目
  for (size_t i = 0; i < kEntriesPerTable; ++i) {
    uint64_t pte = table[i];
    if (!cpu_io::virtual_memory::IsPageTableEntryValid(pte)) {
      continue;
    }

    auto pa = cpu_io::virtual_memory::PageTableEntryToPhysical(pte);

    // 如果不是最后一级，递归释放子页表
    if (level > 0) {
      RecursiveFreePageTable(reinterpret_cast<uint64_t*>(pa), level - 1,
                             free_pages);
    } else if (free_pages) {
      // 最后一级页表，释放物理页
      aligned_free(reinterpret_cast<void*>(pa));
    }

    // 清除页表项
    table[i] = 0;
  }

  // 如果不是根页表，释放当前页表
  if (level < cpu_io::virtual_memory::kPageTableLevels - 1) {
    aligned_free(table);
  }
}

auto VirtualMemory::RecursiveClonePageTable(uint64_t* src_table,
                                            uint64_t* dst_table, size_t level,
                                            bool copy_mappings)
    -> Expected<void> {
  assert(src_table != nullptr && "RecursiveClonePageTable: src_table is null");
  assert(dst_table != nullptr && "RecursiveClonePageTable: dst_table is null");

  for (size_t i = 0; i < kEntriesPerTable; ++i) {
    uint64_t src_pte = src_table[i];
    if (!cpu_io::virtual_memory::IsPageTableEntryValid(src_pte)) {
      continue;
    }

    if (level > 0) {
      // 非最后一级，需要递归复制子页表
      auto src_pa = cpu_io::virtual_memory::PageTableEntryToPhysical(src_pte);
      auto* src_next_table = reinterpret_cast<uint64_t*>(src_pa);

      // 分配新的子页表
      auto* dst_next_table = aligned_alloc(cpu_io::virtual_memory::kPageSize,
                                           cpu_io::virtual_memory::kPageSize);
      if (dst_next_table == nullptr) {
        return std::unexpected(Error(ErrorCode::kVmAllocationFailed));
      }

      // 清零新页表
      std::memset(dst_next_table, 0, cpu_io::virtual_memory::kPageSize);

      // 递归复制子页表
      auto result = RecursiveClonePageTable(
          src_next_table, reinterpret_cast<uint64_t*>(dst_next_table),
          level - 1, copy_mappings);
      if (!result.has_value()) {
        aligned_free(dst_next_table);
        return std::unexpected(result.error());
      }

      // 设置目标页表项指向新的子页表
      dst_table[i] = cpu_io::virtual_memory::PhysicalToPageTableEntry(
          reinterpret_cast<uint64_t>(dst_next_table),
          cpu_io::virtual_memory::GetTableEntryPermissions());
    } else {
      // 最后一级页表
      if (copy_mappings) {
        // 直接复制页表项（共享物理页）
        dst_table[i] = src_pte;
      }
      // 如果不复制映射，保持目标页表项为 0
    }
  }

  return {};
}

auto VirtualMemory::FindPageTableEntry(void* page_dir, void* virtual_addr,
                                       bool allocate) -> Expected<uint64_t*> {
  auto* current_table = reinterpret_cast<uint64_t*>(page_dir);
  auto vaddr = reinterpret_cast<uint64_t>(virtual_addr);

  // 遍历页表层级
  for (size_t level = cpu_io::virtual_memory::kPageTableLevels - 1; level > 0;
       --level) {
    // 获取当前级别的虚拟页号
    auto vpn = cpu_io::virtual_memory::GetVirtualPageNumber(vaddr, level);
    auto* pte = &current_table[vpn];
    if (cpu_io::virtual_memory::IsPageTableEntryValid(*pte)) {
      // 页表项有效，获取下一级页表
      current_table = reinterpret_cast<uint64_t*>(
          cpu_io::virtual_memory::PageTableEntryToPhysical(*pte));
    } else {
      // 页表项无效
      if (allocate) {
        auto* new_table = aligned_alloc(cpu_io::virtual_memory::kPageSize,
                                        cpu_io::virtual_memory::kPageSize);
        if (new_table == nullptr) {
          return std::unexpected(Error(ErrorCode::kVmAllocationFailed));
        }
        // 清零新页表
        std::memset(new_table, 0, cpu_io::virtual_memory::kPageSize);

        // 设置中间页表项
        *pte = cpu_io::virtual_memory::PhysicalToPageTableEntry(
            reinterpret_cast<uint64_t>(new_table),
            cpu_io::virtual_memory::GetTableEntryPermissions());

        current_table = reinterpret_cast<uint64_t*>(new_table);
      } else {
        return std::unexpected(Error(ErrorCode::kVmPageNotMapped));
      }
    }
  }

  // 返回最底层页表中的页表项
  auto vpn = cpu_io::virtual_memory::GetVirtualPageNumber(vaddr, 0);

  return &current_table[vpn];
}
