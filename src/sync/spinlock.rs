#![allow(dead_code)]

use core::mem::ManuallyDrop;
use core::ops::{Deref, DerefMut};
use core::sync::atomic::{AtomicUsize, Ordering};

use crate::per_cpu;

/// 用于强制获取顺序的锁级别常量。
///
/// 数值更小的级别必须优先获取。
/// 在持有更高级别锁时获取更低级别的锁会触发 halt。
pub mod lock_level {
    pub const SCHED_LOCK: u8 = 0;
    pub const TASK_TABLE_LOCK: u8 = 1;
    pub const INTERRUPT_THREADS_LOCK: u8 = 2;
    pub const UNCLASSIFIED: u8 = 0xFF;
}

#[cfg(all(target_arch = "riscv64", target_os = "none"))]
mod interrupt_ops {
    #[inline(always)]
    pub fn get_status() -> bool {
        riscv::register::sstatus::read().sie()
    }

    #[inline(always)]
    pub fn disable() {
        riscv::interrupt::supervisor::disable();
    }

    #[inline(always)]
    pub fn enable() {
        unsafe { riscv::interrupt::supervisor::enable() };
    }
}

#[cfg(all(target_arch = "aarch64", target_os = "none"))]
mod interrupt_ops {
    #[inline(always)]
    pub fn get_status() -> bool {
        let daif: u64;
        unsafe { core::arch::asm!("mrs {daif}, daif", daif = out(reg) daif) };
        (daif & (1 << 7)) == 0
    }

    #[inline(always)]
    pub fn disable() {
        unsafe { core::arch::asm!("msr daifset, #2") };
    }

    #[inline(always)]
    pub fn enable() {
        unsafe { core::arch::asm!("msr daifclr, #2") };
    }
}

#[cfg(not(all(
    any(target_arch = "riscv64", target_arch = "aarch64"),
    target_os = "none"
)))]
mod interrupt_ops {
    #[inline(always)]
    pub fn get_status() -> bool {
        false
    }
    #[inline(always)]
    pub fn disable() {}
    #[inline(always)]
    pub fn enable() {}
}

const NO_OWNER: usize = usize::MAX;

/// 中断安全的自旋锁，支持锁级别顺序检查。
///
/// 基于 `spin::Mutex` 实现自旋逻辑，额外提供：
/// - 通过 RAII guard 自动禁用/恢复中断
/// - 递归加锁检测
/// - 锁级别层次强制，防止死锁
///
/// # Usage
/// ```ignore
/// static MY_LOCK: SpinLock<MyData> = SpinLock::new(MyData::new(), "my_lock");
/// let guard = MY_LOCK.lock();
/// // guard 被丢弃时恢复中断
/// ```
pub struct SpinLock<T> {
    inner: spin::Mutex<T>,
    owner_core: AtomicUsize,
    level: u8,
    name: &'static str,
}

unsafe impl<T: Send> Send for SpinLock<T> {}
unsafe impl<T: Send> Sync for SpinLock<T> {}

impl<T> SpinLock<T> {
    #[must_use]
    pub const fn new(data: T, name: &'static str) -> Self {
        Self {
            inner: spin::Mutex::new(data),
            owner_core: AtomicUsize::new(NO_OWNER),
            level: lock_level::UNCLASSIFIED,
            name,
        }
    }

    #[must_use]
    pub const fn new_with_level(data: T, name: &'static str, level: u8) -> Self {
        Self {
            inner: spin::Mutex::new(data),
            owner_core: AtomicUsize::new(NO_OWNER),
            level,
            name,
        }
    }

    /// 获取锁，返回 RAII guard。
    ///
    /// # Panics
    /// - 同一核心递归加锁
    /// - 锁级别顺序违反
    pub fn lock(&self) -> SpinLockGuard<'_, T> {
        let saved_intr = interrupt_ops::get_status();
        interrupt_ops::disable();

        // 中断已关闭，同核心递归加锁会导致永久自旋，必须提前检测
        if self.owner_core.load(Ordering::Relaxed) == per_cpu::current_core_id() {
            Self::fatal(self.name, "recursive lock");
        }

        let guard = self.inner.lock();
        self.post_acquire();

