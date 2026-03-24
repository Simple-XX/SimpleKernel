use crate::config;
use core::cell::SyncUnsafeCell;
use spin::Once;

pub struct BasicInfo {
    pub physical_memory_addr: u64,
    pub physical_memory_size: usize,
    pub kernel_addr: u64,
    pub kernel_size: usize,
    pub elf_addr: u64,
    pub fdt_addr: u64,
    pub core_count: usize,
    pub interval: usize,
}

impl BasicInfo {
    #[must_use]
    pub const fn new() -> Self {
        Self {
            physical_memory_addr: 0,
            physical_memory_size: 0,
            kernel_addr: 0,
            kernel_size: 0,
            elf_addr: 0,
            fdt_addr: 0,
            core_count: 0,
            interval: 0,
        }
    }
}

pub static BASIC_INFO: Once<BasicInfo> = Once::new();

#[repr(C, align(128))]
pub struct PerCpu {
    pub core_id: usize,
}

impl PerCpu {
    #[must_use]
    pub const fn new(id: usize) -> Self {
        Self { core_id: id }
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
        // SAFETY: tp register is set to hart ID by boot.S (mv tp, a0)
        unsafe { core::arch::asm!("mv {id}, tp", id = out(reg) id) };
        id
    }
    #[cfg(target_arch = "aarch64")]
    {
        let mpidr: u64;
        // SAFETY: MPIDR_EL1 is always readable at EL1
        unsafe { core::arch::asm!("mrs {mpidr}, mpidr_el1", mpidr = out(reg) mpidr) };
        (mpidr & 0xFF) as usize
    }
}

pub fn current_per_cpu() -> *mut PerCpu {
    let core_id = current_core_id();
    // SAFETY: core_id < MAX_CORE_COUNT guaranteed by hardware; array is statically allocated
    unsafe { (PER_CPU_ARRAY.get() as *mut PerCpu).add(core_id) }
}
