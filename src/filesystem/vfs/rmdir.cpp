/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <etl/memory.h>

#include "filesystem.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "spinlock.hpp"
#include "vfs_internal.hpp"

namespace vfs {

auto RmDir(const char* path) -> Expected<void> {
  if (path == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  // 解析父目录路径和目录名
  char parent_path[512];
  char dir_name[256];
  const char* last_slash = strrchr(path, '/');
  if (last_slash == nullptr || last_slash == path) {
    strncpy(parent_path, "/", sizeof(parent_path));
    strncpy(dir_name, path[0] == '/' ? path + 1 : path, sizeof(dir_name));
  } else {
    size_t parent_len = last_slash - path;
    if (parent_len >= sizeof(parent_path)) {
      parent_len = sizeof(parent_path) - 1;
    }
    strncpy(parent_path, path, parent_len);
    parent_path[parent_len] = '\0';
    strncpy(dir_name, last_slash + 1, sizeof(dir_name));
  }
  dir_name[sizeof(dir_name) - 1] = '\0';

  // 查找父目录
  auto parent_result = Lookup(parent_path);
  if (!parent_result.has_value()) {
    return std::unexpected(parent_result.error());
  }

  Dentry* parent_dentry = parent_result.value();
  if (parent_dentry->inode == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsCorrupted));
  }

  // 查找目标目录
  Dentry* target_dentry = FindChild(parent_dentry, dir_name);
  if (target_dentry == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsFileNotFound));
  }

  if (target_dentry->inode == nullptr ||
      target_dentry->inode->type != FileType::kDirectory) {
    return std::unexpected(Error(ErrorCode::kFsNotADirectory));
  }

  // 检查目录是否为空
  if (target_dentry->children != nullptr) {
    return std::unexpected(Error(ErrorCode::kFsNotEmpty));
  }

  // 删除目录
  if (parent_dentry->inode->ops == nullptr) {
    return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
  }

  auto result =
      parent_dentry->inode->ops->Rmdir(parent_dentry->inode, dir_name);
  if (!result.has_value()) {
    return std::unexpected(result.error());
  }

  // 从父目录中移除 dentry
  RemoveChild(parent_dentry, target_dentry);

  if (target_dentry->ref_count > 0) {
    target_dentry->deleted = true;
    target_dentry->inode = nullptr;
  } else {
    etl::unique_ptr<Dentry> dentry_guard(target_dentry);
  }

  klog::Debug("VFS: removed directory '{}'", path);
  return {};
}

}  // namespace vfs
