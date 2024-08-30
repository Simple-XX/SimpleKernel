/**
 * @file interrupt.cpp
 * @brief 中断初始化
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

#include "interrupt.h"

#include "kernel_log.hpp"
#include "sk_cstdio"

Interrupt::Interrupt() { klog::Info("Interrupt init.\n"); }

void Interrupt::Do(uint64_t cause, uint8_t *context) {
  (void)cause;
  (void)context;
}

void Interrupt::RegisterInterruptFunc(uint64_t cause, InterruptFunc func) {
  (void)cause;
  (void)func;
}

uint32_t InterruptInit(uint32_t, uint8_t *) {
  klog::Info("Hello InterruptInit\n");

  return 0;
}
