/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "interrupt.h"

#include <cpu_io.h>

#include <cstddef>
#include <cstdint>

#include "arch.h"
#include "basic_info.hpp"
#include "interrupt_base.h"
#include "kernel.h"
#include "kstd_cstring"
#include "kstd_libcxx.h"
#include "sk_stdlib.h"
#include "system_test.h"

/// @todo 等用户态调通后补上
auto interrupt_test() -> bool {
  klog::Info("interrupt_test: start");

  (void)InterruptSingleton::instance().BroadcastIpi();

  klog::Info("interrupt_test: broadcast ipi passed");

  return true;
}
