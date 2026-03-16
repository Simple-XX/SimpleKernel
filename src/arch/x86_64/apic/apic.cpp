/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "apic.h"

#include <cpu_io.h>

#include <cassert>
#include <cstring>

#include "kernel_log.hpp"

Apic::Apic(size_t cpu_count) : cpu_count_(cpu_count) {
  // 禁用传统的 8259A PIC 以避免与 APIC 冲突
  cpu_io::Pic::Disable();
}

auto Apic::InitCurrentCpuLocalApic() -> Expected<void> {
  return local_apic_.Init()
      .and_then([]() -> Expected<void> {
        klog::Info(
            "Local APIC initialized successfully for CPU with APIC ID {:#x}",
            cpu_io::GetCurrentCoreId());
        return {};
      })
      .or_else([](Error err) -> Expected<void> {
        klog::Err("Failed to initialize Local APIC for current CPU: {}",
                  err.message());
        return std::unexpected(err);
      });
}

auto Apic::SetIrqRedirection(uint8_t irq, uint8_t vector,
                             uint32_t destination_apic_id, bool mask)
    -> Expected<void> {
  // 检查 IRQ 是否在有效范围内
  if (irq >= io_apic_.GetMaxRedirectionEntries()) {
    klog::Err("IRQ {} exceeds IO APIC range (max: {})", irq,
              io_apic_.GetMaxRedirectionEntries() - 1);
    return std::unexpected(Error(ErrorCode::kApicInvalidIrq));
  }

  // 设置重定向
  io_apic_.SetIrqRedirection(irq, vector, destination_apic_id, mask);
  return {};
}

auto Apic::MaskIrq(uint8_t irq) -> Expected<void> {
  // 检查 IRQ 是否在有效范围内
  if (irq >= io_apic_.GetMaxRedirectionEntries()) {
    klog::Err("IRQ {} exceeds IO APIC range (max: {})", irq,
              io_apic_.GetMaxRedirectionEntries() - 1);
    return std::unexpected(Error(ErrorCode::kApicInvalidIrq));
  }

  io_apic_.MaskIrq(irq);
  return {};
}

auto Apic::UnmaskIrq(uint8_t irq) -> Expected<void> {
  // 检查 IRQ 是否在有效范围内
  if (irq >= io_apic_.GetMaxRedirectionEntries()) {
    klog::Err("IRQ {} exceeds IO APIC range (max: {})", irq,
              io_apic_.GetMaxRedirectionEntries() - 1);
    return std::unexpected(Error(ErrorCode::kApicInvalidIrq));
  }

  io_apic_.UnmaskIrq(irq);
  return {};
}

auto Apic::SendIpi(uint32_t target_apic_id, uint8_t vector) const
    -> Expected<void> {
  return local_apic_.SendIpi(target_apic_id, vector);
}

auto Apic::BroadcastIpi(uint8_t vector) const -> Expected<void> {
  return local_apic_.BroadcastIpi(vector);
}

auto Apic::StartupAp(uint32_t apic_id, uint64_t ap_code_addr,
                     size_t ap_code_size, uint64_t target_addr) const
    -> Expected<void> {
  assert(ap_code_addr != 0 && "AP code address must not be null");
  assert(ap_code_size != 0 && "AP code size must not be zero");
  assert((target_addr & 0xFFF) == 0 && "Target address must be 4KB aligned");
  assert(target_addr < 0x100000 &&
         "Target address exceeds real mode limit (1MB)");

  std::memcpy(reinterpret_cast<void*>(target_addr),
              reinterpret_cast<void*>(ap_code_addr), ap_code_size);

  // 验证复制是否成功
  if (std::memcmp(reinterpret_cast<const void*>(target_addr),
                  reinterpret_cast<const void*>(ap_code_addr),
                  ap_code_size) != 0) {
    klog::Err("AP code copy verification failed");
    return std::unexpected(Error(ErrorCode::kApicCodeCopyFailed));
  }

  // 计算启动向量 (物理地址 / 4096)
  auto start_vector = static_cast<uint8_t>(target_addr >> 12);
  // 使用 Local APIC 发送 INIT-SIPI-SIPI 序列
  local_apic_.WakeupAp(apic_id, start_vector);

  return {};
}

auto Apic::StartupAllAps(uint64_t ap_code_addr, size_t ap_code_size,
                         uint64_t target_addr) const -> void {
  assert(ap_code_addr != 0 && "AP code address must not be null");
  assert(ap_code_size != 0 && "AP code size must not be zero");
  assert((target_addr & 0xFFF) == 0 && "Target address must be 4KB aligned");
  assert(target_addr < 0x100000 &&
         "Target address exceeds real mode limit (1MB)");

  // 启动 APIC ID 0 到 cpu_count_-1 的所有处理器
  // 跳过当前的 BSP (Bootstrap Processor)
  for (size_t apic_id = 0; apic_id < cpu_count_; apic_id++) {
    // 跳过当前 BSP
    if (static_cast<uint32_t>(apic_id) == cpu_io::GetCurrentCoreId()) {
      continue;
    }
    StartupAp(static_cast<uint32_t>(apic_id), ap_code_addr, ap_code_size,
              target_addr)
        .or_else([apic_id](Error err) -> Expected<void> {
          klog::Err("Failed to start AP with APIC ID {:#x}: {}", apic_id,
                    err.message());
          return std::unexpected(err);
        });
  }
}

auto Apic::SendEoi() const -> void { local_apic_.SendEoi(); }

auto Apic::SetupPeriodicTimer(uint32_t frequency_hz, uint8_t vector) const
    -> void {
  local_apic_.SetupPeriodicTimer(frequency_hz, vector);
}

auto Apic::PrintInfo() const -> void {
  local_apic_.PrintInfo();
  io_apic_.PrintInfo();
}
