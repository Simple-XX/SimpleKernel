/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "gic.h"

#include <cpu_io.h>

#include <cassert>

#include "kernel_log.hpp"

Gic::Gic(uint64_t gicd_base_addr, uint64_t gicr_base_addr)
    : gicd_(gicd_base_addr), gicr_(gicr_base_addr) {
  cpu_io::ICC_IGRPEN1_EL1::Enable::Clear();
  cpu_io::ICC_PMR_EL1::Priority::Set();

  gicd_.EnableGrp1Ns();

  klog::Info("Gic init.");
}

auto Gic::SetUp() const -> void {
  cpu_io::ICC_IGRPEN1_EL1::Enable::Clear();
  cpu_io::ICC_PMR_EL1::Priority::Set();
  gicd_.EnableGrp1Ns();

  gicr_.SetUp();
}

auto Gic::Spi(uint32_t intid, uint32_t cpuid) const -> void {
  gicd_.SetupSpi(intid, cpuid);
}

auto Gic::Ppi(uint32_t intid, uint32_t cpuid) const -> void {
  gicr_.SetupPpi(intid, cpuid);
}

auto Gic::Sgi(uint32_t intid, uint32_t cpuid) const -> void {
  gicr_.SetupSgi(intid, cpuid);
}

Gic::Gicd::Gicd(uint64_t _base_addr) : base_addr_(_base_addr) {
  assert(base_addr_ != 0 && "GICD base address is invalid");

  // 将 GICD_CTLR 清零
  Write(kCtlr, 0);

  // 读取 ITLinesNumber 数量
  auto it_lines_number = Read(kTyper) & kTyperItLinesNumberMask;

  klog::Info("it_lines_number {}", it_lines_number);

  // 设置中断为 Non-secure Group 1
  for (uint32_t i = 0; i < it_lines_number; i++) {
    Write(Igrouprn(i), UINT32_MAX);
  }
}

auto Gic::Gicd::Enable(uint32_t intid) const -> void {
  auto is = Read(Isenablern(intid / kIsEnablernSize));
  is |= 1 << (intid % kIsEnablernSize);
  Write(Isenablern(intid / kIsEnablernSize), is);
}

auto Gic::Gicd::EnableGrp1Ns() const -> void {
  Write(kCtlr, kCtlrEnableGrp1Ns);
  cpu_io::ICC_IGRPEN1_EL1::Enable::Set();
}

auto Gic::Gicd::Disable(uint32_t intid) const -> void {
  auto ic = Read(Icenablern(intid / kIcEnablernSize));
  ic |= 1 << (intid % kIcEnablernSize);
  Write(Icenablern(intid / kIcEnablernSize), ic);
}

auto Gic::Gicd::Clear(uint32_t intid) const -> void {
  auto ic = Read(Icpendrn(intid / kIcPendrnSize));
  ic |= 1 << (intid % kIcPendrnSize);
  Write(Icpendrn(intid / kIcPendrnSize), ic);
}

auto Gic::Gicd::IsEnable(uint32_t intid) const -> bool {
  auto is = Read(Isenablern(intid / kIsEnablernSize));
  return is & (1 << (intid % kIsEnablernSize));
}

auto Gic::Gicd::SetPrio(uint32_t intid, uint32_t prio) const -> void {
  auto shift = (intid % kIpriorityrnSize) * kIpriorityrnBits;
  auto ip = Read(Ipriorityrn(intid / kIpriorityrnSize));
  ip &= ~(kIpriorityrnBitsMask << shift);
  ip |= prio << shift;
  Write(Ipriorityrn(intid / kIpriorityrnSize), ip);
}

auto Gic::Gicd::SetConfig(uint32_t intid, uint32_t config) const -> void {
  auto shift = (intid % kIcfgrnSize) * kIcfgrnBits;
  auto ic = Read(Icfgrn(intid / kIcfgrnSize));
  ic &= ~(kIcfgrnBitsMask << shift);
  ic |= config << shift;
  Write(Icfgrn(intid / kIcfgrnSize), ic);
}

auto Gic::Gicd::SetTarget(uint32_t intid, uint32_t cpuid) const -> void {
  auto target = Read(Itargetsrn(intid / kItargetsrnSize));
  target &= ~(kIcfgrnBitsMask << ((intid % kItargetsrnSize) * kItargetsrnBits));
  Write(
      Itargetsrn(intid / kItargetsrnSize),
      target | ((1 << cpuid) << ((intid % kItargetsrnSize) * kItargetsrnBits)));
}

