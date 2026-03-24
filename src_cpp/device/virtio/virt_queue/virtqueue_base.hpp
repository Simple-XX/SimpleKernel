/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>

#include <cstdint>

#include "expected.hpp"
#include "virtio/virt_queue/misc.hpp"

namespace virtio {

/**
 * @brief Virtqueue 基类（C++23 Deducing this 编译期多态）
 *
 * 提供 Split/Packed Virtqueue 共享的通用逻辑接口。
 * 利用 C++23 Deducing this（显式对象参数）在编译期静态分发到
 * 派生类的具体实现，零虚表开销，无需传统 CRTP 的 static_cast。
 *
 * 派生类应提供以下方法（隐式接口）：
 * - IsValid() const -> bool
 * - Size() const -> uint16_t
 * - NumFree() const -> uint16_t
 * - AllocDesc() -> Expected<uint16_t>
 * - FreeDesc(uint16_t) -> Expected<void>
 * - Submit(uint16_t head) -> void
 * - HasUsed() const -> bool
 * - PopUsed() -> Expected<UsedElem>
 * - SubmitChain(const IoVec*, size_t, const IoVec*, size_t)
 *     -> Expected<uint16_t>
 * - FreeChain(uint16_t head) -> Expected<void>
 * - DescPhys() const -> uint64_t
 * - AvailPhys() const -> uint64_t
 * - UsedPhys() const -> uint64_t
 *
 * @see cpu_io::Wmb/Rmb for barrier semantics
 * @see virtio-v1.2#2.7 / #2.8
 */
class VirtqueueBase {
 public:
  /**
   * @brief 提交 Scatter-Gather 链并通知设备
   *
   * 在 SubmitChain 前后自动插入适当的内存屏障。
   * 通过 Deducing this 在编译期分发到派生类的 SubmitChain 实现。
   *
   * @param readable 设备只读缓冲区数组
   * @param readable_count readable 数组元素数量
   * @param writable 设备可写缓冲区数组
   * @param writable_count writable 数组元素数量
   * @return 成功返回描述符链头索引；失败返回错误
   *
   * @see virtio-v1.2#2.7.13 Supplying Buffers to The Device
   */
  [[nodiscard]] auto SubmitChainWithBarrier(
      this auto&& self, const IoVec* readable, size_t readable_count,
      const IoVec* writable, size_t writable_count) -> Expected<uint16_t> {
    // 写屏障：确保调用方填充的数据对设备可见
    cpu_io::Wmb();
    auto result =
        self.SubmitChain(readable, readable_count, writable, writable_count);
    if (result.has_value()) {
      cpu_io::Mb();
    }
    return result;
  }

  /**
   * @brief 处理已完成的缓冲区并释放描述符链
   *
   * 通用逻辑：从 Used Ring 弹出已完成的元素，对每个元素调用回调函数，
   * 然后释放描述符链。通过 Deducing this 在编译期分发到派生类的
   * HasUsed/PopUsed/FreeChain。
   *
   * @tparam Callback 回调函数类型，签名：void(uint16_t head, uint32_t len)
   * @param callback 对每个已完成请求的回调
   * @return 处理的已完成请求数量
   *
   * @see virtio-v1.2#2.7.14 Receiving Used Buffers From The Device
   */
  template <typename Callback>
  auto ProcessUsedWithCallback(this auto&& self, Callback&& callback)
      -> uint32_t {
    // 读屏障：确保读取到设备最新的 Used Ring 写入
    cpu_io::Rmb();

    uint32_t processed = 0;
    while (self.HasUsed()) {
      auto result = self.PopUsed();
      if (!result.has_value()) {
        break;
      }

      auto elem = *result;
      auto head = static_cast<uint16_t>(elem.id);
      callback(head, elem.len);
      (void)self.FreeChain(head);
      ++processed;
    }
    return processed;
  }

 protected:
  /// @name 构造/析构函数（仅允许派生类使用）
  /// @{
  VirtqueueBase() = default;
  ~VirtqueueBase() = default;
  VirtqueueBase(VirtqueueBase&&) noexcept = default;
  auto operator=(VirtqueueBase&&) noexcept -> VirtqueueBase& = default;
  VirtqueueBase(const VirtqueueBase&) = delete;
  auto operator=(const VirtqueueBase&) -> VirtqueueBase& = delete;
  /// @}
};

}  // namespace virtio
