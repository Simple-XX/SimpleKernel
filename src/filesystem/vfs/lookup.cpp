/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "filesystem.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "kstd_memory"
#include "spinlock.hpp"
#include "vfs_internal.hpp"

namespace vfs {

auto Lookup(const char* path) -> Expected<Dentry*> {
  if (!GetVfsState().initialized) {
    return std::unexpected(Error(ErrorCode::kFsNotMounted));
  }

  if (path == nullptr || path[0] != '/') {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  // 注意：Lookup 是内部函数，由持锁的上层 VFS 函数（Open/MkDir 等）调用，
  // 此处不需要额外加锁，否则会导致 SpinLock 递归死锁。
  // 空路径或根目录
  if (path[0] == '/' && (path[1] == '\0' || path[1] == '/')) {
    if (GetVfsState().root_dentry == nullptr) {
      return std::unexpected(Error(ErrorCode::kFsNotMounted));
    }
    return GetVfsState().root_dentry;
  }

  // 查找路径对应的挂载点
  MountPoint* mp = GetVfsState().mount_table->Lookup(path);
  if (mp == nullptr || mp->root_dentry == nullptr) {
    return std::unexpected(Error(ErrorCode::kFsNotMounted));
  }

  // 从挂载点根开始解析路径
  Dentry* current = mp->root_dentry;
  const char* p = SkipLeadingSlashes(path);

  // 如果挂载路径不是根，需要跳过挂载路径部分
  if (mp->mount_path != nullptr && strcmp(mp->mount_path, "/") != 0) {
    const char* mount_path = SkipLeadingSlashes(mp->mount_path);
    while (*mount_path != '\0') {
      char component[256];
      size_t len = CopyPathComponent(mount_path, component, sizeof(component));
      if (len == 0) {
        break;
      }
      mount_path += len;
      mount_path = SkipLeadingSlashes(mount_path);

      // 跳过挂载路径的组件
      len = CopyPathComponent(p, component, sizeof(component));
      if (len == 0) {
        break;
      }
      p += len;
      p = SkipLeadingSlashes(p);
    }
  }

  // 逐级解析路径组件
  char component[256];
  while (*p != '\0') {
    // 检查当前 dentry 是否是目录
    if (current->inode == nullptr ||
        current->inode->type != FileType::kDirectory) {
      return std::unexpected(Error(ErrorCode::kFsNotADirectory));
    }

    // 提取路径组件
    size_t len = CopyPathComponent(p, component, sizeof(component));
    if (len == 0) {
      break;
    }
    p += len;
    p = SkipLeadingSlashes(p);

    // 处理 "." 和 ".."
    if (strcmp(component, ".") == 0) {
      continue;
    }
    if (strcmp(component, "..") == 0) {
      if (current->parent != nullptr) {
        current = current->parent;
      }
      continue;
    }

    // 在子节点中查找
    Dentry* child = FindChild(current, component);
    if (child == nullptr) {
      // 尝试通过 inode ops 查找
      if (current->inode->ops != nullptr) {
        auto result = current->inode->ops->Lookup(current->inode, component);
        if (!result.has_value()) {
          return std::unexpected(Error(ErrorCode::kFsFileNotFound));
        }

        // 创建新的 dentry
        auto new_child = kstd::make_unique<Dentry>();
        if (!new_child) {
          return std::unexpected(Error(ErrorCode::kOutOfMemory));
        }

        strncpy(new_child->name, component, sizeof(new_child->name) - 1);
        new_child->name[sizeof(new_child->name) - 1] = '\0';
        new_child->inode = result.value();
        child = new_child.get();
        AddChild(current, new_child.release());
      } else {
        return std::unexpected(Error(ErrorCode::kFsFileNotFound));
      }
    }

    current = child;

    // 检查是否遇到挂载点
    if (current->inode != nullptr) {
      MountPoint* next_mp =
          GetVfsState().mount_table->FindByMountDentry(current);
      if (next_mp != nullptr && next_mp != mp) {
        mp = next_mp;
        current = next_mp->root_dentry;
      }
    }
  }

  return current;
}

}  // namespace vfs
