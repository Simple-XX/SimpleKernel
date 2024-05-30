
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
#include <functional>

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
  Clint();

  /// @name 构造/析构函数
  /// @{
  Clint(const Clint &) = delete;
  Clint(Clint &&) = delete;
  auto operator=(const Clint &) -> Clint & = delete;
  auto operator=(Clint &&) -> Clint & = delete;
  ~Clint() = default;
  /// @}
};

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

  uint32_t DoInterrupt(uint32_t, uint8_t *) override;
  uint32_t DoException(uint32_t, uint8_t *) override;

 private:
  // typedef std::function<uint32_t(uint32_t, uint8_t *)> InterruptFunc;
  /**
   * @brief 中断处理函数指针
   * @param  _argc           参数个数
   * @param  _argv           参数列表
   * @return uint32_t 返回值，0 成功
   */
  typedef uint32_t (*InterruptFunc)(uint32_t _argc, uint8_t *_argv);

  /// 中断处理函数数组
  InterruptFunc interrupt_handlers[Cpu::kInterruptFuncMaxCount]
      __attribute__((aligned(4)));
  /// 异常处理函数数组
  InterruptFunc exception_handlers[Cpu::kExceptionFuncMaxCount]
      __attribute__((aligned(4)));
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_H_ */
