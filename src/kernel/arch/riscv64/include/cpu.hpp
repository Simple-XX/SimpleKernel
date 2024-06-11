
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

namespace cpu {

/**
 * 读 fp 寄存器
 * @return fp 寄存器的值
 */
static __always_inline uint64_t ReadFp() {
  uint64_t fp = -1;
  __asm__ volatile("mv %0, fp" : "=r"(fp));
  return fp;
}

};  // namespace cpu

#endif  // SIMPLEKERNEL_SRC_KERNEL_ARCH_RISCV64_INCLUDE_CPU_HPP_
