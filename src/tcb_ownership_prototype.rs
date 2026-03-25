//! TCB 所有权原型——验证 Phase 5 的 Arc\<TCB\> 所有权模型。
//!
//! 该模块仅用于测试。它证明 `Arc<TaskControlBlock>` 可以：
//! 1. 存储在全局任务表中（`BTreeMap<Pid, Arc<TCB>>`）
//! 2. 被调度器运行队列持有（`VecDeque<Arc<TCB>>`）
//! 3. 被每个 CPU 的 `current_task` 引用（`Option<Arc<TCB>>`）
//! 4. 从任务表中移除后，若调度器仍持有引用则 TCB 不会被释放
//!
//! 决策（2026-03-25 生态调研）：采用 Arc\<TCB\> 替代裸指针，
//! 与 rCore-Tutorial-v3 一致，消除 use-after-free 风险。
//! 可变字段通过内部 SpinLock 保护。

#[cfg(test)]
mod tests {
    extern crate alloc;

    use alloc::collections::BTreeMap;
    use alloc::collections::VecDeque;
    use alloc::sync::Arc;

    use crate::sync::SpinLock;

    type Pid = u32;

    #[derive(Debug, Clone, Copy, PartialEq)]
    enum TaskState {
        Ready,
        Running,
        #[allow(dead_code)]
        Blocked,
    }

    /// 不可变字段直接在外层（创建后不变）
    struct TaskControlBlock {
        pid: Pid,
        name: &'static str,
        /// 可变字段在 SpinLock 内部
        inner: SpinLock<TcbInner>,
    }

    struct TcbInner {
        state: TaskState,
        priority: u32,
    }

    impl TaskControlBlock {
        fn new(pid: Pid, name: &'static str) -> Arc<Self> {
            Arc::new(Self {
                pid,
                name,
                inner: SpinLock::new(
                    TcbInner {
                        state: TaskState::Ready,
                        priority: 100,
                    },
                    "tcb_inner",
                ),
            })
        }
    }

    /// 任务表持有 Arc —— 任务的"权威"存储
    struct TaskTable {
        tasks: BTreeMap<Pid, Arc<TaskControlBlock>>,
    }

    impl TaskTable {
        fn new() -> Self {
            Self {
                tasks: BTreeMap::new(),
            }
        }

        fn insert(&mut self, task: Arc<TaskControlBlock>) {
            self.tasks.insert(task.pid, task);
        }

        fn get(&self, pid: Pid) -> Option<&Arc<TaskControlBlock>> {
            self.tasks.get(&pid)
        }

        fn remove(&mut self, pid: Pid) -> Option<Arc<TaskControlBlock>> {
            self.tasks.remove(&pid)
        }
    }

    /// 调度器持有 Arc clone —— 与任务表共享所有权
    struct MockScheduler {
        run_queue: VecDeque<Arc<TaskControlBlock>>,
    }

    impl MockScheduler {
        fn new() -> Self {
            Self {
                run_queue: VecDeque::new(),
            }
        }

        fn enqueue(&mut self, task: Arc<TaskControlBlock>) {
            self.run_queue.push_back(task);
        }

        fn dequeue(&mut self) -> Option<Arc<TaskControlBlock>> {
            self.run_queue.pop_front()
        }

        fn queue_size(&self) -> usize {
            self.run_queue.len()
        }
    }

    // ---- 测试 ----

    #[test]
    fn tcb_stored_in_task_table() {
        let task_table: SpinLock<TaskTable> = SpinLock::new(TaskTable::new(), "task_table");

        let task1 = TaskControlBlock::new(1, "task_1");
        let task2 = TaskControlBlock::new(2, "task_2");

        let mut table = task_table.lock();
        table.insert(task1);
        table.insert(task2);

        assert_eq!(table.get(1).unwrap().pid, 1);
        assert_eq!(table.get(2).unwrap().pid, 2);
        assert_eq!(table.get(1).unwrap().name, "task_1");
    }

