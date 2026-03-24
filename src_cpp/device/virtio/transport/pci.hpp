/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include "virtio/transport/transport.hpp"

namespace virtio {

/**
 * @brief Virtio PCI 传输层（占位）
 * @todo 实现 PCI Modern (1.0+) 传输层
 * @see virtio-v1.2#4.1
 */
class PciTransport final : public Transport {
 public:
  [[nodiscard]] auto IsValid() const -> bool { return false; }
  [[nodiscard]] auto GetDeviceId() const -> uint32_t { return 0; }
  [[nodiscard]] auto GetVendorId() const -> uint32_t { return 0; }

  [[nodiscard]] auto GetStatus() const -> uint32_t { return 0; }
  auto SetStatus(uint32_t /*status*/) -> void {}

  [[nodiscard]] auto GetDeviceFeatures() -> uint64_t { return 0; }
  auto SetDriverFeatures(uint64_t /*features*/) -> void {}

  [[nodiscard]] auto GetQueueNumMax(uint32_t /*queue_idx*/) -> uint32_t {
    return 0;
  }
  auto SetQueueNum(uint32_t /*queue_idx*/, uint32_t /*num*/) -> void {}
  auto SetQueueDesc(uint32_t /*queue_idx*/, uint64_t /*addr*/) -> void {}
  auto SetQueueAvail(uint32_t /*queue_idx*/, uint64_t /*addr*/) -> void {}
  auto SetQueueUsed(uint32_t /*queue_idx*/, uint64_t /*addr*/) -> void {}
  [[nodiscard]] auto GetQueueReady(uint32_t /*queue_idx*/) -> bool {
    return false;
  }
  auto SetQueueReady(uint32_t /*queue_idx*/, bool /*ready*/) -> void {}

  auto NotifyQueue(uint32_t /*queue_idx*/) -> void {}
  [[nodiscard]] auto GetInterruptStatus() const -> uint32_t { return 0; }
  auto AckInterrupt(uint32_t /*ack_bits*/) -> void {}

  [[nodiscard]] auto ReadConfigU8(uint32_t /*offset*/) const -> uint8_t {
    return 0;
  }
  [[nodiscard]] auto ReadConfigU16(uint32_t /*offset*/) const -> uint16_t {
    return 0;
  }
  [[nodiscard]] auto ReadConfigU32(uint32_t /*offset*/) const -> uint32_t {
    return 0;
  }
  [[nodiscard]] auto ReadConfigU64(uint32_t /*offset*/) const -> uint64_t {
    return 0;
  }
  [[nodiscard]] auto GetConfigGeneration() const -> uint32_t { return 0; }
};

}  // namespace virtio
