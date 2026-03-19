/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include "arch.h"
#include "basic_info.hpp"
#include "interrupt.h"
#include "kernel.h"
#include "kernel_fdt.hpp"
#include "kernel_log.hpp"
#include "kstd_cstdio"
#include "pl011/pl011_driver.hpp"
#include "pl011_singleton.h"

using InterruptDelegate = InterruptBase::InterruptDelegate;
namespace {
/**
 * @brief 通用异常处理辅助函数
 * @param exception_msg 异常类型描述
 * @param context 中断上下文
 * @param print_regs 要打印的寄存器数量 (0, 4, 或 8)
 */
auto HandleException(const char* exception_msg, cpu_io::TrapContext* context,
                     int print_regs = 0) -> void {
  klog::Err("{}", exception_msg);
  klog::Err(
      "  ESR_EL1: {:#X}, ELR_EL1: {:#X}, SP_EL0: {:#X}, SP_EL1: {:#X}, "
      "SPSR_EL1: {:#X}",
      context->esr_el1, context->elr_el1, context->sp_el0, context->sp_el1,
      context->spsr_el1);

  if (print_regs == 4) {
    klog::Err("  x0-x3: {:#X} {:#X} {:#X} {:#X}", context->x0, context->x1,
              context->x2, context->x3);
  } else if (print_regs == 8) {
    klog::Err("  x0-x7: {:#X} {:#X} {:#X} {:#X} {:#X} {:#X} {:#X} {:#X}",
              context->x0, context->x1, context->x2, context->x3, context->x4,
              context->x5, context->x6, context->x7);
  }

  while (true) {
    cpu_io::Pause();
  }
}
}  // namespace

/// 异常向量表入口
extern "C" auto vector_table() -> void;

// 同步异常处理程序
/// 同步异常处理 - Current EL with SP0
extern "C" auto sync_current_el_sp0_handler(cpu_io::TrapContext* context)
    -> void {
  HandleException("Sync Exception at Current EL with SP0", context, 4);
}

/// IRQ 异常处理 - Current EL with SP0
extern "C" auto irq_current_el_sp0_handler(
    [[maybe_unused]] cpu_io::TrapContext* context) -> void {
  klog::Err("IRQ Exception at Current EL with SP0");
  // 处理 IRQ 中断
  // ...
}

/// FIQ 异常处理 - Current EL with SP0
extern "C" auto fiq_current_el_sp0_handler(
    [[maybe_unused]] cpu_io::TrapContext* context) -> void {
  klog::Err("FIQ Exception at Current EL with SP0");
  // 处理 FIQ 中断
  // ...
}

/// 错误异常处理 - Current EL with SP0
extern "C" auto error_current_el_sp0_handler(cpu_io::TrapContext* context)
    -> void {
  HandleException("Error Exception at Current EL with SP0", context);
}

/// 同步异常处理 - Current EL with SPx
extern "C" auto sync_current_el_spx_handler(cpu_io::TrapContext* context)
    -> void {
  HandleException("Sync Exception at Current EL with SPx", context, 4);
}

/// IRQ 异常处理 - Current EL with SPx
extern "C" auto irq_current_el_spx_handler(cpu_io::TrapContext* context)
    -> void {
  auto cause = cpu_io::ICC_IAR1_EL1::INTID::Get();
  // GICv3: INTID 1020-1023 为特殊值（spurious），不应处理也不应写 EOI
  if (cause >= 1020) {
    klog::Warn("Spurious IRQ: INTID {}", cause);
    return;
  }
  InterruptSingleton::instance().Do(cause, context);
}

/// FIQ 异常处理 - Current EL with SPx
extern "C" auto fiq_current_el_spx_handler(
    [[maybe_unused]] cpu_io::TrapContext* context) -> void {
  klog::Err("FIQ Exception at Current EL with SPx");
  // 处理 FIQ 中断
  // ...
}