auto Gic::Gicr::SetUp() const -> void {
  auto cpuid = cpu_io::GetCurrentCoreId();

  // 将 GICR_CTLR 清零
  Write(cpuid, kCtlr, 0);

  // The System register interface for the current Security state is enabled.
  cpu_io::ICC_SRE_EL1::SRE::Set();

  // 允许 Non-secure Group 1 中断
  Write(cpuid, kIgroupr0, kIgroupr0Set);
  Write(cpuid, kIgrpmodr0, kIgrpmodr0Clear);

  // 唤醒 Redistributor
  // @see
  // https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-WAKER--Redistributor-Wake-Register?lang=en
  auto waker = Read(cpuid, kWaker);
  // Clear the ProcessorSleep bit
  Write(cpuid, kWaker, waker & ~kWakerProcessorSleepMask);
  // 等待唤醒完成
  while (Read(cpuid, kWaker) & kWakerChildrenAsleepMask) {
    cpu_io::Pause();
  }
}

auto Gic::Gicd::SetupSpi(uint32_t intid, uint32_t cpuid) const -> void {
  // 电平触发
  SetConfig(intid, kIcfgrnLevelSensitive);

  // 优先级设定
  SetPrio(intid, 0);

  // 设置所有中断由 cpu0 处理
  SetTarget(intid, cpuid);
  // 清除中断请求
  Clear(intid);
  // 使能中断
  Enable(intid);
}

Gic::Gicr::Gicr(uint64_t _base_addr) : base_addr_(_base_addr) {
  assert(base_addr_ != 0 && "GICR base address is invalid");

  auto cpuid = cpu_io::GetCurrentCoreId();

  // 将 GICR_CTLR 清零
  Write(cpuid, kCtlr, 0);

  // The System register interface for the current Security state is enabled.
  cpu_io::ICC_SRE_EL1::SRE::Set();

  // 允许 Non-secure Group 1 中断
  Write(cpuid, kIgroupr0, kIgroupr0Set);
  Write(cpuid, kIgrpmodr0, kIgrpmodr0Clear);

  // 唤醒 Redistributor
  // @see
  // https://developer.arm.com/documentation/ddi0601/2024-12/External-Registers/GICR-WAKER--Redistributor-Wake-Register?lang=en
  auto waker = Read(cpuid, kWaker);
  // Clear the ProcessorSleep bit
  Write(cpuid, kWaker, waker & ~kWakerProcessorSleepMask);
  // 等待唤醒完成
  while (Read(cpuid, kWaker) & kWakerChildrenAsleepMask) {
    cpu_io::Pause();
  }
}

auto Gic::Gicr::Enable(uint32_t intid, uint32_t cpuid) const -> void {
  auto is = Read(cpuid, kIsEnabler0);
  is |= 1 << (intid % kIsEnabler0Size);
  Write(cpuid, kIsEnabler0, is);
}

auto Gic::Gicr::Disable(uint32_t intid, uint32_t cpuid) const -> void {
  auto ic = Read(cpuid, kIcEnabler0);
  ic |= 1 << (intid % kIcEnabler0Size);
  Write(cpuid, kIcEnabler0, ic);
}

auto Gic::Gicr::Clear(uint32_t intid, uint32_t cpuid) const -> void {
  auto ic = Read(cpuid, kIcPendr0);
  ic |= 1 << (intid % kIcPendr0Size);
  Write(cpuid, kIcPendr0, ic);
}

auto Gic::Gicr::SetPrio(uint32_t intid, uint32_t cpuid, uint32_t prio) const
    -> void {
  auto shift = (intid % kIpriorityrnSize) * kIpriorityrnBits;
  auto ip = Read(cpuid, Ipriorityrn(intid / kIpriorityrnSize));
  ip &= ~(kIpriorityrnBitsMask << shift);
  ip |= prio << shift;
  Write(cpuid, Ipriorityrn(intid / kIpriorityrnSize), ip);
}

auto Gic::Gicr::SetupPpi(uint32_t intid, uint32_t cpuid) const -> void {
  SetPrio(intid, cpuid, 0);
  Clear(intid, cpuid);
  Enable(intid, cpuid);
}

auto Gic::Gicr::SetupSgi(uint32_t intid, uint32_t cpuid) const -> void {
  SetPrio(intid, cpuid, 0);
  Clear(intid, cpuid);
  Enable(intid, cpuid);
}
