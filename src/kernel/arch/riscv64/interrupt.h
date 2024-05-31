
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

class Interrupt final : public InterruptBase {
 public:
  Interrupt();

  /// @name 构造/析构函数
  /// @{
  Interrupt(const Interrupt &) = delete;
  Interrupt(Interrupt &&) = delete;
  auto operator=(const Interrupt &) -> Interrupt & = delete;
  auto operator=(Interrupt &&) -> Interrupt & = delete;
  ~Interrupt() = default;
  /// @}

  /**
   * @brief 执行中断处理
   * @param  cause 中断或异常号
   * @param  context 中断上下文
   */
  void Do(uint64_t cause, uint8_t *context) override;

  /**
   * @brief 注册中断处理函数
   * @param scause 中断原因
   * @param func 处理函数
   */
  void RegisterInterruptFunc(uint64_t scause, InterruptFunc func) override;

 private:
  /// 中断处理函数数组
  static InterruptFunc interrupt_handlers[Cpu::kInterruptMaxCount]
      __attribute__((aligned(4)));
  /// 异常处理函数数组
  static InterruptFunc exception_handlers[Cpu::kExceptionMaxCount]
      __attribute__((aligned(4)));
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_H_ */
