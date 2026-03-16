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

auto Close(File* file) -> Expected<void> {
  if (file == nullptr) {
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }

  LockGuard<SpinLock> guard(GetVfsState().vfs_lock_);
  if (file->ops != nullptr) {
    auto result = file->ops->Close(file);
    if (!result.has_value()) {
      return result;
    }
  }

  if (file->dentry != nullptr && file->dentry->ref_count > 0) {
    file->dentry->ref_count--;
  }

  etl::unique_ptr<File> file_guard(file);
  return {};
}

}  // namespace vfs
