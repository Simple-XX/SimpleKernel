# Linux Scheduler Internals Comparative Analysis Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Generate a deep-technical comparative analysis between SimpleKernel and Linux scheduler internals with verified source evidence.

**Architecture:** Approach 2 (Design Pattern & Trade-off Analysis) using `grep_app` for fast Linux code discovery and `gh repo clone` for deep analysis.

**Tech Stack:** GitHub CLI, Grep.app, Context7 (Docs), Git.

---

### Task 1: Linux Source Evidence Gathering (Locking & Handoff)

**Files:**
- Create: `docs/research/linux_locking_evidence.md`
- Target: `kernel/sched/core.c`, `kernel/sched/sched.h` (Linux v6.13+)

- [ ] **Step 1: Get Linux HEAD SHA for permalinks**
Run: `gh api repos/torvalds/linux/commits/HEAD --jq '.sha'`
Expected: A 40-character commit hash.

- [ ] **Step 2: Locate `rq->lock` acquisition in `__schedule`**
Run: `grep_app_searchGitHub(query: "raw_spin_rq_lock(", repo: "torvalds/linux", path: "kernel/sched/core.c")`
Expected: Line number in `__schedule` where the local runqueue is locked.

- [ ] **Step 3: Locate `finish_task_switch` lock release**
Run: `grep_app_searchGitHub(query: "finish_task_switch(", repo: "torvalds/linux", path: "kernel/sched/core.c")`
Expected: Line number where `finish_lock_switch` or `raw_spin_rq_unlock` is called by the incoming task.

- [ ] **Step 4: Commit research evidence**
Run: `git add docs/research/linux_locking_evidence.md && git commit -m "research: gather linux scheduling lock evidence"`

---

### Task 2: SimpleKernel Source Baseline Analysis

**Files:**
- Create: `docs/research/simplekernel_locking_baseline.md`
- Analyze: `src/task/include/task_manager.hpp`, `src/task/task_manager.cpp`

- [ ] **Step 1: Identify `task_table_lock_` usage**
Run: `grep -n "task_table_lock_" src/task/task_manager.cpp`
Expected: Verification of global lock held during task lookup/migration.

- [ ] **Step 2: Identify `sched_lock` usage**
Run: `grep -n "sched_lock" src/task/include/task_manager.hpp`
Expected: Definition of per-CPU runqueue lock.

- [ ] **Step 3: Commit baseline**
Run: `git add docs/research/simplekernel_locking_baseline.md && git commit -m "research: gather simplekernel locking baseline"`

---

### Task 3: Signal & IRQ Latency Comparison

**Files:**
- Create: `docs/research/signal_delivery_comparison.md`
- Linux: `include/linux/sched/signal.h`, `kernel/signal.c`

- [ ] **Step 1: Locate `TIF_SIGPENDING` flag check in Linux**
Run: `grep_app_searchGitHub(query: "TIF_SIGPENDING", repo: "torvalds/linux", path: "include/linux/sched/signal.h")`
Expected: Bit definition in thread info flags.

- [ ] **Step 2: Contrast with SimpleKernel signal path**
Run: Analyze `src/task/signal.cpp` (if exists) or signal handling in `task_manager.cpp`.

---

### Task 4: Synthesis & Final Matrix Generation

**Files:**
- Create: `docs/analysis/scheduler_comparison_matrix.md`

- [ ] **Step 1: Build the Comparison Matrix**
Construct a Markdown table matching the spec (Feature, SimpleKernel, Linux, Trade-off, Recommendation).

- [ ] **Step 2: Verify all Permalinks**
Ensure all Linux links use the SHA gathered in Task 1.

- [ ] **Step 3: Final Commit**
Run: `git add docs/analysis/scheduler_comparison_matrix.md && git commit -m "docs: finalize scheduler comparative analysis"`
