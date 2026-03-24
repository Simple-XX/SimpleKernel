/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include "filesystem.hpp"

namespace vfs {

/**
 * @brief 挂载点
 * @details 将一个文件系统的根 inode 关联到目录树中的某个 dentry 上
 */
struct MountPoint {
  /// 挂载路径（如 "/mnt/disk"）
  const char* mount_path{nullptr};
  /// 挂载点在父文件系统中的 dentry
  Dentry* mount_dentry{nullptr};
  /// 挂载的文件系统实例
  FileSystem* filesystem{nullptr};
  /// 关联的块设备（可为 nullptr）
  BlockDevice* device{nullptr};
  /// 该文件系统的根 inode
  Inode* root_inode{nullptr};
  /// 该文件系统的根 dentry
  Dentry* root_dentry{nullptr};
  /// 是否处于活动状态
  bool active{false};
};
/**
 * @brief 挂载表管理器
 */
class MountTable {
 public:
  /// 最大挂载点数
  static constexpr size_t kMaxMounts = 16;

  /// @name 构造/析构函数
  /// @{
  MountTable() = default;
  MountTable(const MountTable&) = delete;
  MountTable(MountTable&&) = delete;
  auto operator=(const MountTable&) -> MountTable& = delete;
  auto operator=(MountTable&&) -> MountTable& = delete;
  ~MountTable() = default;
  /// @}

  /**
   * @brief 挂载文件系统到指定路径
   * @param path 挂载点路径
   * @param fs 文件系统实例
   * @param device 块设备（可为 nullptr）
   * @return Expected<void>
   * @pre path 必须是已存在的目录
   * @post 后续对 path 下路径的访问将被重定向到新文件系统
   */
  [[nodiscard]] auto Mount(const char* path, FileSystem* fs,
                           BlockDevice* device) -> Expected<void>;

  /**
   * @brief 卸载指定路径的文件系统
   * @param path 挂载点路径
   * @return Expected<void>
   */
  [[nodiscard]] auto Unmount(const char* path) -> Expected<void>;

  /**
   * @brief 根据路径查找对应的挂载点
   * @param path 文件路径
   * @return MountPoint* 最长前缀匹配的挂载点，未找到返回 nullptr
   */
  [[nodiscard]] auto Lookup(const char* path) -> MountPoint*;

  /**
   * @brief 获取指定挂载点的根 dentry
   * @param mp 挂载点
   * @return Dentry* 挂载点根 dentry
   */
  [[nodiscard]] auto GetRootDentry(MountPoint* mp) -> Dentry*;

  /**
   * @brief 根据 dentry 查找挂载在其上的文件系统
   * @param dentry 目标 dentry
   * @return MountPoint* 挂载在该 dentry 上的挂载点，未找到返回 nullptr
   */
  [[nodiscard]] auto FindByMountDentry(const Dentry* dentry) -> MountPoint*;

  /**
   * @brief 检查路径是否是挂载点
   * @param path 路径
   * @return true 是挂载点
   * @return false 不是挂载点
   */
  [[nodiscard]] auto IsMountPoint(const char* path) -> bool;

  /**
   * @brief 获取根挂载点
   * @return MountPoint* 根挂载点
   */
  [[nodiscard]] auto GetRootMount() -> MountPoint*;

 private:
  MountPoint mounts_[kMaxMounts]{};
  size_t mount_count_{0};
  MountPoint* root_mount_{nullptr};
};

/**
 * @brief 获取全局挂载表实例
 * @return MountTable& 挂载表引用
 */
[[nodiscard]] auto GetMountTable() -> MountTable&;

}  // namespace vfs
