/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "vfs.hpp"

#include "filesystem.hpp"
#include "kernel_log.hpp"
#include "kstd_cstdio"
#include "kstd_cstring"
#include "mount.hpp"
#include "spinlock.hpp"
#include "vfs_internal.hpp"

namespace vfs {

auto GetVfsState() -> VfsState& {
  static VfsState state;
  return state;
}

auto SkipLeadingSlashes(const char* path) -> const char* {
  while (*path == '/') {
    ++path;
  }
  return path;
}

auto CopyPathComponent(const char* src, char* dst, size_t dst_size) -> size_t {
  size_t i = 0;
  while (*src != '\0' && *src != '/' && i < dst_size - 1) {
    dst[i++] = *src++;
  }
  dst[i] = '\0';
  return i;
}

auto FindChild(Dentry* parent, const char* name) -> Dentry* {
  if (parent == nullptr || name == nullptr) {
    return nullptr;
  }

  Dentry* child = parent->children;
  while (child != nullptr) {
    if (strcmp(child->name, name) == 0) {
      return child;
    }
    child = child->next_sibling;
  }
  return nullptr;
}

auto AddChild(Dentry* parent, Dentry* child) -> void {
  if (parent == nullptr || child == nullptr) {
    return;
  }

  child->parent = parent;
  child->next_sibling = parent->children;
  parent->children = child;
}

auto RemoveChild(Dentry* parent, Dentry* child) -> void {
  if (parent == nullptr || child == nullptr) {
    return;
  }

  Dentry** current = &parent->children;
  while (*current != nullptr) {
    if (*current == child) {
      *current = child->next_sibling;
      child->parent = nullptr;
      child->next_sibling = nullptr;
      return;
    }
    current = &(*current)->next_sibling;
  }
}

auto Init() -> Expected<void> {
  if (GetVfsState().initialized) {
    return {};
  }

  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  klog::Info("VFS: initializing...");

  // 初始化挂载表（使用全局单例，与 GetMountTable() 统一）
  GetVfsState().mount_table = &GetMountTable();

  GetVfsState().initialized = true;
  klog::Info("VFS: initialization complete");
  return {};
}

auto GetRootDentry() -> Dentry* { return GetVfsState().root_dentry; }

// 内部接口：设置根 dentry
auto SetRootDentry(Dentry* dentry) -> void {
  GetVfsState().root_dentry = dentry;
}

// 内部接口：获取挂载表
auto GetMountTableInternal() -> MountTable* {
  return GetVfsState().mount_table;
}

}  // namespace vfs
