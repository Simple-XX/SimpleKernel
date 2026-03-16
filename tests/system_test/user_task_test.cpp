/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include <cstddef>
#include <cstdint>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kstd_cstring"
#include "kstd_libcxx.h"
#include "sk_stdlib.h"
#include "syscall.hpp"
#include "system_test.h"
#include "task_control_block.hpp"
#include "task_manager.hpp"

/// @todo Implement user task tests when ELF loading path is testable
auto user_task_test() -> bool {
  klog::Info("user_task_test: SKIPPED (not yet implemented)");
  return true;
}
