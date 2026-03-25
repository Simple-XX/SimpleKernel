//! TCB 所有权原型——验证 Phase 5 的所有权模型。
//!
//! 该模块仅用于测试。它证明 `TaskControlBlock` 可以：
//! 1. 存储在全局任务表中（`BTreeMap<Pid, Box<TCB>>`）
//! 2. 通过调度器运行队列引用（裸指针）
//! 3. 通过每个 CPU 的 “current_task” 指针引用
//! 4. 在 SpinLock 保护下安全访问
//!
//! 决策：使用 UnsafeCell + 裸指针，并通过 SpinLock 保护访问。

#[cfg(test)]
mod tests {
    extern crate alloc;

    use alloc::boxed::Box;
    use alloc::collections::BTreeMap;
    use alloc::vec::Vec;
    use core::cell::UnsafeCell;

    use crate::sync::SpinLock;

    type Pid = u32;

    #[derive(Debug, PartialEq)]
    enum TaskState {
        Ready,
        Running,
        Blocked,
    }

    struct TaskControlBlock {
        pid: Pid,
        state: TaskState,
        priority: u32,
    }

    impl TaskControlBlock {
        fn new(pid: Pid) -> Self {
            Self {
                pid,
                state: TaskState::Ready,
                priority: 100,
            }
        }
    }

    struct MockScheduler {
        run_queue: Vec<*mut TaskControlBlock>,
    }

    // SAFETY: MockScheduler 仅用于单线程测试
    unsafe impl Send for MockScheduler {}
    unsafe impl Sync for MockScheduler {}

    impl MockScheduler {
        fn new() -> Self {
            Self {
                run_queue: Vec::new(),
            }
        }

        fn enqueue(&mut self, tcb: *mut TaskControlBlock) {
            self.run_queue.push(tcb);
        }

        fn dequeue(&mut self) -> Option<*mut TaskControlBlock> {
            if self.run_queue.is_empty() {
                None
            } else {
                Some(self.run_queue.remove(0))
            }
        }

        fn queue_size(&self) -> usize {
            self.run_queue.len()
        }
    }

    struct TaskTable {
        tasks: BTreeMap<Pid, Box<UnsafeCell<TaskControlBlock>>>,
    }

    impl TaskTable {
        fn new() -> Self {
            Self {
                tasks: BTreeMap::new(),
            }
        }

        fn insert(&mut self, tcb: TaskControlBlock) -> *mut TaskControlBlock {
            let pid = tcb.pid;
            let cell = Box::new(UnsafeCell::new(tcb));
            let ptr = cell.get();
            self.tasks.insert(pid, cell);
            ptr
        }

        fn get_ptr(&self, pid: Pid) -> Option<*mut TaskControlBlock> {
            self.tasks.get(&pid).map(|cell| cell.get())
        }

        fn remove(&mut self, pid: Pid) -> Option<Box<UnsafeCell<TaskControlBlock>>> {
            self.tasks.remove(&pid)
        }
    }

    #[test]
    fn tcb_stored_in_task_table() {
        let task_table: SpinLock<TaskTable> = SpinLock::new(TaskTable::new(), "task_table");

        let mut table = task_table.lock();
        let ptr1 = table.insert(TaskControlBlock::new(1));
        let ptr2 = table.insert(TaskControlBlock::new(2));

        assert!(!ptr1.is_null());
        assert!(!ptr2.is_null());
        assert_ne!(ptr1, ptr2);

        // SAFETY: ptr1 有效，受 task_table 锁保护
        assert_eq!(unsafe { (*ptr1).pid }, 1);
        // SAFETY: ptr2 有效，受 task_table 锁保护
        assert_eq!(unsafe { (*ptr2).pid }, 2);
    }

    #[test]
    fn tcb_referenced_by_scheduler() {
        let task_table: SpinLock<TaskTable> = SpinLock::new(TaskTable::new(), "task_table");
        let scheduler: SpinLock<MockScheduler> = SpinLock::new(MockScheduler::new(), "scheduler");

        let (ptr1, ptr2) = {
            let mut table = task_table.lock();
            let p1 = table.insert(TaskControlBlock::new(1));
            let p2 = table.insert(TaskControlBlock::new(2));
            (p1, p2)
        };

        {
            let mut sched = scheduler.lock();
            sched.enqueue(ptr1);
            sched.enqueue(ptr2);
            assert_eq!(sched.queue_size(), 2);
        }

        {
            let mut sched = scheduler.lock();
            let next = sched.dequeue().expect("should have a task");
            // SAFETY: 指针有效，且持有调度器锁
            let tcb = unsafe { &mut *next };
            tcb.state = TaskState::Running;
            assert_eq!(tcb.pid, 1);
            assert_eq!(tcb.state, TaskState::Running);
        }
    }

    #[test]
    fn tcb_current_task_pointer() {
        let task_table: SpinLock<TaskTable> = SpinLock::new(TaskTable::new(), "task_table");

        let mut current_task: *mut TaskControlBlock;

        let ptr1 = {
            let mut table = task_table.lock();
            table.insert(TaskControlBlock::new(1))
        };

        let ptr2 = {
            let mut table = task_table.lock();
            table.insert(TaskControlBlock::new(2))
        };

        // SAFETY: ptr1 有效，模拟上下文切换
        current_task = ptr1;
        assert_eq!(unsafe { (*current_task).pid }, 1);

        // SAFETY: ptr2 有效，切换 current_task
        current_task = ptr2;
        assert_eq!(unsafe { (*current_task).pid }, 2);

        let _ = current_task;
    }

    #[test]
    fn tcb_accessed_through_table_and_scheduler() {
        let task_table: SpinLock<TaskTable> = SpinLock::new(TaskTable::new(), "task_table");

        let ptr = {
            let mut table = task_table.lock();
            table.insert(TaskControlBlock::new(42))
        };

        // SAFETY: 通过调度器引用修改
        unsafe {
            (*ptr).priority = 50;
        }

        {
            let table = task_table.lock();
            let table_ptr = table.get_ptr(42).expect("task should exist");
            // SAFETY: 同一个 TCB 通过两条路径访问——二者都在锁保护下
            assert_eq!(unsafe { (*table_ptr).priority }, 50);
        }
    }

    #[test]
    fn tcb_removal_from_table() {
        let task_table: SpinLock<TaskTable> = SpinLock::new(TaskTable::new(), "task_table");

        {
            let mut table = task_table.lock();
            table.insert(TaskControlBlock::new(1));
        }

        {
            let mut table = task_table.lock();
            let removed = table.remove(1);
            assert!(removed.is_some());
            assert!(table.get_ptr(1).is_none());
        }
    }
}
