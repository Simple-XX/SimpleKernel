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

#include "cstdio"
#include "kernel_log.hpp"

Interrupt::Interrupt() { Info("Interrupt init.\n"); }

void Interrupt::Do(uint64_t cause, uint8_t *context) {
  (void)cause;
  (void)context;
}

void Interrupt::RegisterInterruptFunc(uint64_t cause, InterruptFunc func) {
  (void)cause;
  (void)func;
}

static uint64_t kInterval = 0;

// 在 riscv64 情景下，argc 为启动核 id，argv 为 dtb 地址
/// @todo 从 dtb 读取 cpu 速度
uint32_t InterruptInit(uint32_t argc, uint8_t *argv) {
  (void)argc;
  (void)argv;

  Info("Hello InterruptInit\n");

  return 0;
}
