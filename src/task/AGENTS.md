# AGENTS.md — src/task/

## OVERVIEW
Task management subsystem: schedulers (CFS/FIFO/RR/Idle), TaskControlBlock, TaskManager singleton, sync primitives (mutex via spinlock), syscall-level task operations (clone, exit, sleep, wait, wakeup, block).

## STRUCTURE
```
include/
  scheduler_base.hpp       # SchedulerBase ABC — PickNext, Enqueue, Dequeue, OnTick, OnTimeSliceExpired
  cfs_scheduler.hpp        # CFS (Completely Fair Scheduler) — vruntime-based
  fifo_scheduler.hpp       # FIFO — first-in first-out, no preemption
  rr_scheduler.hpp         # Round-Robin — time-slice based preemption
  idle_scheduler.hpp       # Idle — runs when no other tasks ready
  task_control_block.hpp   # TCB — task state, context, priority, stack
  task_fsm.hpp             # Task state machine — valid state transitions
  task_manager.hpp         # TaskManagerSingleton (etl::singleton<TaskManager>) — owns schedulers, dispatches
  task_messages.hpp        # Task-related message types for inter-task communication
  lifecycle_messages.hpp   # Task lifecycle event message definitions
  resource_id.hpp          # Typed resource IDs (TaskId, etc.)
schedule.cpp               # Schedule() — main scheduling loop, context switch trigger
task_control_block.cpp     # TCB construction, state transitions
task_manager.cpp           # TaskManager — AddTask, InitCurrentCore, scheduler selection
tick_update.cpp            # Timer tick handler — calls scheduler TickUpdate
clone.cpp                  # sys_clone — task creation
exit.cpp                   # sys_exit — task termination, cleanup
sleep.cpp                  # sys_sleep — timed task suspension
wait.cpp                   # sys_wait — wait for child task
wakeup.cpp                 # Wakeup — move task from blocked to ready
block.cpp                  # Block — move task from ready to blocked
mutex.cpp                  # Mutex implementation (uses SpinLock internally)
```

## WHERE TO LOOK
- **Adding a scheduler** → Subclass `SchedulerBase` (see `cfs_scheduler.hpp`), implement `PickNext()`, `Enqueue()`, `Dequeue()`, `OnTick()`
- **Task lifecycle** → clone.cpp (create) → schedule.cpp (run) → exit.cpp (destroy)
- **Context switch** → `schedule.cpp` calls arch-specific `switch.S` via function pointer
- **Sync primitives** → `mutex.cpp` uses `SpinLock` + task blocking; `spinlock.hpp` in `src/include/`

## CONVENTIONS
- One syscall operation per .cpp file (clone, exit, sleep, wait, wakeup, block)
- Schedulers own their internal run queues — TaskManager dispatches to per-policy schedulers
- `TaskManagerSingleton::instance()` (defined in `task_manager.hpp`) is the global entry point
- TCB contains arch-specific context pointer — populated by `switch.S`
- `Balance()` in `task_manager.cpp`: cross-core work-stealing (steals kNormal tasks from most-loaded core, called every 64 ticks)

## ANTI-PATTERNS
- **DO NOT** call Schedule() before TaskManager initialization in boot sequence
- **DO NOT** hold SpinLock across context switch boundaries — deadlock
- **DO NOT** access TCB fields without proper locking (SpinLock or LockGuard)
- **DO NOT** add scheduler implementations without corresponding test in tests/
