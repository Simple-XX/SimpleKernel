
/**
 * @file interrupt_base.h
 * @brief 中断处理接口
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_BASE_H_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_BASE_H_

#include <cstdint>

class InterruptBase {
 public:
  /// @name 构造/析构函数
  /// @{
  InterruptBase() = default;
  InterruptBase(const InterruptBase &) = default;
  InterruptBase(InterruptBase &&) = default;
  auto operator=(const InterruptBase &) -> InterruptBase & = default;
  auto operator=(InterruptBase &&) -> InterruptBase & = default;
  virtual ~InterruptBase() = default;
  /// @}
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_BASE_H_ */
