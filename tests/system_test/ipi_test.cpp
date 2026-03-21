/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include <atomic>
#include <cstdint>

#include "basic_info.hpp"
#include "interrupt_base.h"
#include "interrupt.h"
#include "kernel.h"
#include "kernel_log.hpp"
#include "syscall.hpp"
#include "system_test.h"

namespace {

std::atomic<uint32_t> g_ipi_received_count{0};

using InterruptDelegate = InterruptBase::InterruptDelegate;

// ===========================================================================
// IPI handler for delivery verification tests.
// Performs original handler duties (clear SSIP on riscv64) + increments
// the shared counter so the test thread can observe receipt.
// ===========================================================================

auto TestIpiHandler(uint64_t /*cause*/,
                    cpu_io::TrapContext* /*context*/) -> uint64_t {
#if defined(__riscv)
  cpu_io::Sip::Ssip::Clear();
#endif
  g_ipi_received_count.fetch_add(1, std::memory_order_release);
  return 0;
}

auto OriginalIpiHandler(uint64_t /*cause*/,
                        cpu_io::TrapContext* /*context*/) -> uint64_t {
#if defined(__riscv)
  cpu_io::Sip::Ssip::Clear();
#endif
  return 0;
}

void InstallTestIpiHandler() {
#if defined(__riscv)
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kSupervisorSoftwareInterrupt,
      InterruptDelegate::create<TestIpiHandler>());
#elif defined(__aarch64__)
  InterruptSingleton::instance().RegisterInterruptFunc(
      0, InterruptDelegate::create<TestIpiHandler>());
#endif
}

void RestoreIpiHandler() {
#if defined(__riscv)
  InterruptSingleton::instance().RegisterInterruptFunc(
      cpu_io::ScauseInfo::kSupervisorSoftwareInterrupt,
      InterruptDelegate::create<OriginalIpiHandler>());
#elif defined(__aarch64__)
  InterruptSingleton::instance().RegisterInterruptFunc(
      0, InterruptDelegate::create<OriginalIpiHandler>());
#endif
}

}  // namespace

auto ipi_test() -> bool {
  klog::Info("ipi_test: start");

  auto& interrupt = InterruptSingleton::instance();
  auto core_count = BasicInfoSingleton::instance().core_count;

  // ===========================================================================
  // Test 1: BroadcastIpi API
  // ===========================================================================
  {
    auto result = interrupt.BroadcastIpi();
    EXPECT_TRUE(result.has_value(), "BroadcastIpi should return success");
    klog::Info("ipi_test: BroadcastIpi API passed");
  }

  // ===========================================================================
  // Test 2: SendIpi to single core
  // ===========================================================================
  if (core_count >= 2) {
    auto result = interrupt.SendIpi(1UL << 1);
    EXPECT_TRUE(result.has_value(), "SendIpi to core 1 should return success");
    klog::Info("ipi_test: SendIpi single core passed");
  }

  // ===========================================================================
  // Test 3: SendIpi to self
  // ===========================================================================
  {
    auto self_mask = 1UL << cpu_io::GetCurrentCoreId();
    auto result = interrupt.SendIpi(self_mask);
    EXPECT_TRUE(result.has_value(), "SendIpi to self should return success");
    klog::Info("ipi_test: SendIpi to self passed");
  }

  // ===========================================================================
  // Test 4: SendIpi with invalid mask returns error (riscv64 only)
  // ===========================================================================
#if defined(__riscv)
  {
    auto result = interrupt.SendIpi(~0UL);
    EXPECT_FALSE(result.has_value(),
                 "SendIpi with invalid mask should return error");
    klog::Info("ipi_test: SendIpi invalid mask passed");
  }
#endif

  // ===========================================================================
  // Test 5: BroadcastIpi cross-core delivery verification
  // ===========================================================================
  if (core_count >= 2) {
    g_ipi_received_count.store(0, std::memory_order_release);
    InstallTestIpiHandler();

    auto result = interrupt.BroadcastIpi();
    EXPECT_TRUE(result.has_value(),
                "BroadcastIpi for delivery test should succeed");

    int timeout = 100;
    while (timeout > 0 &&
           g_ipi_received_count.load(std::memory_order_acquire) == 0) {
      (void)sys_sleep(10);
      timeout--;
    }

    auto count = g_ipi_received_count.load(std::memory_order_acquire);
    EXPECT_GT(count, 0u,
              "BroadcastIpi should be received by at least one core");
    klog::Info("ipi_test: BroadcastIpi delivery verified, count={}", count);

    RestoreIpiHandler();
  }

  // ===========================================================================
  // Test 6: SendIpi targeted delivery verification
  // ===========================================================================
  if (core_count >= 2) {
    g_ipi_received_count.store(0, std::memory_order_release);
    InstallTestIpiHandler();

    auto current = cpu_io::GetCurrentCoreId();
    uint64_t target = (current == 0) ? 1 : 0;

    auto result = interrupt.SendIpi(1UL << target);
    EXPECT_TRUE(result.has_value(), "SendIpi targeted delivery should succeed");

    int timeout = 100;
    while (timeout > 0 &&
           g_ipi_received_count.load(std::memory_order_acquire) == 0) {
      (void)sys_sleep(10);
      timeout--;
    }

    auto count = g_ipi_received_count.load(std::memory_order_acquire);
    EXPECT_GT(count, 0u, "Targeted SendIpi should be received");
    klog::Info("ipi_test: SendIpi targeted delivery verified, count={}", count);

    RestoreIpiHandler();
  }

  klog::Info("ipi_test: all sub-tests passed");
  return true;
}