        SpinLockGuard {
            lock: self,
            guard: ManuallyDrop::new(guard),
            saved_intr,
        }
    }

    /// 尝试在不阻塞的情况下获取锁。
    pub fn try_lock(&self) -> Option<SpinLockGuard<'_, T>> {
        let saved_intr = interrupt_ops::get_status();
        interrupt_ops::disable();

        match self.inner.try_lock() {
            Some(guard) => {
                self.post_acquire();
                Some(SpinLockGuard {
                    lock: self,
                    guard: ManuallyDrop::new(guard),
                    saved_intr,
                })
            }
            None => {
                if saved_intr {
                    interrupt_ops::enable();
                }
                None
            }
        }
    }

    pub fn is_locked(&self) -> bool {
        self.inner.is_locked()
    }

    fn post_acquire(&self) {
        self.owner_core
            .store(per_cpu::current_core_id(), Ordering::Release);
        self.check_lock_order();
        self.push_lock_stack();
    }

    fn pre_release(&self) {
        self.pop_lock_stack();
        self.owner_core.store(NO_OWNER, Ordering::Release);
    }

    #[cold]
    #[inline(never)]
    fn fatal(name: &str, reason: &str) -> ! {
        crate::logging::raw_put("FATAL: SpinLock '");
        crate::logging::raw_put(name);
        crate::logging::raw_put("': ");
        crate::halt::halt(reason);
    }

    fn check_lock_order(&self) {
        if self.level == lock_level::UNCLASSIFIED {
            return;
        }
        // SAFETY: 中断已禁用，无同核心并发访问
        let stack = &unsafe { per_cpu::current_per_cpu() }.lock_stack;
        if stack.depth > 0 {
            let top = stack.entries[stack.depth - 1].level;
            if top != lock_level::UNCLASSIFIED && self.level <= top {
                Self::fatal(self.name, "lock order violation");
            }
        }
    }

    fn push_lock_stack(&self) {
        // SAFETY: 中断已禁用
        let stack = &mut unsafe { per_cpu::current_per_cpu() }.lock_stack;
        if stack.depth >= per_cpu::LockStack::MAX_DEPTH {
            crate::halt::halt("FATAL: lock stack overflow\n");
        }
        stack.entries[stack.depth] = per_cpu::LockStackEntry {
            lock_ptr: self as *const Self as *const (),
            level: self.level,
        };
        stack.depth += 1;
    }

    fn pop_lock_stack(&self) {
        // SAFETY: 中断已禁用
        let stack = &mut unsafe { per_cpu::current_per_cpu() }.lock_stack;
        if stack.depth == 0 {
            crate::halt::halt("FATAL: lock stack underflow\n");
        }
        if stack.entries[stack.depth - 1].lock_ptr != (self as *const Self as *const ()) {
            Self::fatal(self.name, "lock stack corrupted");
        }
        stack.depth -= 1;
    }
}

/// RAII guard。丢弃时释放锁并恢复中断状态。
pub struct SpinLockGuard<'a, T> {
    lock: &'a SpinLock<T>,
    guard: ManuallyDrop<spin::MutexGuard<'a, T>>,
    saved_intr: bool,
}

impl<T> Deref for SpinLockGuard<'_, T> {
    type Target = T;

    fn deref(&self) -> &T {
        &self.guard
    }
}

impl<T> DerefMut for SpinLockGuard<'_, T> {
    fn deref_mut(&mut self) -> &mut T {
        &mut self.guard
    }
}

impl<T> Drop for SpinLockGuard<'_, T> {
    fn drop(&mut self) {
        self.lock.pre_release();
        // SAFETY: guard 有效且仅在此处 drop 一次
        unsafe { ManuallyDrop::drop(&mut self.guard) };
        if self.saved_intr {
            interrupt_ops::enable();
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn lock_and_unlock() {
        let lock = SpinLock::new(42u32, "test");
        {
            let guard = lock.lock();
            assert_eq!(*guard, 42);
        }
        assert!(!lock.is_locked());
    }

    #[test]
    fn guard_provides_mutable_access() {
        let lock = SpinLock::new(0u32, "test_mut");
        {
            let mut guard = lock.lock();
            *guard = 99;
        }
        let guard = lock.lock();
        assert_eq!(*guard, 99);
    }

    #[test]
    fn try_lock_succeeds_when_free() {
        let lock = SpinLock::new(7u32, "try_lock_test");
        let guard = lock.try_lock();
        assert!(guard.is_some());
        assert_eq!(*guard.expect("lock should succeed"), 7);
    }

    #[test]
    fn try_lock_fails_when_held() {
        let lock = SpinLock::new(0u32, "try_lock_held");
        let _g = lock.lock();
        let second = lock.try_lock();
        assert!(second.is_none());
    }

    #[test]
    fn guard_drop_releases_lock() {
        let lock = SpinLock::new(0u32, "drop_test");
        {
            let _g = lock.lock();
            assert!(lock.is_locked());
        }
        assert!(!lock.is_locked());
    }

    #[test]
    fn new_with_level() {
        let lock = SpinLock::new_with_level(0u32, "leveled", lock_level::SCHED_LOCK);
        let _g = lock.lock();
        assert!(lock.is_locked());
    }
}
