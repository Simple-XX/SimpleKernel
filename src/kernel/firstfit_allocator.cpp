
/**
 * @file firstfit_allocator.cpp
 * @brief firstfit 内存分配器实现
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

#include "memory/firstfit_allocator.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "kernel_log.hpp"

FirstFitAllocator::FirstFitAllocator(const char* name, uint64_t addr,
                                     size_t pages_count)
    : AllocatorBase(name, addr, pages_count) {
  if (addr % PAGE_SIZE != 0) {
    log::Err("addr not aligned. 0x%lX\n", addr);
    throw;
  }
  printf("%s: 0x%p(0x%X pages) init.\n", name_, addr_, length_);
}

uint64_t FirstFitAllocator::Alloc(size_t pages_count) {
  uint64_t res_addr = 0;
  // 在位图中寻找连续 pages_count 的位置
  auto [is_found, idx] = Find(pages_count, false);
  if (is_found == false) {
    log::Warn("NO ENOUGH MEM %d.\n", pages_count);
    return res_addr;
  }
  // 遍历区域
  for (auto i = idx; i < idx + pages_count; i++) {
    // 置位，说明已使用
    map[i] = 1;
  }
  // 计算实际地址
  // 分配器起始地址+页长度*第几页
  res_addr = addr_ + (PAGE_SIZE * idx);
  // 更新统计信息
  free_length_ -= pages_count;
  used_length_ += pages_count;
  return res_addr;
}

bool FirstFitAllocator::AllocAt(uint64_t addr, size_t pages_count) {
  // 页对齐
  if (addr % PAGE_SIZE != 0) {
    log::Warn("addr not aligned 0x%lX.\n", addr);
    return false;
  }
  // 申请地址超出范围
  if (addr < addr_ || addr > addr_ ||
      addr + pages_count * PAGE_SIZE > addr_ + length_ * PAGE_SIZE) {
    log::Warn("out of range 0x%lX %d.\n", addr, pages_count);
    return false;
  }
  // 计算 addr 在 map 中的索引
  size_t idx = (addr - addr_) / PAGE_SIZE;
  // 遍历
  for (auto i = idx; i < idx + pages_count; i++) {
    // 如果在范围内有已经分配的内存，返回 false
    if (map[i] == true) {
      return false;
    }
  }
  // 到这里说明范围内没有已使用内存
  // 再次遍历
  for (auto i = idx; i < idx + pages_count; i++) {
    // 置位
    map[i] = 1;
  }
  // 更新统计信息
  free_length_ -= pages_count;
  used_length_ += pages_count;
  return true;
}

void FirstFitAllocator::Free(uint64_t addr, size_t pages_count) {
  // 页对齐
  if (addr % PAGE_SIZE != 0) {
    log::Warn("addr not aligned 0x%lX.\n", addr);
    return;
  }
  // 申请地址超出范围
  if (addr < addr_ || addr > addr_ ||
      addr + pages_count * PAGE_SIZE > addr_ + length_ * PAGE_SIZE) {
    log::Warn("out of range 0x%lX %d.\n", addr, pages_count);
    return;
  }
  // 计算 addr 在 map 中的索引
  size_t idx = (addr - addr_) / PAGE_SIZE;
  for (auto i = idx; i < idx + pages_count; i++) {
    map[i] = 0;
  }
  // 更新统计信息
  free_length_ += pages_count;
  used_length_ -= pages_count;
}

std::pair<bool, size_t> FirstFitAllocator::Find(size_t pages_count,
                                                bool val) const {
  size_t count = 0;
  size_t idx = 0;
  // 遍历位图
  for (uint64_t i = 0; i < length_; i++) {
    if (map[i] != val) {
      count = 0;
      idx = i + 1;
    } else {
      count++;
    }
    if (count == pages_count) {
      return {true, idx};
    }
  }
  return {false, 0};
}
