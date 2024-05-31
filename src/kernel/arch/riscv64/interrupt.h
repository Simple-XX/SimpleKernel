
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
class Clint final : public InterruptBase {
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

  /**
   * @brief 执行中断处理
   * @param  cause 中断或异常号
   * @param  context 中断上下文
   * @return uint32_t 返回值，0 成功
   */
  uint32_t Do(uint32_t cause, uint8_t *context) override;

  void RegisterInterruptFunc(uint32_t intr_no, InterruptFunc func) override;

 private:
  /// 中断处理函数数组
  static InterruptFunc interrupt_handlers[Cpu::kInterruptMaxCount]
      __attribute__((aligned(4)));
  /// 异常处理函数数组
  static InterruptFunc exception_handlers[Cpu::kExceptionMaxCount]
      __attribute__((aligned(4)));
  uint32_t Interrupt(uint32_t, uint8_t *);
  uint32_t Exception(uint32_t, uint8_t *);
};

// /**
//  * @brief platform-level interrupt controller
//  * 平台级中断控制器
//  * 用于控制外部中断
//  */
// class PLIC {
//  public:
//   PLIC();

//   /// @name 构造/析构函数
//   /// @{
//   PLIC(const PLIC &) = delete;
//   PLIC(PLIC &&) = delete;
//   auto operator=(const PLIC &) -> PLIC & = delete;
//   auto operator=(PLIC &&) -> PLIC & = delete;
//   ~PLIC() = default;
//   /// @}

//   /**
//    * @brief 向 PLIC 询问中断
//    * 返回发生的外部中断号
//    * @return uint8_t         中断号
//    */
//   uint8_t get(void);

//   /**
//    * @brief 告知 PLIC 已经处理了当前 IRQ
//    * @param  _no             中断号
//    */
//   void done(uint8_t _no);

//   /**
//    * @brief 设置中断状态
//    * @param  _no             中断号
//    * @param  _status         状态
//    * @todo 不确定
//    */
//   void set(uint8_t _no, bool _status);

//   /**
//    * @brief 注册外部中断处理函数
//    * @param  _no             外部中断号
//    * @param  _interrupt_handler 外部中断处理函数
//    */
//   void register_externel_handler(
//       uint8_t _no, externel_interrupt_handler_t _interrupt_handler);

//   /**
//    * @brief 执行外部中断处理
//    * @param  _no              外部中断号
//    */
//   void do_externel_interrupt(uint8_t _no);

//  private:
//   /// 外部中断处理函数指针
//   typedef void (*externel_interrupt_handler_t)(uint8_t _no);
//   /// 最大外部中断数量
//   static constexpr const size_t EXTERNEL_INTERRUPR_MAX = 16;
//   /// 外部中断处理函数数组
//   externel_interrupt_handler_t
//       externel_interrupt_handlers[EXTERNEL_INTERRUPR_MAX];
// };

class Interrupt final : public InterruptBase {
 public:
  Clint clint;

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
   * @return uint32_t 返回值，0 成功
   */
  uint32_t Do(uint32_t cause, uint8_t *context) override;

  void RegisterInterruptFunc(uint32_t intr_no, InterruptFunc func) override;

 private:
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_H_ */
