
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

#include <atomic>
#include <cstdint>

class InterruptBase {
 public:
  /// @name 构造/析构函数
  /// @{
  InterruptBase() = default;
  InterruptBase(const InterruptBase &) = delete;
  InterruptBase(InterruptBase &&) = delete;
  auto operator=(const InterruptBase &) -> InterruptBase & = delete;
  auto operator=(InterruptBase &&) -> InterruptBase & = delete;
  virtual ~InterruptBase() = default;
  /// @}

  virtual uint32_t DoInterrupt(uint32_t, uint8_t *) = 0;
  virtual uint32_t DoException(uint32_t, uint8_t *) = 0;

 protected:
  std::atomic_bool is_inited = false;
};

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_INTERRUPT_BASE_H_ */