    #[test]
    fn tcb_shared_between_table_and_scheduler() {
        let task_table: SpinLock<TaskTable> = SpinLock::new(TaskTable::new(), "task_table");
        let scheduler: SpinLock<MockScheduler> = SpinLock::new(MockScheduler::new(), "scheduler");

        let task = TaskControlBlock::new(1, "shared");

        // 任务表和调度器各持有一个 Arc clone
        {
            let mut table = task_table.lock();
            table.insert(Arc::clone(&task));
        }
        {
            let mut sched = scheduler.lock();
            sched.enqueue(Arc::clone(&task));
        }

        // Arc 引用计数 = 3（task 局部变量 + 任务表 + 调度器）
        assert_eq!(Arc::strong_count(&task), 3);

        // 通过调度器修改可变状态
        {
            let mut sched = scheduler.lock();
            let next = sched.dequeue().expect("should have a task");
            let mut inner = next.inner.lock();
            inner.state = TaskState::Running;
        }

        // 通过任务表读取——可以看到调度器的修改
        {
            let table = task_table.lock();
            let t = table.get(1).expect("task should exist");
            let inner = t.inner.lock();
            assert_eq!(inner.state, TaskState::Running);
        }
    }

    #[test]
    fn tcb_survives_table_removal() {
        let task_table: SpinLock<TaskTable> = SpinLock::new(TaskTable::new(), "task_table");
        let scheduler: SpinLock<MockScheduler> = SpinLock::new(MockScheduler::new(), "scheduler");

        let task = TaskControlBlock::new(42, "survivor");

        // 插入任务表和调度器
        {
            let mut table = task_table.lock();
            table.insert(Arc::clone(&task));
        }
        {
            let mut sched = scheduler.lock();
            sched.enqueue(Arc::clone(&task));
        }

        // 从任务表移除
        {
            let mut table = task_table.lock();
            let removed = table.remove(42);
            assert!(removed.is_some());
            assert!(table.get(42).is_none());
        }

        // 调度器仍然持有有效引用——不是 use-after-free！
        {
            let mut sched = scheduler.lock();
            let t = sched
                .dequeue()
                .expect("scheduler should still have the task");
            assert_eq!(t.pid, 42);
            let inner = t.inner.lock();
            assert_eq!(inner.state, TaskState::Ready);
        }
    }

    #[test]
    fn tcb_current_task_switching() {
        let task1 = TaskControlBlock::new(1, "task_1");
        let task2 = TaskControlBlock::new(2, "task_2");

        // 模拟 per_cpu.running_task: Option<Arc<TCB>>
        // 切换到 task1
        let mut current_task: Option<Arc<TaskControlBlock>> = Some(Arc::clone(&task1));
        assert_eq!(current_task.as_ref().unwrap().pid, 1);

        // 切换到 task2——旧引用自动释放
        current_task = Some(Arc::clone(&task2));
        assert_eq!(current_task.as_ref().unwrap().pid, 2);

        // 清除——引用自动释放
        current_task = None;
        assert!(current_task.is_none());
    }

    #[test]
    fn tcb_priority_modification() {
        let task = TaskControlBlock::new(10, "prio_test");

        // 修改优先级
        {
            let mut inner = task.inner.lock();
            inner.priority = 50;
        }

        // 读取——确认修改
        {
            let inner = task.inner.lock();
            assert_eq!(inner.priority, 50);
        }
    }

    #[test]
    fn scheduler_enqueue_dequeue() {
        let scheduler: SpinLock<MockScheduler> = SpinLock::new(MockScheduler::new(), "scheduler");

        let task1 = TaskControlBlock::new(1, "first");
        let task2 = TaskControlBlock::new(2, "second");

        {
            let mut sched = scheduler.lock();
            sched.enqueue(Arc::clone(&task1));
            sched.enqueue(Arc::clone(&task2));
            assert_eq!(sched.queue_size(), 2);
        }

        {
            let mut sched = scheduler.lock();
            let next = sched.dequeue().unwrap();
            assert_eq!(next.pid, 1); // FIFO 顺序
            assert_eq!(sched.queue_size(), 1);
        }

        {
            let mut sched = scheduler.lock();
            let next = sched.dequeue().unwrap();
            assert_eq!(next.pid, 2);
            assert!(sched.dequeue().is_none()); // 队列空
        }
    }
}
