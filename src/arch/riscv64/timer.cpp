/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>
#include <opensbi_interface.h>

#include "arch.h"
#include "basic_info.hpp"
#include "interrupt.h"
#include "kernel.h"
#include "per_cpu.hpp"
#include "task_manager.hpp"

using InterruptDelegate = InterruptBase::InterruptDelegate;
namespace {
uint64_t interval{0};

auto TimerHandler(uint64_t, cpu_io::TrapContext*) -> uint64_t {
  sbi_set_timer(cpu_io::Time::Read() + interval);

  auto core_id = cpu_io::GetCurrentCoreId();
  if (per_cpu::in_timer_handler[core_id]) {
    return 0;
  }
  per_cpu::in_timer_handler[core_id] = true;

  auto& tm = TaskManagerSingleton::instance();
  tm.TickUpdate();
  (void)tm.CheckPendingSignals();

  per_cpu::in_timer_handler[core_id] = false;
  return 0;
}
}  // namespace

auto TimerInitSMP() -> void {
  // 开启时钟中断
  cpu_io::Sie::Stie::Set();

  // 设置初次时钟中断时间
  sbi_set_timer(cpu_io::Time::Read() + interval);
}

auto TimerInit() -> void {
  // 计算 interval
  interval = BasicInfoSingleton::instance().interval / SIMPLEKERNEL_TICK;

  // 注册时钟中断
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kSupervisorTimerInterrupt,
      InterruptDelegate::create<TimerHandler>());

  // 开启时钟中断
  cpu_io::Sie::Stie::Set();

  // 设置初次时钟中断时间
  sbi_set_timer(cpu_io::Time::Read() + interval);
}
