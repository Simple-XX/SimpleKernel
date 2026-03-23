# Design Spec: SimpleKernel vs. Linux Scheduler Comparative Analysis

**Date**: 2026-03-23
**Status**: Draft
**Topic**: Task Management, Scheduling Locking, and Context Switching

## 1. Overview
This document specifies the structure and technical content for a deep-dive comparison between **SimpleKernel** (teaching-oriented) and the **Linux Kernel** (production-grade) task management subsystems. The goal is to provide implementation-level evidence to inform future SimpleKernel design improvements.

## 2. Research Targets

### 2.1 Scheduling & Locking
- **Linux**: Analysis of `rq->lock` in `kernel/sched/core.c`.
  - Mechanism: `raw_spin_lock(&rq->lock)`.
  - Handoff: Analysis of `finish_task_switch()` and its cross-CPU locking invariants.
- **SimpleKernel**: Analysis of `task_table_lock_` and per-CPU `sched_lock`.
  - Mechanism: Global SpinLock vs. Per-CPU SpinLock.

### 2.2 Task Lifecycle & Lookup
- **Linux**: RCU-protected `task_struct` lookup.
  - Path: `include/linux/sched.h`, `kernel/pid.c`.
- **SimpleKernel**: PID-to-TCB hash table with global locking.
  - Path: `src/task/include/task_manager.hpp`.

### 2.3 Signal Delivery from IRQ
- **Linux**: `TIF_SIGPENDING` flag and deferred delivery.
  - Path: `include/linux/sched/signal.h`, `kernel/signal.c`.
- **SimpleKernel**: Synchronous signal handling risks in interrupt context.

### 2.4 Load Balancing Locking
- **Linux**: `double_rq_lock()` and memory-address-ordered acquisition.
  - Path: `kernel/sched/core.c`.

## 3. Comparison Format
The final analysis will be presented as a **Comparative Matrix** with the following columns:
1. **Feature/Problem**: (e.g., "Context Switch Lock Handoff")
2. **SimpleKernel Implementation**: Code snippet and logic summary.
3. **Linux Kernel Implementation**: Code snippet, file reference, and logic summary.
4. **Architectural Trade-off**: Analysis of performance vs. complexity.
5. **Recommendation**: Specific refinement for SimpleKernel.

## 4. Technical Constraints
- No RTTI/Exceptions (SimpleKernel).
- Maintain C++23/C23 standards.
- Use GitHub permalinks for all Linux source references.

## 5. Success Criteria
- [ ] 100% coverage of the 7 user-requested inquiry points.
- [ ] Valid permalinks for Linux v6.13+ sources.
- [ ] Traceable design rationale for SimpleKernel improvements.
