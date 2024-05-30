
/**
 * @file interrupt.h
 * @brief 中断处理
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_H_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_H_

#include <cstdint>

#include "cpu.hpp"
#include "interrupt_base.h"
#include "stdio.h"

/**
 * @brief core-local interrupt controller
 * 本地核心中断控制器
 * 用于控制 excp 与 intr
 */
class Clint {
 public:
  Clint() {
    // 开启内部中断
    Cpu::WriteSie(Cpu::ReadSie() | Cpu::kSieSsie);
    printf("Clint init.\n");
  }

  /// @name 构造/析构函数
  /// @{
  Clint(const Clint &) = default;
  Clint(Clint &&) = default;
  auto operator=(const Clint &) -> Clint & = default;
  auto operator=(Clint &&) -> Clint & = default;
  ~Clint() = default;
  /// @}
};

class Interrupt : public InterruptBase {
 public:
  Interrupt() { Clint(); }

  /// @name 构造/析构函数
  /// @{
  Interrupt(const Interrupt &) = default;
  Interrupt(Interrupt &&) = default;
  auto operator=(const Interrupt &) -> Interrupt & = default;
  auto operator=(Interrupt &&) -> Interrupt & = default;
  ~Interrupt() = default;
  /// @}
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_H_ */
