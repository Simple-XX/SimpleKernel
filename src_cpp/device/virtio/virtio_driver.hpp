/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/io_port.h>
#include <etl/singleton.h>
#include <etl/span.h>

#include <array>
#include <optional>
#include <variant>

#include "device_manager.hpp"
#include "device_node.hpp"
#include "driver_registry.hpp"
#include "expected.hpp"
#include "io_buffer.hpp"
#include "kernel_log.hpp"
#include "kstd_memory"
#include "virtio/device/blk/virtio_blk.hpp"
#include "virtio/device/blk/virtio_blk_vfs_adapter.hpp"

/**
 * @brief 统一 VirtIO 驱动
 *
 * 匹配所有 virtio,mmio 和 virtio,pci 兼容设备。
 * Probe() 运行期读取 device_id 寄存器，自动分发到对应设备实现
 * 并注册到 DeviceManager。调用方无需关心具体 VirtIO 设备类型。
 */
class VirtioDriver {
 public:
  /// VirtIO 设备类型枚举（来自 VirtIO 1.2 规范）
  enum class DeviceId : uint32_t {
    kNet = 1,
    kBlock = 2,
    kConsole = 3,
    kEntropy = 4,
    kGpu = 16,
    kInput = 18,
  };

  static constexpr uint32_t kDefaultQueueCount = 1;
  static constexpr uint32_t kDefaultQueueSize = 128;
  static constexpr size_t kMinDmaBufferSize = 32768;

  /**
   * @brief 返回驱动注册入口
   *
   * 匹配 virtio,mmio 兼容字符串；MatchStatic 检查 VirtIO magic number。
   */
  [[nodiscard]] static auto GetEntry() -> const DriverEntry& {
    static const DriverEntry entry{
        .name = "virtio",
        .match_table = etl::span<const MatchEntry>(kMatchTable),
        .match = etl::delegate<bool(
            DeviceNode&)>::create<&VirtioDriver::MatchStatic>(),
        .probe = etl::delegate<Expected<void>(DeviceNode&)>::create<
            VirtioDriver, &VirtioDriver::Probe>(
            etl::singleton<VirtioDriver>::instance()),
        .remove = etl::delegate<Expected<void>(DeviceNode&)>::create<
            VirtioDriver, &VirtioDriver::Remove>(
            etl::singleton<VirtioDriver>::instance()),
    };
    return entry;
  }

  /**
   * @brief 硬件检测：验证 VirtIO magic number
   *
   * 只检查 magic number（0x74726976），不检查 device_id——
   * device_id 的分发在 Probe() 中完成。
   *
   * @pre  node.mmio_base != 0
   * @return true 如果设备响应 VirtIO magic
   */
  static auto MatchStatic(DeviceNode& node) -> bool;

  /**
   * @brief 初始化 VirtIO 设备
   *
   * 运行期读取 device_id，分发到对应设备实现，创建并注册设备。
   *
   * @pre  node.mmio_base != 0，MatchStatic() 已返回 true
   * @post 设备已注册到 DeviceManager（node.block_device 已填入适配器指针）
   */
  [[nodiscard]] auto Probe(DeviceNode& node) -> Expected<void>;

  /**
   * @brief 卸载所有 VirtIO 块设备
   * @param  node 设备节点
   * @return Expected<void> 成功返回空值
   */
  [[nodiscard]] auto Remove([[maybe_unused]] DeviceNode& node)
      -> Expected<void> {
    for (size_t i = 0; i < blk_device_count_; ++i) {
      blk_devices_[i].reset();
      dma_buffers_[i].reset();
      slot_buffers_[i].reset();
    }
    blk_device_count_ = 0;
    blk_adapter_count_ = 0;
    return {};
  }

  /**
   * @brief 获取第一个 VirtIO 块设备实例
   * @return 设备实例指针，若不存在则返回 nullptr
   */
  [[nodiscard]] auto GetBlkDevice() -> virtio::blk::VirtioBlk<>* {
    return (blk_device_count_ > 0 && blk_devices_[0].has_value())
               ? &blk_devices_[0].value()
               : nullptr;
  }

  /**
   * @brief 获取第一个块设备的 IRQ 号
   * @return IRQ 号，若无设备则返回 0
   */
  [[nodiscard]] auto GetIrq() const -> uint32_t {
    return blk_device_count_ > 0 ? irqs_[0] : 0;
  }

  /**
   * @brief 处理所有块设备中断
   * @tparam CompletionCallback 完成回调类型
   * @param  on_complete        回调函数
   */
  template <typename CompletionCallback>
  auto HandleInterrupt(CompletionCallback&& on_complete) -> void {
    for (size_t i = 0; i < blk_device_count_; ++i) {
      if (blk_devices_[i].has_value()) {
        blk_devices_[i].value().HandleInterrupt(
            static_cast<CompletionCallback&&>(on_complete));
      }
    }
  }

  /// @name 构造/析构函数
  /// @{
  VirtioDriver() = default;
  VirtioDriver(const VirtioDriver&) = delete;
  VirtioDriver(VirtioDriver&&) = delete;
  auto operator=(const VirtioDriver&) -> VirtioDriver& = delete;
  auto operator=(VirtioDriver&&) -> VirtioDriver& = delete;
  ~VirtioDriver() = default;
  /// @}

 private:
  static constexpr MatchEntry kMatchTable[] = {
      {BusType::kPlatform, "virtio,mmio"},
  };

  static constexpr size_t kMaxBlkDevices = 4;

  std::array<std::optional<virtio::blk::VirtioBlk<>>, kMaxBlkDevices>
      blk_devices_;
  std::array<etl::unique_ptr<IoBuffer>, kMaxBlkDevices> dma_buffers_;
  std::array<etl::unique_ptr<IoBuffer>, kMaxBlkDevices> slot_buffers_;
  std::array<uint32_t, kMaxBlkDevices> irqs_{};
  size_t blk_device_count_{0};

  // Static adapter pool — one slot per probed blk device (kernel lifetime).
  std::array<std::optional<virtio::blk::VirtioBlkVfsAdapter>, kMaxBlkDevices>
      blk_adapters_;
  size_t blk_adapter_count_{0};
};

using VirtioDriverSingleton = etl::singleton<VirtioDriver>;
