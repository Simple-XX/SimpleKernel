
/**
 * @file basic_info.hpp
 * @brief 基础信息
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_BASIC_INFO_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_BASIC_INFO_HPP_

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "singleton.hpp"

struct BasicInfo {
  /// physical_memory 地址
  uint64_t physical_memory_addr;
  /// physical_memory 大小
  size_t physical_memory_size;

  /// kernel 地址
  uint64_t kernel_addr;
  /// kernel 大小
  size_t kernel_size;

  /// elf 地址
  uint64_t elf_addr;
  /// elf 大小
  size_t elf_size;

  /**
   * 构造函数，在 arch_main.cpp 中定义
   * @param argc 同 _start
   * @param argv 同 _start
   */
  explicit BasicInfo(uint32_t argc, uint8_t *argv);

  /// @name 构造/析构函数
  /// @{
  BasicInfo() = default;
  BasicInfo(const BasicInfo &) = default;
  BasicInfo(BasicInfo &&) = default;
  auto operator=(const BasicInfo &) -> BasicInfo & = default;
  auto operator=(BasicInfo &&) -> BasicInfo & = default;
  ~BasicInfo() = default;
  /// @}

  friend std::ostream &operator<<(std::ostream &os,
                                  const BasicInfo &basic_info) {
    printf("physical_memory_addr: 0x%X, size 0x%X.\n",
           basic_info.physical_memory_addr, basic_info.physical_memory_size);
    printf("kernel_addr: 0x%X, size 0x%X.\n", basic_info.kernel_addr,
           basic_info.kernel_size);
    printf("elf_addr: 0x%X, size 0x%X\n", basic_info.elf_addr,
           basic_info.elf_size);
    return os;
  }
};

/// 保存基本信息
[[maybe_unused]] static Singleton<BasicInfo> kBasicInfo;

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_BASIC_INFO_HPP_ */
