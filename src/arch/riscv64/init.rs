use crate::fdt::KernelFdt;
use crate::logging;
use crate::memory::address::PhysAddr;
use crate::per_cpu::{BASIC_INFO, BasicInfo};

unsafe extern "C" {
    static __executable_start: u8;
    static _end: u8;
}

macro_rules! fatal {
    ($msg:expr) => {{
        logging::raw_put(concat!("FATAL: ", $msg, "\n"));
        loop {
            core::hint::spin_loop();
        }
    }};
}

pub fn arch_init(_argc: i32, argv: *const *const u8) {
    logging::init();

    // riscv64 上 OpenSBI 传递：a0 = hart ID，a1 = DTB 地址。
    // boot.S 将 a1 作为第二个 C 参数（argv）转发。
    let dtb_addr = argv as usize;

    let fdt = match KernelFdt::new(dtb_addr) {
        Ok(f) => f,
        Err(_) => fatal!("Failed to parse FDT"),
    };

    let node_count = fdt.node_count();
    let core_count = fdt.core_count().unwrap_or(1);

    let (mem_addr, mem_size) = match fdt.memory() {
        Ok(m) => m,
        Err(_) => fatal!("Failed to get memory info from FDT"),
    };

    // SAFETY: 链接器定义的符号，地址在内核生命周期内有效
    let kernel_start = unsafe { &__executable_start as *const u8 as u64 };
    let kernel_end = unsafe { &_end as *const u8 as u64 };

    BASIC_INFO.call_once(|| BasicInfo {
        physical_memory_addr: PhysAddr::new(mem_addr as usize),
        physical_memory_size: mem_size,
        kernel_addr: PhysAddr::new(kernel_start as usize),
        kernel_size: (kernel_end - kernel_start) as usize,
        elf_addr: PhysAddr::new(kernel_start as usize),
        fdt_addr: PhysAddr::new(dtb_addr),
        core_count,
    });

    log::info!("FDT: found {} nodes, {} CPUs", node_count, core_count);
    log::info!("Memory: {} MB", mem_size / (1024 * 1024));
    log::info!("Hello SimpleKernel");
    logging::flush();
}

/// TODO(P4): 从核架构初始化（当前为空）
#[allow(dead_code)]
pub fn arch_init_smp(_argc: i32, _argv: *const *const u8) {}
