
/**
 * @file cpu.hpp
 * @brief riscv64 cpu 相关定义
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_INCLUDE_CPU_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_INCLUDE_CPU_HPP_

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <typeinfo>

#include "cstdio"
#include "iostream"
#include "kernel_log.hpp"

/**
 * riscv64 cpu 相关定义
 * @note 分为四个部分
 * 1. 寄存器定义
 *    如果寄存器有特殊的位域含义(如 csr 寄存器)，参考 RegInfoBase 给出
 * 2. 读/写模版实现
 *    针对通用寄存器只实现 Read/Write 接口
 *    csr 寄存器还有 ReadImm/WriteImm/ReadWrite/ReadWriteImm 等接口
 *    调用时根据寄存器类型选择对应的模版实现
 * 3. 寄存器实例
 *    通过向读写模版传递寄存器定义，生成对应的实例
 * 4. 访问接口
 *    通过访问接口访问寄存器
 */
namespace cpu {

/// 第一部分：寄存器定义
namespace reginfo {

struct RegInfoBase {
  using DataType = uint64_t;
  static constexpr uint64_t kBitOffset = 0;
  static constexpr uint64_t kBitWidth = 64;
  static constexpr uint64_t kBitMask = ~0;
  static constexpr uint64_t kAllSetMask = ~0;
};

/// 通用寄存器
struct FpInfo : public RegInfoBase {};

};  // namespace reginfo

/// 第二部分：读/写模版实现
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
   * @return uint64_t 寄存器的值
   */
  static __always_inline uint64_t Read() {
    uint64_t value = -1;
    if constexpr (std::is_same<Reg, reginfo::FpInfo>::value) {
      __asm__ volatile("mv %0, fp" : "=r"(value) : :);
    } else {
      Err("No Type\n");
      throw;
    }
    return value;
  }

  /**
   * () 重载
   */
  static __always_inline uint64_t operator()() { return Read(); }
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
  static __always_inline void Write(uint64_t value) {
    if constexpr (std::is_same<Reg, reginfo::FpInfo>::value) {
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

/// 第三部分：寄存器实例
class Fp : public ReadWriteRegBase<reginfo::FpInfo> {
 public:
  /// @name 构造/析构函数
  /// @{
  Fp() = default;
  Fp(const Fp &) = delete;
  Fp(Fp &&) = delete;
  auto operator=(const Fp &) -> Fp & = delete;
  auto operator=(Fp &&) -> Fp & = delete;
  virtual ~Fp() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os, const Fp &fp) {
    printf("val: 0x%p", fp.Read());
    return os;
  }
};

/// 第四部分：访问接口
/**
 * @brief 通用寄存器
 */
class AllXreg {
 public:
  Fp fp;

  /// @name 构造/析构函数
  /// @{
  AllXreg() = default;
  AllXreg(const AllXreg &) = delete;
  AllXreg(AllXreg &&) = delete;
  auto operator=(const AllXreg &) -> AllXreg & = delete;
  auto operator=(AllXreg &&) -> AllXreg & = delete;
  virtual ~AllXreg() = default;
  /// @}
};

static AllXreg kAllXreg;

};  // namespace cpu

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_INCLUDE_CPU_HPP_
