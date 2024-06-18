
/**
 * @file cpu.hpp
 * @brief aarch64 cpu 相关定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2024-03-05
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2024-03-05<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_AARCH64_INCLUDE_CPU_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_AARCH64_INCLUDE_CPU_HPP_

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <typeinfo>

#include "cstdio"
#include "iostream"
#include "kernel_log.hpp"

/**
 * aarch64 cpu 相关定义
 * @note 寄存器读写设计见 arch/README.md
 */
namespace cpu {

// 第一部分：寄存器定义
namespace reginfo {

struct RegInfoBase {
  /// 寄存器数据类型
  using DataType = uint64_t;
  /// 起始位
  static constexpr uint64_t kBitOffset = 0;
  /// 位宽
  static constexpr uint64_t kBitWidth = 64;
  /// 掩码，(val & kBitMask) == 对应当前位的值
  static constexpr uint64_t kBitMask =
      (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) << kBitOffset : ~0ULL;
  /// 对应位置位掩码
  static constexpr uint64_t kAllSetMask =
      (kBitWidth < 64) ? ((1ULL << kBitWidth) - 1) : ~0ULL;
};

/// 通用寄存器
struct X29Info : public RegInfoBase {};

};  // namespace reginfo

// 第二部分：读/写模版实现
namespace {
/**
 * 只读接口
 * @tparam 寄存器类型
 */
template <class Reg>
class ReadOnlyRegBase {
 public:
  /// @name 构造/析构函数
  /// @{
  ReadOnlyRegBase() = default;
  ReadOnlyRegBase(const ReadOnlyRegBase &) = delete;
  ReadOnlyRegBase(ReadOnlyRegBase &&) = delete;
  auto operator=(const ReadOnlyRegBase &) -> ReadOnlyRegBase & = delete;
  auto operator=(ReadOnlyRegBase &&) -> ReadOnlyRegBase & = delete;
  ~ReadOnlyRegBase() = default;
  /// @}

  /**
   * 读寄存器
   * @return Reg::DataType 寄存器的值
   */
  static __always_inline Reg::DataType Read() {
    typename Reg::DataType value{};
    if constexpr (std::is_same<Reg, reginfo::X29Info>::value) {
      __asm__ volatile("mov %0, x29" : "=r"(value) : :);
    } else {
      Err("No Type\n");
      throw;
    }
    return value;
  }

  /**
   * () 重载
   */
  static __always_inline Reg::DataType operator()() { return Read(); }
};

/**
 * 只写接口
 * @tparam 寄存器类型
 */
template <class Reg>
class WriteOnlyRegBase {
 public:
  /// @name 构造/析构函数
  /// @{
  WriteOnlyRegBase() = default;
  WriteOnlyRegBase(const WriteOnlyRegBase &) = delete;
  WriteOnlyRegBase(WriteOnlyRegBase &&) = delete;
  auto operator=(const WriteOnlyRegBase &) -> WriteOnlyRegBase & = delete;
  auto operator=(WriteOnlyRegBase &&) -> WriteOnlyRegBase & = delete;
  ~WriteOnlyRegBase() = default;
  /// @}

  /**
   * 写寄存器
   * @param value 要写的值
   */
  static __always_inline void Write(Reg::DataType value) {
    if constexpr (std::is_same<Reg, reginfo::X29Info>::value) {
      __asm__ volatile("mv fp, %0" : : "r"(value) :);
    } else {
      Err("No Type\n");
      throw;
    }
  }
};

/**
 * 读写接口
 * @tparam 寄存器类型
 */
template <class Reg>
class ReadWriteRegBase : public ReadOnlyRegBase<Reg>,
                         public WriteOnlyRegBase<Reg> {
 public:
  /// @name 构造/析构函数
  /// @{
  ReadWriteRegBase() = default;
  ReadWriteRegBase(const ReadWriteRegBase &) = delete;
  ReadWriteRegBase(ReadWriteRegBase &&) = delete;
  auto operator=(const ReadWriteRegBase &) -> ReadWriteRegBase & = delete;
  auto operator=(ReadWriteRegBase &&) -> ReadWriteRegBase & = delete;
  ~ReadWriteRegBase() = default;
  /// @}
};

// 第三部分：寄存器实例
class X29 : public ReadWriteRegBase<reginfo::X29Info> {
 public:
  friend std::ostream &operator<<(std::ostream &os, const X29 &x29) {
    printf("val: 0x%p", (void *)x29.Read());
    return os;
  }
};

/// 通用寄存器
struct AllXreg {
  X29 x29;
};

};  // namespace

// 第四部分：访问接口
[[maybe_unused]] static AllXreg kAllXreg;

};  // namespace cpu

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_AARCH64_INCLUDE_CPU_HPP_