/// 错误异常处理 - Current EL with SPx
extern "C" auto error_current_el_spx_handler(cpu_io::TrapContext* context)
    -> void {
  HandleException("Error Exception at Current EL with SPx", context);
}

/// 同步异常处理 - Lower EL using AArch64
extern "C" auto sync_lower_el_aarch64_handler(cpu_io::TrapContext* context)
    -> void {
  HandleException("Sync Exception at Lower EL using AArch64", context, 8);
}

/// IRQ 异常处理 - Lower EL using AArch64
extern "C" auto irq_lower_el_aarch64_handler(
    [[maybe_unused]] cpu_io::TrapContext* context) -> void {
  klog::Err("IRQ Exception at Lower EL using AArch64");
  // 处理 IRQ 中断
  // ...
}

/// FIQ 异常处理 - Lower EL using AArch64
extern "C" auto fiq_lower_el_aarch64_handler(
    [[maybe_unused]] cpu_io::TrapContext* context) -> void {
  klog::Err("FIQ Exception at Lower EL using AArch64");
  // 处理 FIQ 中断
  // ...
}

/// 错误异常处理 - Lower EL using AArch64
extern "C" auto error_lower_el_aarch64_handler(cpu_io::TrapContext* context)
    -> void {
  HandleException("Error Exception at Lower EL using AArch64", context);
}

/// 同步异常处理 - Lower EL using AArch32
extern "C" auto sync_lower_el_aarch32_handler(cpu_io::TrapContext* context)
    -> void {
  HandleException("Sync Exception at Lower EL using AArch32", context);
}

/// IRQ 异常处理 - Lower EL using AArch32
extern "C" auto irq_lower_el_aarch32_handler(
    [[maybe_unused]] cpu_io::TrapContext* context) -> void {
  klog::Err("IRQ Exception at Lower EL using AArch32");
  // 处理 IRQ 中断
  // ...
}

/// FIQ 异常处理 - Lower EL using AArch32
extern "C" auto fiq_lower_el_aarch32_handler(
    [[maybe_unused]] cpu_io::TrapContext* context) -> void {
  klog::Err("FIQ Exception at Lower EL using AArch32");
  // 处理 FIQ 中断
  // ...
}

/// 错误异常处理 - Lower EL using AArch32
extern "C" auto error_lower_el_aarch32_handler(cpu_io::TrapContext* context)
    -> void {
  HandleException("Error Exception at Lower EL using AArch32", context);
}

/**
 * @brief UART 中断处理函数
 * @param cause 中断号
 * @return 中断号
 */
auto uart_handler(uint64_t cause, cpu_io::TrapContext*) -> uint64_t {
  Pl011Singleton::instance().HandleInterrupt(
      [](uint8_t ch) { etl_putchar(ch); });
  return cause;
}

auto InterruptInit(int, const char**) -> void {
  InterruptSingleton::create();

  cpu_io::VBAR_EL1::Write(reinterpret_cast<uint64_t>(vector_table));

  auto uart_intid =
      KernelFdtSingleton::instance().GetAarch64Intid("arm,pl011").value() +
      Gic::kSpiBase;

  klog::Info("uart_intid: {}", uart_intid);

  // 通过统一接口注册 UART 外部中断（先注册 handler，再启用 GIC SPI）
  InterruptSingleton::instance()
      .RegisterExternalInterrupt(uart_intid, cpu_io::GetCurrentCoreId(), 0,
                                 InterruptDelegate::create<uart_handler>())
      .or_else([](Error err) -> Expected<void> {
        klog::Err("Failed to register UART IRQ: {}", err.message());
        return std::unexpected(err);
      });

  cpu_io::EnableInterrupt();

  klog::Info("Hello InterruptInit");
}

auto InterruptInitSMP(int, const char**) -> void {
  cpu_io::VBAR_EL1::Write(reinterpret_cast<uint64_t>(vector_table));

  InterruptSingleton::instance().SetUp();

  cpu_io::EnableInterrupt();

  klog::Info("Hello InterruptInitSMP");
}
