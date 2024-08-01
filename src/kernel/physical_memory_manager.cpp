
/**
 * @file physical_memory_manager.cpp
 * @brief 物理内存管理
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#include "memory/physical_memory_manager.h"

#include <cstdio>
#include <cstring>

#include "basic_info.hpp"
#include "kernel_elf.hpp"
#include "kernel_fdt.hpp"
#include "kernel_log.hpp"
#include "memory/firstfit_allocator.h"
#include "project_config.h"
// #include "resource.h"

PhysicalMemoryManager::PhysicalMemoryManager(uint64_t addr, size_t pages_count)
    : addr_(addr), pages_count_(pages_count) {
  // 内核空间地址开始
  kernel_addr_ = kBasicInfo.GetInstance().kernel_addr;
  // 长度手动指定
  kernel_pages_count_ = kKernelSpaceSize / kPageSize;
  // 非内核空间在内核空间结束后
  user_start_ = kBasicInfo.GetInstance().kernel_addr + kKernelSpaceSize;
  // 长度为总长度减去内核长度
  user_pages_count_ = pages_count_ - kernel_pages_count_;

  // 创建分配器
  // 内核空间
  static FirstFitAllocator first_fit_allocator_kernel(
      "First Fit Allocator(Kernel space)", kernel_addr_, kernel_pages_count_);
  kernel_allocator_ = (AllocatorBase*)&first_fit_allocator_kernel;
  // 用户空间
  static FirstFitAllocator first_fit_allocator(
      "First Fit Allocator(User space)", user_start_, user_pages_count_);
  user_allocator_ = (AllocatorBase*)&first_fit_allocator;

  // 内核占用页数
  auto kernel_pages = kBasicInfo.GetInstance().kernel_size / kPageSize;
  if (kBasicInfo.GetInstance().kernel_size % kPageSize != 0) {
    kernel_pages++;
  }
  // 将内核已使用部进行分配
  AllocKernelPagesAt(kBasicInfo.GetInstance().kernel_addr, kernel_pages);
  MoveElfDtb();
}

size_t PhysicalMemoryManager::GetPagesCount() const { return pages_count_; }

uint64_t PhysicalMemoryManager::GetKernelSpaceAddr() const {
  return kernel_addr_;
}

size_t PhysicalMemoryManager::GetKernelSpacePagesCount() const {
  return kernel_pages_count_;
}

uint64_t PhysicalMemoryManager::GetUserSpaceAddr() const { return user_start_; }

size_t PhysicalMemoryManager::GetUserSpacePagesCount() const {
  return user_pages_count_;
}

size_t PhysicalMemoryManager::GetUsedPagesCount() const {
  return kernel_allocator_->GetUsedCount() + user_allocator_->GetUsedCount();
}

size_t PhysicalMemoryManager::GetFreePagesCount() const {
  return kernel_allocator_->GetFreeCount() + user_allocator_->GetFreeCount();
}

uint64_t PhysicalMemoryManager::AllocUserPage() {
  return user_allocator_->Alloc(1);
}

uint64_t PhysicalMemoryManager::AllocUserPages(size_t pages_count) {
  return user_allocator_->Alloc(pages_count);
}

bool PhysicalMemoryManager::AllocUserPagesAt(uint64_t addr,
                                             size_t pages_count) {
  bool ret = user_allocator_->AllocAt(addr, pages_count);
  return ret;
}

uint64_t PhysicalMemoryManager::AllocKernelPage() {
  return kernel_allocator_->Alloc(1);
}

uint64_t PhysicalMemoryManager::AllocKernelPages(size_t pages_count) {
  return kernel_allocator_->Alloc(pages_count);
}

bool PhysicalMemoryManager::AllocKernelPagesAt(uint64_t addr,
                                               size_t pages_count) {
  return kernel_allocator_->AllocAt(addr, pages_count);
}

void PhysicalMemoryManager::FreePage(uint64_t addr) {
  // 判断应该使用哪个分配器
  if (addr >= kernel_addr_ && addr < kernel_addr_ + kernel_pages_count_) {
    kernel_allocator_->Free(addr, 1);
  } else if (addr >= user_start_ && addr < user_start_ + user_pages_count_) {
    user_allocator_->Free(addr, 1);
  }
}

void PhysicalMemoryManager::FreePages(uint64_t addr, size_t pages_count) {
  // 判断应该使用哪个分配器
  if (addr >= kernel_addr_ && addr < kernel_addr_ + kernel_pages_count_) {
    kernel_allocator_->Free(addr, pages_count);
  } else if (addr >= user_start_ && addr < user_start_ + user_pages_count_) {
    user_allocator_->Free(addr, pages_count);
  }
}

void PhysicalMemoryManager::MoveElfDtb() {
  // 重新初始化
  if (kBasicInfo.GetInstance().elf_addr != 0) {
    auto old_elf_addr = kBasicInfo.GetInstance().elf_addr;
    // 计算需要多少页
    auto elf_pages = kBasicInfo.GetInstance().elf_size / kPageSize;
    if (kBasicInfo.GetInstance().elf_size % kPageSize != 0) {
      elf_pages++;
    }
    // 申请空间
    auto new_elf_addr = AllocKernelPages(elf_pages);
    // 复制过来，完成后以前的内存就可以使用了
    memcpy((void*)new_elf_addr, (void*)old_elf_addr, elf_pages * kPageSize);
    // 更新 kBasicInfo 信息
    kBasicInfo.GetInstance().elf_addr = new_elf_addr;
    kKernelElf.GetInstance() = KernelElf(kBasicInfo.GetInstance().elf_addr,
                                         kBasicInfo.GetInstance().elf_size);
  }
  if (kBasicInfo.GetInstance().fdt_addr != 0) {
    auto old_fdt_addr = kBasicInfo.GetInstance().fdt_addr;
    auto fdt_pages = 1;
    auto new_fdt_addr = AllocKernelPages(fdt_pages);
    memcpy((void*)new_fdt_addr, (void*)old_fdt_addr, fdt_pages * kPageSize);
    kBasicInfo.GetInstance().fdt_addr = new_fdt_addr;
    kKernelFdt.GetInstance() = KernelFdt(kBasicInfo.GetInstance().fdt_addr);
  }
}

uint32_t PhysicalMemoryInit(uint32_t, uint8_t*) {
  // 初始化物理内存管理器
  kPhysicalMemoryManager.GetInstance() =
      PhysicalMemoryManager(kBasicInfo.GetInstance().physical_memory_addr,
                            kBasicInfo.GetInstance().physical_memory_size);

  log::Info("Hello PhysicalMemoryInit\n");

  return 0;
}
