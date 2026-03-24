/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "filesystem.hpp"
#include "kernel_log.hpp"
#include "kstd_cstring"
#include "spinlock.hpp"
#include "vfs_internal.hpp"

namespace vfs {

auto Read(File* file, void* buf, size_t count) -> Expected<size_t> {
  if (file == nullptr || buf == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  if (file->ops == nullptr) {
    return std::unexpected(Error(ErrorCode::kDeviceNotSupported));
  }

  return file->ops->Read(file, buf, count);
}

}  // namespace vfs
