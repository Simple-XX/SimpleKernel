# Scheduler Design: Unified Preemption and Lock Handoff

- **Date**: 2026-03-22
- **Status**: Draft
- **Topic**: Refined Timer Re-entrancy and Context Switch Locking in SimpleKernel

## 1. Overview
This specification details the transition from fragmented per-CPU boolean flags to a unified `preempt_count` mechanism, inspired by the Linux kernel, while maintaining an educational "Lock Handoff" pattern for context switching.

## 2. Goals
- **Unified Context Tracking**: Consolidate hardware ISR, software ISR, and preemption-disabled states into a single 32-bit counter.
- **Improved Correctness**: Prevent nested scheduling from within interrupts or preemption-disabled sections.
- **Educational Clarity**: Make "interrupt context" and "preemption state" explicitly inspectable by students.
- **SMP Safety**: Ensure consistent state transitions during cross-CPU context switches.

## 3. Architecture

### 3.1 `PreemptCount` Structure
The `PerCpu` structure will include a 32-bit `preempt_count` field:

| Bit Range | Purpose | Incrementer |
|-----------|---------|-------------|
| 0-7       | Preemption Disable Depth | `PreemptDisable()` |
| 8-15      | Software IRQ Depth | `local_bh_disable()` (Reserved) |
| 16-23     | Hardware IRQ Depth | `InterruptGuard` (Trap Entry) |
| 24-31     | NMI Depth | `NmiGuard` (Panic/NMI) |

### 3.2 Helpers and Invariants
- `InInterrupt()`: Returns true if bits 8-31 are non-zero.
- `InHardIrq()`: Returns true if bits 16-23 are non-zero.
- `CanReschedule()`: Returns true if `preempt_count == 0`.
- **Invariant**: Any blocking operation (e.g., `Mutex::Lock()`) MUST assert `!InInterrupt()`.

## 4. Context Switch Mechanism

### 4.1 Lock Handoff Protocol
SimpleKernel continues to use a "Lock Handoff" for its per-CPU `sched_lock`:

1.  **Schedule Start**: `Schedule()` is called.
2.  **Lock Acquisition**: `sched_lock` is acquired. This automatically disables interrupts.
3.  **Preemption Guard**: `PreemptDisable()` is called (preempt_count increments).
4.  **Context Save**: Current task's registers are saved to its TCB.
5.  **Switch**: `switch_to` assembly moves the CPU to the next task's stack and context.
6.  **Handoff Completion**:
    - For existing tasks: `Schedule()` returns, releases `sched_lock`, and calls `PreemptEnable()`.
    - For new tasks: `kernel_thread_bootstrap` releases `sched_lock`, enables interrupts, and decrements the preempt count.

### 4.2 Deferred Preemption
If a timer tick or `Wakeup()` occurs while `preempt_count > 0`, a per-CPU `need_resched` flag is set. `PreemptEnable()` will check this flag when the count returns to zero and trigger `Schedule()`.

## 5. Implementation Details

### 5.1 RAII Guards
Introduce `InterruptGuard` in `src/arch/*/interrupt_main.cpp`:
```cpp
class InterruptGuard {
public:
    InterruptGuard() { IncrementHwIrqCount(); }
    ~InterruptGuard() { DecrementHwIrqCount(); }
};
```

### 5.2 Header Updates
- `src/include/per_cpu.hpp`: Add `uint32_t preempt_count` and `bool need_resched`.
- `src/include/interrupt_base.h`: Add context check helpers.

## 6. Testing Strategy
- **Unit Tests**: Verify `preempt_count` bit manipulation and overflow protection.
- **Integration Tests**: Simulate nested interrupts and verify that `Schedule()` is correctly deferred.
- **Panic Tests**: Assert that calling `Sleep()` or `Mutex::Lock()` from an ISR triggers a kernel panic.
