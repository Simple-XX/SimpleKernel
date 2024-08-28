
/**
 * @file allocator_base.hpp
 * @brief 内存分配器基类
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_ALLOCATOR_BASE_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_ALLOCATOR_BASE_HPP_

#include <cstddef>
#include <cstdint>

/**
 * @brief 内存分配器基类
 */
class AllocatorBase {
 public:
  /**
   * @brief 构造函数
   * @param name 分配器名
   * @param addr 要管理的内存开始地址
   * @param length 要管理的内存长度，单位由派生类型指定
   */
  explicit AllocatorBase(const char *name, uint64_t addr, size_t length) {
    name_ = name;
    addr_ = addr;
    length_ = length;
    free_length_ = length;
    used_length_ = 0;
  }

  /// @name 构造/析构函数
  /// @{
  AllocatorBase() = default;
  AllocatorBase(const AllocatorBase &) = default;
  AllocatorBase(AllocatorBase &&) = default;
  auto operator=(const AllocatorBase &) -> AllocatorBase & = default;
  auto operator=(AllocatorBase &&) -> AllocatorBase & = default;
  virtual ~AllocatorBase() = default;
  /// @}

  /**
   * @brief 分配 length 内存
   * @param length 要申请的内存长度
   * @return uint64_t 分配到的地址
   */
  virtual uint64_t Alloc(size_t length) = 0;

  /**
   * @brief 在指定地址分配 length 长度
   * @param addr 指定的地址
   * @param length 长度
   * @return true 成功
   * @return false 失败
   */
  virtual bool AllocAt(uint64_t addr, size_t length) = 0;

  /**
   * @brief 释放内存
   * @param addr 地址
   * @param length 长度
   */
  virtual void Free(uint64_t addr, size_t length) = 0;

  /**
   * @brief 已使用内存数量
   * @return size_t 数量
   */
  virtual size_t GetUsedCount() const { return used_length_; }

  /**
   * @brief 空闲内存数量
   * @return size_t 数量
   */
  virtual size_t GetFreeCount() const { return free_length_; }

 protected:
  /// 分配器名称
  const char *name_;
  /// 当前管理的内存区域地址
  uint64_t addr_;
  /// 当前管理的内存区域长度
  size_t length_;
  /// 空闲内存数量
  size_t free_length_;
  /// 已使用内存数量
  size_t used_length_;
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_MEMORY_ALLOCATOR_BASE_HPP_ */
