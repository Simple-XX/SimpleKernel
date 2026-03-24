/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/singleton.h>

#include <cstddef>
#include <cstdint>

#include "kernel_log.hpp"

// 引用链接脚本中的变量
/// @see http://wiki.osdev.org/Using_Linker_Script_Values
/// 内核开始
extern "C" void* __executable_start[];
/// 代码段结束
extern "C" void* __etext[];
/// 内核结束
extern "C" void* end[];
/// 内核入口，在 boot.S 中定义
extern "C" void _boot();

/// @brief 内核基础信息（物理内存、内核地址、CPU 核数等）
struct BasicInfo {
  /// physical_memory 地址
  uint64_t physical_memory_addr{0};
  /// physical_memory 大小
  size_t physical_memory_size{0};

  /// kernel 地址
  uint64_t kernel_addr{0};
  /// kernel 大小
  size_t kernel_size{0};

  /// elf 地址
  uint64_t elf_addr{0};

  /// fdt 地址
  uint64_t fdt_addr{0};

  /// cpu 核数
  size_t core_count{0};

  /// 时钟频率
  size_t interval{0};

  /// @name 构造/析构函数
  /// @{
  /**
   * 构造函数，在 arch_main.cpp 中定义
   * @param argc 同 _start
   * @param argv 同 _start
   */
  explicit BasicInfo(int argc, const char** argv);

  BasicInfo() = default;
  BasicInfo(const BasicInfo&) = default;
  BasicInfo(BasicInfo&&) = default;
  auto operator=(const BasicInfo&) -> BasicInfo& = default;
  auto operator=(BasicInfo&&) -> BasicInfo& = default;
  ~BasicInfo() = default;
  /// @}
};

using BasicInfoSingleton = etl::singleton<BasicInfo>;
