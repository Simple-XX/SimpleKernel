/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include "block_device.hpp"
#include "vfs_types.hpp"

namespace vfs {

/**
 * @brief 文件系统类型基类
 * @details 每种文件系统（ramfs/fat32/ext2 等）注册一个 FileSystem 实例。
 *          VFS 通过此接口挂载/卸载文件系统。
 */
class FileSystem {
 public:
  virtual ~FileSystem() = default;

  /**
   * @brief 获取文件系统类型名（如 "ramfs", "fat32"）
   * @return 文件系统类型名
   */
  [[nodiscard]] virtual auto GetName() const -> const char* = 0;

  /**
   * @brief 挂载文件系统
   * @param device 块设备指针（ramfs 等内存文件系统传 nullptr）
   * @return Expected<Inode*> 根目录 inode
   * @post 返回的 inode->type == FileType::kDirectory
   */
  [[nodiscard]] virtual auto Mount(BlockDevice* device) -> Expected<Inode*> = 0;

  /**
   * @brief 卸载文件系统
   * @return Expected<void> 成功或错误
   * @pre 没有打开的文件引用此文件系统
   */
  [[nodiscard]] virtual auto Unmount() -> Expected<void> = 0;

  /**
   * @brief 将缓存数据刷写到磁盘
   * @return Expected<void> 成功或错误
   */
  [[nodiscard]] virtual auto Sync() -> Expected<void> = 0;

  /**
   * @brief 分配新 inode
   * @return Expected<Inode*> 新分配的 inode 或错误
   * @note 由具体文件系统实现 inode 分配策略
   */
  [[nodiscard]] virtual auto AllocateInode() -> Expected<Inode*> = 0;

  /**
   * @brief 释放 inode
   * @param inode 要释放的 inode
   * @return Expected<void> 成功或错误
   * @pre inode != nullptr
   * @pre inode->link_count == 0
   */
  [[nodiscard]] virtual auto FreeInode(Inode* inode) -> Expected<void> = 0;

  /**
   * @brief 获取文件系统的文件操作接口
   * @return FileOps* 文件操作接口指针
   * @note 用于创建 File 对象时设置 ops
   */
  [[nodiscard]] virtual auto GetFileOps() -> FileOps* = 0;
};

}  // namespace vfs
