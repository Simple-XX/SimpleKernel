/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <array>

#include "block_device.hpp"
#include "expected.hpp"
#include "virtio/device/blk/virtio_blk.hpp"

namespace virtio::blk {

/**
 * @brief Adapts VirtioBlk<> to vfs::BlockDevice.
 *
 * Wraps a VirtioBlk instance and forwards ReadSectors/WriteSectors
 * to the underlying VirtIO block device.
 */
class VirtioBlkVfsAdapter final : public vfs::BlockDevice {
 public:
  using VirtioBlkType = VirtioBlk<>;

  /// @name 构造/析构函数
  /// @{
  /**
   * @brief 构造函数
   * @param  _dev   VirtIO 块设备实例
   * @param  _index 设备索引
   */
  explicit VirtioBlkVfsAdapter(VirtioBlkType* _dev, uint32_t _index = 0)
      : dev_(_dev), index_(_index) {}
  VirtioBlkVfsAdapter() = delete;
  VirtioBlkVfsAdapter(const VirtioBlkVfsAdapter&) = delete;
  VirtioBlkVfsAdapter(VirtioBlkVfsAdapter&&) = delete;
  auto operator=(const VirtioBlkVfsAdapter&) -> VirtioBlkVfsAdapter& = delete;
  auto operator=(VirtioBlkVfsAdapter&&) -> VirtioBlkVfsAdapter& = delete;
  ~VirtioBlkVfsAdapter() override = default;
  /// @}

  auto ReadSectors(uint64_t lba, uint32_t count, void* buf)
      -> Expected<size_t> override {
    auto* ptr = static_cast<uint8_t*>(buf);
    for (uint32_t i = 0; i < count; ++i) {
      auto result = dev_->Read(lba + i, ptr + i * kSectorSize);
      if (!result) {
        return std::unexpected(Error(result.error().code));
      }
    }
    return static_cast<size_t>(count) * kSectorSize;
  }

  auto WriteSectors(uint64_t lba, uint32_t count, const void* buf)
      -> Expected<size_t> override {
    const auto* ptr = static_cast<const uint8_t*>(buf);
    for (uint32_t i = 0; i < count; ++i) {
      auto result = dev_->Write(lba + i, ptr + i * kSectorSize);
      if (!result) {
        return std::unexpected(Error(result.error().code));
      }
    }
    return static_cast<size_t>(count) * kSectorSize;
  }

  [[nodiscard]] auto GetSectorSize() const -> uint32_t override {
    return kSectorSize;
  }

  [[nodiscard]] auto GetSectorCount() const -> uint64_t override {
    return dev_->GetCapacity();
  }

  [[nodiscard]] auto GetName() const -> const char* override {
    static constexpr std::array<const char*, 4> kNames = {
        "virtio-blk0", "virtio-blk1", "virtio-blk2", "virtio-blk3"};
    return (index_ < kNames.size()) ? kNames[index_] : "virtio-blk?";
  }

 private:
  static constexpr uint32_t kSectorSize = 512;
  VirtioBlkType* dev_{nullptr};
  uint32_t index_{0};
};

}  // namespace virtio::blk
