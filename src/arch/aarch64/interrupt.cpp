/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "interrupt.h"

#include "kernel.h"
#include "kernel_fdt.hpp"
#include "kernel_log.hpp"
#include "virtual_memory.hpp"

namespace {
/**
 * @brief 默认中断处理函数
 * @param cause 中断号
 * @param context 中断上下文
 * @return 始终返回 0
 */
auto DefaultInterruptHandler(uint64_t cause, cpu_io::TrapContext* context)
    -> uint64_t {
  klog::Info("Default Interrupt handler {:#X}, {:#x}", cause,
             reinterpret_cast<uintptr_t>(context));
  return 0;
}
}  // namespace

Interrupt::Interrupt() {
  auto [dist_base_addr, dist_size, redist_base_addr, redist_size] =
      KernelFdtSingleton::instance().GetGIC().value();
  VirtualMemorySingleton::instance()
      .MapMMIO(dist_base_addr, dist_size)
      .or_else([](Error err) -> Expected<void*> {
        klog::Err("Failed to map GIC distributor MMIO: {}", err.message());
        while (true) {
          cpu_io::Pause();
        }
        return std::unexpected(err);
      });
  VirtualMemorySingleton::instance()
      .MapMMIO(redist_base_addr, redist_size)
      .or_else([](Error err) -> Expected<void*> {
        klog::Err("Failed to map GIC redistributor MMIO: {}", err.message());
        while (true) {
          cpu_io::Pause();
        }
        return std::unexpected(err);
      });

  gic_ = std::move(Gic(dist_base_addr, redist_base_addr));

  // 注册默认中断处理函数
  for (auto& i : interrupt_handlers_) {
    i = InterruptDelegate::create<DefaultInterruptHandler>();
  }

  // 设置 SGI 0 用于 IPI
  auto cpuid = cpu_io::GetCurrentCoreId();
  gic_.Sgi(0, cpuid);

  klog::Info("Interrupt init.");
}

auto Interrupt::Do(uint64_t cause, cpu_io::TrapContext* context) -> void {
  interrupt_handlers_[cause](cause, context);
  cpu_io::ICC_EOIR1_EL1::Write(cause);
}

auto Interrupt::RegisterInterruptFunc(uint64_t cause, InterruptDelegate func)
    -> void {
  if (func) {
    interrupt_handlers_[cause] = func;
  }
}

auto Interrupt::SendIpi(uint64_t target_cpu_mask) -> Expected<void> {
  /// @todo 默认使用 SGI 0 作为 IPI 中断
  static constexpr uint64_t kIPISGI = 0;

  uint64_t sgi_value = 0;

  // 设置 INTID 为 0 (SGI 0)
  sgi_value |= (kIPISGI & 0xF) << 24;

  // 设置 TargetList (Aff0 级别，低 16 位)
  sgi_value |= (target_cpu_mask & 0xFFFF);

  // 写入 ICC_SGI1R_EL1 寄存器发送 SGI
  cpu_io::ICC_SGI1R_EL1::Write(sgi_value);

  return {};
}

auto Interrupt::BroadcastIpi() -> Expected<void> {
  /// @todo 默认使用 SGI 0 作为 IPI 中断
  static constexpr uint64_t kIPISGI = 0;

  // 构造 ICC_SGI1R_EL1 寄存器的值
  uint64_t sgi_value = 0;

  // 设置 INTID 为 0 (SGI 0)
  sgi_value |= (kIPISGI & 0xF) << 24;

  // 设置 IRM (Interrupt Routing Mode) 为 1，表示广播到所有 PE
  sgi_value |= (1ULL << 40);

  // 写入 ICC_SGI1R_EL1 寄存器发送 SGI
  cpu_io::ICC_SGI1R_EL1::Write(sgi_value);

  return {};
}

auto Interrupt::RegisterExternalInterrupt(uint32_t irq, uint32_t cpu_id,
                                          uint32_t priority,
                                          InterruptDelegate handler)
    -> Expected<void> {
  // irq 为 GIC INTID（已含 kSpiBase 偏移）
  if (irq >= kMaxInterrupt) {
    return std::unexpected(Error(ErrorCode::kIrqChipInvalidIrq));
  }

  // 先注册处理函数
  RegisterInterruptFunc(irq, handler);

  // 再在 GIC 上为指定核心配置并启用 SPI
  gic_.Spi(irq, cpu_id);

  klog::Info("RegisterExternalInterrupt: INTID {}, cpu {}, priority {}", irq,
             cpu_id, priority);
  return {};
}
