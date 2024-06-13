
/**
 * @file kernel_fdt.hpp
 * @brief 用于解析内核自身的 fdt 解析
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_FDT_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_FDT_HPP_

// 禁用 GCC/Clang 的警告
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include <libfdt.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <cstddef>
#include <cstdint>
#include <utility>

#include "cstdio"
#include "cstring"

/**
 * elf 文件相关
 */
class KernelFdt {
 public:
  /**
   * 构造函数
   * @param fdt_addr fdt 地址
   */
  explicit KernelFdt(uint64_t fdt_addr) : fdt_addr_((void *)fdt_addr) {
    if (!fdt_addr_) {
      printf("Fatal Error: Invalid fdt_addr.\n");
      return;
    }

    // 检查 fdt 头数据
    if (fdt_check_header(fdt_addr_) != 0) {
      printf("Invalid device tree blob\n");
      return;
    }
  }

  /// @name 构造/析构函数
  /// @{
  KernelFdt() = default;
  KernelFdt(const KernelFdt &) = default;
  KernelFdt(KernelFdt &&) = default;
  auto operator=(const KernelFdt &) -> KernelFdt & = default;
  auto operator=(KernelFdt &&) -> KernelFdt & = default;
  ~KernelFdt() = default;
  /// @}

  /**
   * 获取内存信息
   * @return 内存信息<地址，长度>
   */
  std::pair<uint64_t, size_t> GetMemory() {
    uint64_t base = 0;
    uint64_t size = 0;

    int len = 0;

    // 找到 /memory 节点
    auto offset = fdt_path_offset(fdt_addr_, "/memory");
    if (offset < 0) {
      printf("Error finding /memory node: %s\n", fdt_strerror(offset));
      return {};
    }

    // 获取 reg 属性
    auto prop = fdt_get_property(fdt_addr_, offset, "reg", &len);
    if (!prop) {
      printf("Error finding reg property: %s\n", fdt_strerror(len));
      return {};
    }

    // 解析 reg 属性，通常包含基地址和大小
    auto reg = (const uint64_t *)prop->data;
    for (size_t i = 0; i < len / sizeof(uint64_t); i += 2) {
      base = fdt64_to_cpu(reg[i]);
      size = fdt64_to_cpu(reg[i + 1]);
    }
    return {base, size};
  }

  /**
   * 获取串口信息
   * @return 内存信息<地址，长度>
   */
  std::pair<uint64_t, size_t> GetSerial() {
    uint64_t base = 0;
    uint64_t size = 0;

    int len = 0;

    // 找到 /memory 节点
    auto offset = fdt_path_offset(fdt_addr_, "/soc/serial");
    if (offset < 0) {
      printf("Error finding /soc/serial node: %s\n", fdt_strerror(offset));
      return {};
    }

    // 获取 reg 属性
    auto prop = fdt_get_property(fdt_addr_, offset, "reg", &len);
    if (!prop) {
      printf("Error finding reg property: %s\n", fdt_strerror(len));
      return {};
    }

    // 解析 reg 属性，通常包含基地址和大小
    auto reg = (const uint64_t *)prop->data;
    for (size_t i = 0; i < len / sizeof(uint64_t); i += 2) {
      base = fdt64_to_cpu(reg[i]);
      size = fdt64_to_cpu(reg[i + 1]);
    }
    return {base, size};
  }

 private:
  void *fdt_addr_;
};

/// 全局 elf 对象，需要在相应体系结构初始化时重新初始化
[[maybe_unused]] static KernelFdt kKernelFdt;

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_FDT_HPP_ */
