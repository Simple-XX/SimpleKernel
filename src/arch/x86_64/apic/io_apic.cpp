/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "io_apic.h"

#include <etl/io_port.h>

#include "kernel_log.hpp"

IoApic::IoApic() {
  // 禁用所有重定向条目（设置为屏蔽状态）
  for (uint32_t i = 0; i < GetMaxRedirectionEntries(); i++) {
    auto entry = ReadRedirectionEntry(i);
    entry |= kMaskBit;
    WriteRedirectionEntry(i, entry);
  }

  klog::Info("IO APIC initialization completed");
}

auto IoApic::SetIrqRedirection(uint8_t irq, uint8_t vector,
                               uint32_t destination_apic_id, bool mask) const
    -> void {
  // 检查 IRQ 是否在有效范围内
  auto max_entries = GetMaxRedirectionEntries();
  if (irq >= max_entries) {
    klog::Err("IRQ {} exceeds maximum entries {}", irq, max_entries);
    return;
  }

  // 构造重定向表项
  uint64_t entry = 0;

  // 设置中断向量 (位 0-7)
  entry |= vector & kVectorMask;
  // 设置屏蔽位 (位 16)
  if (mask) {
    entry |= kMaskBit;
  }

  // 设置目标 APIC ID (位 56-63)
  entry |= (static_cast<uint64_t>(destination_apic_id & kDestApicIdMask)
            << kDestApicIdShift);

  WriteRedirectionEntry(irq, entry);
}

auto IoApic::MaskIrq(uint8_t irq) const -> void {
  auto max_entries = GetMaxRedirectionEntries();
  if (irq >= max_entries) {
    klog::Err("IRQ {} exceeds maximum entries {}", irq, max_entries);
    return;
  }

  auto entry = ReadRedirectionEntry(irq);
  entry |= kMaskBit;
  WriteRedirectionEntry(irq, entry);
}

auto IoApic::UnmaskIrq(uint8_t irq) const -> void {
  auto max_entries = GetMaxRedirectionEntries();
  if (irq >= max_entries) {
    klog::Err("IRQ {} exceeds maximum entries {}", irq, max_entries);
    return;
  }

  auto entry = ReadRedirectionEntry(irq);
  entry &= ~kMaskBit;
  WriteRedirectionEntry(irq, entry);
}

auto IoApic::GetId() const -> uint32_t {
  // ID 位于位 24-27
  return (Read(kRegId) >> 24) & 0x0F;
}

auto IoApic::GetVersion() const -> uint32_t {
  // 版本位于位 0-7
  return Read(kRegVer) & 0xFF;
}

auto IoApic::GetMaxRedirectionEntries() const -> uint32_t {
  // MRE 位于位 16-23，实际数量需要 +1
  return ((Read(kRegVer) >> 16) & 0xFF) + 1;
}

auto IoApic::PrintInfo() const -> void {
  klog::Info("IO APIC Information");
  klog::Info("Base Address: {:#x}", base_address_);
  klog::Info("ID: {:#x}", GetId());
  klog::Info("Version: {:#x}", GetVersion());
  klog::Info("Max Redirection Entries: {}", GetMaxRedirectionEntries());
}

auto IoApic::Read(uint32_t reg) const -> uint32_t {
  etl::io_port_wo<uint32_t> sel{
      reinterpret_cast<void*>(base_address_ + kRegSel)};
  sel.write(reg);
  etl::io_port_ro<uint32_t> win{
      reinterpret_cast<void*>(base_address_ + kRegWin)};
  return win.read();
}

auto IoApic::Write(uint32_t reg, uint32_t value) const -> void {
  etl::io_port_wo<uint32_t> sel{
      reinterpret_cast<void*>(base_address_ + kRegSel)};
  sel.write(reg);
  etl::io_port_wo<uint32_t> win{
      reinterpret_cast<void*>(base_address_ + kRegWin)};
  win.write(value);
}

auto IoApic::ReadRedirectionEntry(uint8_t irq) const -> uint64_t {
  auto low_reg = kRedTblBase + (irq * 2);
  auto high_reg = low_reg + 1;

  auto low = Read(low_reg);
  auto high = Read(high_reg);

  return (static_cast<uint64_t>(high) << 32) | low;
}

auto IoApic::WriteRedirectionEntry(uint8_t irq, uint64_t value) const -> void {
  auto low_reg = kRedTblBase + (irq * 2);
  auto high_reg = low_reg + 1;

  auto low = static_cast<uint32_t>(value & 0xFFFFFFFF);
  auto high = static_cast<uint32_t>((value >> 32) & 0xFFFFFFFF);

  Write(low_reg, low);
  Write(high_reg, high);
}
