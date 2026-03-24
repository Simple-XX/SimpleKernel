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

auto Unlink(const char* path) -> Expected<void> {
  if (path == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  // 解析父目录路径和文件名
  char parent_path[512];
  char file_name[256];
  const char* last_slash = strrchr(path, '/');
  if (last_slash == nullptr || last_slash == path) {
    strncpy(parent_path, "/", sizeof(parent_path));
    strncpy(file_name, path[0] == '/' ? path + 1 : path, sizeof(file_name));
  } else {
    size_t parent_len = last_slash - path;
    if (parent_len >= sizeof(parent_path)) {
      parent_len = sizeof(parent_path) - 1;
    }
    strncpy(parent_path, path, parent_len);
    parent_path[parent_len] = '\0';
    strncpy(file_name, last_slash + 1, sizeof(file_name));
  }
  file_name[sizeof(file_name) - 1] = '\0';

  // 查找父目录
  auto parent_result = Lookup(parent_path);
  if (!parent_result.has_value()) {
    return std::unexpected(parent_result.error());
  }

  Dentry* parent_dentry = parent_result.value();
  if (parent_dentry->inode == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsCorrupted));
  }

  // 查找目标文件
  Dentry* target_dentry = FindChild(parent_dentry, file_name);
  if (target_dentry == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsFileNotFound));
  }

  if (target_dentry->inode == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsCorrupted));
  }

  // 不能删除目录
  if (target_dentry->inode->type == FileType::kDirectory) {
    return std::unexpected(Error(ErrorCode::kFsIsADirectory));
  }

  // 删除文件
  if (parent_dentry->inode->ops == nullptr) {
    return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
  }

  auto result =
      parent_dentry->inode->ops->Unlink(parent_dentry->inode, file_name);
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

  klog::Debug("VFS: unlinked '{}'", path);
  return {};
}

}  // namespace vfs
