/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <etl/observer.h>

#include <cstdint>

#include "kernel_config.hpp"

/// Tick event payload
struct TickEvent {
  uint64_t jiffies{0};
};

/// Observer interface for tick events
using ITickObserver = etl::observer<TickEvent>;

/// Observable base for tick event publishers
using TickObservable =
    etl::observable<ITickObserver, kernel::config::kTickObservers>;
