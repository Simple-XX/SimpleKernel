/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include "arch.h"
#include "basic_info.hpp"
#include "interrupt.h"
#include "kernel.h"
#include "kernel_fdt.hpp"
#include "per_cpu.hpp"
#include "task_manager.hpp"

using InterruptDelegate = InterruptBase::InterruptDelegate;
namespace {
/// 定时器中断间隔
uint64_t interval{0};
/// 定时器中断号
uint64_t timer_intid{0};

auto TimerHandler(uint64_t, cpu_io::TrapContext*) -> uint64_t {
  cpu_io::CNTV_TVAL_EL0::Write(interval);

  auto& preempt = per_cpu::GetCurrentCore().preempt;
  if (preempt.InHardIrq()) {
    return 0;
  }
  preempt.hardirq_count++;

  auto& tm = TaskManagerSingleton::instance();
  tm.TickUpdate();
  (void)tm.CheckPendingSignals();

  preempt.hardirq_count--;
  return 0;
}
}  // namespace

auto TimerInitSMP() -> void {
  InterruptSingleton::instance().Ppi(timer_intid, cpu_io::GetCurrentCoreId());

  cpu_io::CNTV_CTL_EL0::ENABLE::Clear();
  cpu_io::CNTV_CTL_EL0::IMASK::Set();

  cpu_io::CNTV_TVAL_EL0::Write(interval);

  cpu_io::CNTV_CTL_EL0::ENABLE::Set();
  cpu_io::CNTV_CTL_EL0::IMASK::Clear();
}

auto TimerInit() -> void {
  // 计算 interval
  interval = BasicInfoSingleton::instance().interval / SIMPLEKERNEL_TICK;

  // 获取定时器中断号
  timer_intid = KernelFdtSingleton::instance()
                    .GetAarch64Intid("arm,armv8-timer")
                    .value() +
                Gic::kPpiBase;

  InterruptSingleton::instance().RegisterInterruptFunc(
      timer_intid, InterruptDelegate::create<TimerHandler>());

  InterruptSingleton::instance().Ppi(timer_intid, cpu_io::GetCurrentCoreId());

  cpu_io::CNTV_CTL_EL0::ENABLE::Clear();
  cpu_io::CNTV_CTL_EL0::IMASK::Set();

  cpu_io::CNTV_TVAL_EL0::Write(interval);

  cpu_io::CNTV_CTL_EL0::ENABLE::Set();
  cpu_io::CNTV_CTL_EL0::IMASK::Clear();
}
