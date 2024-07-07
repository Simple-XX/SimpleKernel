
/**
 * @file firstfit_allocator.h
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_FIRSTFIT_ALLOCATOR_BASE_H_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_FIRSTFIT_ALLOCATOR_BASE_H_

#include <bitset>
#include <cstddef>
#include <cstdint>

#include "memory/allocator_base.hpp"

/**
 * @brief 使用 first fit 算法的分配器
 */
class FirstFitAllocator : AllocatorBase {
 public:
  /**
   * @brief 构造函数
   * @param name 分配器名称
   * @param addr 开始地址
   * @param pages_count 页数量
   */
  explicit FirstFitAllocator(const char *name, uint64_t addr,
                             size_t pages_count);

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
  uint64_t Alloc(size_t pages_count) override;

  /**
   * @brief 在 addr 处分配长度为 pages_count 页的内存
   * @param addr 指定的地址
   * @param pages_count 页数
   * @return true 成功
   * @return false 失败
   */
  bool AllocAt(uint64_t addr, size_t pages_count) override;

  /**
   * @brief 释放 addr 处 pages_count 页的内存
   * @param addr 要释放内存起点地址
   * @param pages_count 页数
   */
  void Free(uint64_t addr, size_t pages_count) override;

 private:
  static constexpr const uint64_t PAGE_SIZE = 0x1000;

  /// 位图，每一位表示一页内存，1 表示已使用，0 表示未使用
  std::bitset<PAGE_SIZE * sizeof(uint64_t)> map;

  /**
   * @brief 寻找连续 pages_count 个 val 位，返回开始索引
   * @param pages_count  连续页数量
   * @param val 要寻找的页状态
   * @return std::pair<bool, size_t> 找到返回 <true, 索引>，失败返回 <false, 0>
   */
  std::pair<bool, size_t> Find(size_t pages_count, bool val) const;
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_FIRSTFIT_ALLOCATOR_BASE_H_ */
