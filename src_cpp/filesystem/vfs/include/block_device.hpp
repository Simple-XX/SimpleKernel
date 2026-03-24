/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "expected.hpp"

namespace vfs {

/**
 * @brief 块设备抽象基类
 * @details 所有块设备驱动（virtio-blk、ramdisk 等）必须实现此接口。
 *          块设备以固定大小的扇区 (sector) 为最小 I/O 单位。
 */
class BlockDevice {
 public:
  virtual ~BlockDevice() = default;

  /**
   * @brief 读取连续扇区
   * @param sector_start 起始扇区号（LBA）
   * @param sector_count 扇区数量
   * @param buffer 输出缓冲区，大小至少为 sector_count * GetSectorSize()
   * @return Expected<size_t> 成功时返回实际读取的字节数
   * @pre buffer != nullptr
   * @pre sector_start + sector_count <= GetSectorCount()
   * @post 返回值 == sector_count * GetSectorSize() 或错误
   */
  [[nodiscard]] virtual auto ReadSectors(uint64_t sector_start,
                                         uint32_t sector_count, void* buffer)
      -> Expected<size_t> = 0;

  /**
   * @brief 写入连续扇区
   * @param sector_start 起始扇区号（LBA）
   * @param sector_count 扇区数量
   * @param buffer 输入缓冲区
   * @return Expected<size_t> 成功时返回实际写入的字节数
   * @pre buffer != nullptr
   * @pre sector_start + sector_count <= GetSectorCount()
   */
  [[nodiscard]] virtual auto WriteSectors(uint64_t sector_start,
                                          uint32_t sector_count,
                                          const void* buffer)
      -> Expected<size_t> = 0;

  /**
   * @brief 获取扇区大小（通常为 512 字节）
   * @return 扇区大小（字节）
   */
  [[nodiscard]] virtual auto GetSectorSize() const -> uint32_t = 0;

  /**
   * @brief 获取设备总扇区数
   * @return 总扇区数
   */
  [[nodiscard]] virtual auto GetSectorCount() const -> uint64_t = 0;

  /**
   * @brief 获取设备名称（如 "virtio-blk0"）
   * @return 设备名称
   */
  [[nodiscard]] virtual auto GetName() const -> const char* = 0;

  /**
   * @brief 刷新设备缓存到物理介质
   * @return Expected<void> 成功或错误
   */
  [[nodiscard]] virtual auto Flush() -> Expected<void> { return {}; }
};

}  // namespace vfs
