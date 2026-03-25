use crate::config;
use core::cell::SyncUnsafeCell;
use spin::Once;

// P3 将字段类型升级为 PhysAddr，P4+ 读取这些字段
#[allow(dead_code)]
pub struct BasicInfo {
    pub physical_memory_addr: u64,
    pub physical_memory_size: usize,
    pub kernel_addr: u64,
    pub kernel_size: usize,
    pub elf_addr: u64,
    pub fdt_addr: u64,
    pub core_count: usize,
}

impl BasicInfo {
    #[must_use]
    #[allow(dead_code)] // P1 init.rs 中通过 BASIC_INFO.call_once() 使用
    pub const fn new() -> Self {
        Self {
            physical_memory_addr: 0,
            physical_memory_size: 0,
            kernel_addr: 0,
            kernel_size: 0,
            elf_addr: 0,
            fdt_addr: 0,
            core_count: 0,
        }
    }
}

pub static BASIC_INFO: Once<BasicInfo> = Once::new();

/// 锁栈条目——记录当前持有的 SpinLock 及其级别。
#[derive(Clone, Copy)]
pub struct LockStackEntry {
    /// 指向 SpinLock 的类型擦除原始指针，仅用于诊断比较，不会解引用。
    pub lock_ptr: *const (),
    /// 所持锁的级别。
    pub level: u8,
}

// SAFETY: lock_ptr 仅用于比较、从不解引用；访问发生在中断关闭的 per-CPU 上下文中。
unsafe impl Send for LockStackEntry {}
unsafe impl Sync for LockStackEntry {}

/// Per-CPU 锁栈，用于强制锁获取顺序。
///
/// 每个核心维护一个当前持有锁的栈。
/// 获取新锁时，SpinLock 检查新锁的级别是否高于栈顶。
pub struct LockStack {
    pub entries: [LockStackEntry; Self::MAX_DEPTH],
    pub depth: usize,
}

impl LockStack {
    pub const MAX_DEPTH: usize = 8;

    #[must_use]
    pub const fn new() -> Self {
        Self {
            entries: [LockStackEntry {
                lock_ptr: core::ptr::null(),
                level: 0,
            }; Self::MAX_DEPTH],
            depth: 0,
        }
    }
}

#[repr(C, align(128))]
pub struct PerCpu {
    pub core_id: usize,
    pub lock_stack: LockStack,
}

impl PerCpu {
    #[must_use]
    pub const fn new(id: usize) -> Self {
        Self {
            core_id: id,
            lock_stack: LockStack::new(),
        }
    }
}

static PER_CPU_ARRAY: SyncUnsafeCell<[PerCpu; config::MAX_CORE_COUNT]> = SyncUnsafeCell::new([
    PerCpu::new(0),
    PerCpu::new(1),
    PerCpu::new(2),
    PerCpu::new(3),
]);

pub fn current_core_id() -> usize {
    #[cfg(target_arch = "riscv64")]
    {
        let id: usize;
        // SAFETY: tp 寄存器在 boot.S 中由 mv tp, a0 设置为 hart ID
        unsafe { core::arch::asm!("mv {id}, tp", id = out(reg) id) };
        id
    }
    #[cfg(target_arch = "aarch64")]
    {
        let mpidr: u64;
        // SAFETY: MPIDR_EL1 在 EL1 下始终可读
        unsafe { core::arch::asm!("mrs {mpidr}, mpidr_el1", mpidr = out(reg) mpidr) };
        (mpidr & 0xFF) as usize
    }
    #[cfg(not(any(target_arch = "riscv64", target_arch = "aarch64")))]
    {
        // 宿主机 (x86_64/aarch64-linux) 回退，用于单元测试——始终返回核心 0
        0
    }
}

/// 返回当前核心的 `PerCpu` 数据的可变引用。
///
/// # Safety
/// 必须在中断关闭时调用（例如 SpinLock 临界区内），
/// 以防止同核心上的并发访问。
pub unsafe fn current_per_cpu() -> &'static mut PerCpu {
    let core_id = current_core_id();
    // SAFETY: core_id < MAX_CORE_COUNT 由硬件保证；
    // 调用方保证无并发访问（中断已关闭）
    unsafe { &mut *(PER_CPU_ARRAY.get() as *mut PerCpu).add(core_id) }
}
