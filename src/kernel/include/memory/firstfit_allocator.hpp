
/**
 * @file firstfit_allocator.hpp
 * @brief firstfit 内存分配器头文件
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_FIRSTFIT_ALLOCATOR_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_FIRSTFIT_ALLOCATOR_HPP_

#include <bitset>
#include <cstddef>
#include <cstdint>

#include "kernel_log.hpp"
#include "memory/allocator_base.hpp"

/**
 * @brief 使用 first fit 算法的分配器
 * @tparam page_size 页大小，默认 0x1000，即 4096 bytes
 */
template <size_t page_size = 0x1000>
class FirstFitAllocator : public AllocatorBase {
 public:
  /**
   * @brief 构造函数
   * @param name 分配器名称
   * @param addr 开始地址
   * @param pages_count 页数量
   */
  explicit FirstFitAllocator(const char *name, uint64_t addr,
                             size_t pages_count)
      : AllocatorBase(name, addr, pages_count) {
    if (addr % page_size != 0) {
      klog::Err("addr not aligned. 0x%lX\n", addr);
      throw;
    }
    printf("%s: 0x%p(0x%X pages) init.\n", name_, addr_, length_);
  }

  /// @name 构造/析构函数
  /// @{
  FirstFitAllocator() = default;
  FirstFitAllocator(const FirstFitAllocator &) = default;
  FirstFitAllocator(FirstFitAllocator &&) = default;
  auto operator=(const FirstFitAllocator &) -> FirstFitAllocator & = default;
  auto operator=(FirstFitAllocator &&) -> FirstFitAllocator & = default;
  ~FirstFitAllocator() = default;
  /// @}

  /**
   * @brief 分配长度为 pages_count 页的内存
   * @param  pages_count 页数
   * @return uint64_t 分配的内存起点地址
   */
  uint64_t Alloc(size_t pages_count) override {
    uint64_t res_addr = 0;
    // 在位图中寻找连续 pages_count 的位置
    auto [is_found, idx] = Find(pages_count, false);
    if (is_found == false) {
      klog::Warn("NO ENOUGH MEM %d.\n", pages_count);
      return res_addr;
    }
    // 遍历区域
    for (auto i = idx; i < idx + pages_count; i++) {
      // 置位，说明已使用
      map_[i] = 1;
    }
    // 计算实际地址
    // 分配器起始地址+页长度*第几页
    res_addr = addr_ + (page_size * idx);
    // 更新统计信息
    free_length_ -= pages_count;
    used_length_ += pages_count;
    return res_addr;
  }

  /**
   * @brief 在 addr 处分配长度为 pages_count 页的内存
   * @param addr 指定的地址
   * @param pages_count 页数
   * @return true 成功
   * @return false 失败
   */
  bool AllocAt(uint64_t addr, size_t pages_count) override {
    // 页对齐
    if (addr % page_size != 0) {
      klog::Warn("addr not aligned 0x%lX.\n", addr);
      return false;
    }
    // 申请地址超出范围
    if (addr < addr_ || addr > addr_ ||
        addr + pages_count * page_size > addr_ + length_ * page_size) {
      klog::Warn("out of range 0x%lX %d.\n", addr, pages_count);
      return false;
    }
    // 计算 addr 在 map_ 中的索引
    size_t idx = (addr - addr_) / page_size;
    // 遍历
    for (auto i = idx; i < idx + pages_count; i++) {
      // 如果在范围内有已经分配的内存，返回 false
      if (map_[i] == true) {
        return false;
      }
    }
    // 到这里说明范围内没有已使用内存
    // 再次遍历
    for (auto i = idx; i < idx + pages_count; i++) {
      // 置位
      map_[i] = 1;
    }
    // 更新统计信息
    free_length_ -= pages_count;
    used_length_ += pages_count;
    return true;
  }

  /**
   * @brief 释放 addr 处 pages_count 页的内存
   * @param addr 要释放内存起点地址
   * @param pages_count 页数
   */
  void Free(uint64_t addr, size_t pages_count) override {
    // 页对齐
    if (addr % page_size != 0) {
      klog::Warn("addr not aligned 0x%lX.\n", addr);
      return;
    }
    // 申请地址超出范围
    if (addr < addr_ || addr > addr_ ||
        addr + pages_count * page_size > addr_ + length_ * page_size) {
      klog::Warn("out of range 0x%lX %d.\n", addr, pages_count);
      return;
    }
    // 计算 addr 在 map_ 中的索引
    size_t idx = (addr - addr_) / page_size;
    for (auto i = idx; i < idx + pages_count; i++) {
      map_[i] = 0;
    }
    // 更新统计信息
    free_length_ += pages_count;
    used_length_ -= pages_count;
  }

 private:
  /// 位图，每一位表示一页内存，1 表示已使用，0 表示未使用
  std::bitset<page_size * sizeof(uint64_t)> map_;

  /**
   * @brief 寻找连续 pages_count 个 val 位，返回开始索引
   * @param pages_count  连续页数量
   * @param val 要寻找的页状态
   * @return std::pair<bool, size_t> 找到返回 <true, 索引>，失败返回 <false, 0>
   */
  std::pair<bool, size_t> Find(size_t pages_count, bool val) const {
    size_t count = 0;
    size_t idx = 0;
    // 遍历位图
    for (uint64_t i = 0; i < length_; i++) {
      if (map_[i] != val) {
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
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_FIRSTFIT_ALLOCATOR_HPP_ */
