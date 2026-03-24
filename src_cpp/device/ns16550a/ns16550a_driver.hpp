/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/singleton.h>

#include "device_node.hpp"
#include "driver_registry.hpp"
#include "expected.hpp"
#include "kernel_log.hpp"
#include "ns16550a/ns16550a.hpp"

/**
 * @brief NS16550A UART 驱动
 *
 * 提供 NS16550A UART 设备的驱动程序实现，包括设备检测、初始化和访问接口。
 */
class Ns16550aDriver {
 public:
  using Ns16550aType = ns16550a::Ns16550a;

  /// @note 使用 Ns16550aDriverSingleton::instance() 访问单例。

  /**
   * @brief 返回用于注册的 DriverEntry
   * @return DriverEntry 常量引用
   */
  [[nodiscard]] static auto GetEntry() -> const DriverEntry& {
    static const DriverEntry entry{
        .name = "ns16550a",
        .match_table = etl::span<const MatchEntry>(kMatchTable),
        .match = etl::delegate<bool(
            DeviceNode&)>::create<&Ns16550aDriver::MatchStatic>(),
        .probe = etl::delegate<Expected<void>(DeviceNode&)>::create<
            Ns16550aDriver, &Ns16550aDriver::Probe>(
            etl::singleton<Ns16550aDriver>::instance()),
        .remove = etl::delegate<Expected<void>(DeviceNode&)>::create<
            Ns16550aDriver, &Ns16550aDriver::Remove>(
            etl::singleton<Ns16550aDriver>::instance()),
    };
    return entry;
  }

  /**
   * @brief 硬件检测：MMIO 区域大小是否足够 NS16550A？
   *
   * NS16550A 没有可读的 magic 寄存器，仅验证 MMIO 资源。
   *
   * @return true 如果 mmio_base 非零且 mmio_size 足够
   */
  [[nodiscard]] static auto MatchStatic([[maybe_unused]] DeviceNode& node)
      -> bool {
    static constexpr size_t kMinRegisterSpace = 8;
    return node.mmio_base != 0 && node.mmio_size >= kMinRegisterSpace;
  }

  /**
   * @brief 初始化 NS16550A UART。
   *
   * @pre  node.mmio_base != 0
   * @post uart_ 有效；node.type == DeviceType::kChar
   */
  [[nodiscard]] auto Probe(DeviceNode& node) -> Expected<void> {
    auto ctx = mmio_helper::Prepare(node, 0x100);
    if (!ctx) {
      return std::unexpected(ctx.error());
    }

    auto result = Ns16550aType::Create(ctx->base);
    if (!result) {
      return std::unexpected(Error(result.error().code));
    }

    uart_ = std::move(*result);
    node.type = DeviceType::kChar;
    klog::Info("Ns16550aDriver: UART at {:#x} bound", node.mmio_base);
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
   * @brief 获取 NS16550A 设备实例
   * @return 设备实例指针
   */
  [[nodiscard]] auto GetDevice() -> Ns16550aType* { return &uart_; }

  /// @name 构造/析构函数
  /// @{
  Ns16550aDriver() = default;
  Ns16550aDriver(const Ns16550aDriver&) = delete;
  Ns16550aDriver(Ns16550aDriver&&) = default;
  auto operator=(const Ns16550aDriver&) -> Ns16550aDriver& = delete;
  auto operator=(Ns16550aDriver&&) -> Ns16550aDriver& = default;
  ~Ns16550aDriver() = default;
  /// @}

 private:
  static constexpr MatchEntry kMatchTable[] = {
      {BusType::kPlatform, "ns16550a"},
      {BusType::kPlatform, "ns16550"},
  };

  Ns16550aType uart_{};
};

using Ns16550aDriverSingleton = etl::singleton<Ns16550aDriver>;
