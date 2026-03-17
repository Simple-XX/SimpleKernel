/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/singleton.h>

#include "device_node.hpp"
#include "driver_registry.hpp"
#include "expected.hpp"
#include "kernel_log.hpp"
#include "pl011/pl011.hpp"

namespace pl011 {
using Pl011Device = Pl011;
}  // namespace pl011

/**
 * @brief PL011 UART 驱动
 *
 * 提供 PL011 UART 设备的驱动程序实现，包括设备检测、初始化和访问接口。
 */
class Pl011Driver {
 public:
  using Pl011Type = pl011::Pl011;

  /// @note 使用 Pl011DriverSingleton::instance() 访问单例。

  /**
   * @brief 返回用于注册的 DriverEntry
   * @return DriverEntry 常量引用
   */
  [[nodiscard]] static auto GetEntry() -> const DriverEntry& {
    static const DriverEntry entry{
        .name = "pl011",
        .match_table = etl::span<const MatchEntry>(kMatchTable),
        .match = etl::delegate<bool(
            DeviceNode&)>::create<&Pl011Driver::MatchStatic>(),
        .probe = etl::delegate<Expected<void>(DeviceNode&)>::create<
            Pl011Driver, &Pl011Driver::Probe>(
            etl::singleton<Pl011Driver>::instance()),
        .remove = etl::delegate<Expected<void>(DeviceNode&)>::create<
            Pl011Driver, &Pl011Driver::Remove>(
            etl::singleton<Pl011Driver>::instance()),
    };
    return entry;
  }

  /**
   * @brief 硬件检测：MMIO 区域大小是否足够 PL011？
   *
   * PL011 没有可读的 magic 寄存器，仅验证 MMIO 资源。
   *
   * @return true 如果 mmio_base 非零且 mmio_size 足够
   */
  [[nodiscard]] static auto MatchStatic(DeviceNode& node) -> bool {
    static constexpr size_t kMinRegisterSpace = 0x48;
    return node.mmio_base != 0 && node.mmio_size >= kMinRegisterSpace;
  }

  /**
   * @brief 初始化 PL011 UART。
   *
   * @pre  node.mmio_base != 0
   * @post uart_ 有效；node.type == DeviceType::kChar
   */
  [[nodiscard]] auto Probe(DeviceNode& node) -> Expected<void> {
    auto ctx = mmio_helper::Prepare(node, 0x1000);
    if (!ctx) {
      return std::unexpected(ctx.error());
    }

    uart_ = Pl011Type(ctx->base);
    node.type = DeviceType::kChar;
    klog::Info("Pl011Driver: UART at {:#x} bound", node.mmio_base);
    return {};
  }

  /**
   * @brief 卸载驱动
   * @param  node 设备节点
   * @return Expected<void> 成功返回空值
   */
  [[nodiscard]] auto Remove([[maybe_unused]] DeviceNode& node)
      -> Expected<void> {
    return {};
  }

  /**
   * @brief 获取 PL011 设备实例
   * @return 设备实例指针
   */
  [[nodiscard]] auto GetDevice() -> Pl011Type* { return &uart_; }

  /// @name 构造/析构函数
  /// @{
  Pl011Driver() = default;
  Pl011Driver(const Pl011Driver&) = delete;
  Pl011Driver(Pl011Driver&&) = default;
  auto operator=(const Pl011Driver&) -> Pl011Driver& = delete;
  auto operator=(Pl011Driver&&) -> Pl011Driver& = default;
  ~Pl011Driver() = default;
  /// @}

 private:
  static constexpr MatchEntry kMatchTable[] = {
      {BusType::kPlatform, "arm,pl011"},
  };

  Pl011Type uart_{};
};

using Pl011DriverSingleton = etl::singleton<Pl011Driver>;
