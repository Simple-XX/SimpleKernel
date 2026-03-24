use crate::fdt::KernelFdt;
use crate::logging;
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

    // On riscv64, OpenSBI passes: a0 = hart ID, a1 = DTB address.
    // boot.S forwards a1 as the second C argument (argv).
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

    let timebase = fdt.timebase_frequency().unwrap_or(0);

    // SAFETY: linker-defined symbols, addresses valid throughout kernel lifetime
    let kernel_start = unsafe { &__executable_start as *const u8 as u64 };
    let kernel_end = unsafe { &_end as *const u8 as u64 };

    BASIC_INFO.call_once(|| BasicInfo {
        physical_memory_addr: mem_addr,
        physical_memory_size: mem_size,
        kernel_addr: kernel_start,
        kernel_size: (kernel_end - kernel_start) as usize,
        elf_addr: kernel_start,
        fdt_addr: dtb_addr as u64,
        core_count,
        interval: timebase as usize,
    });

    log::info!("FDT: found {} nodes, {} CPUs", node_count, core_count);
    log::info!("Memory: {} MB", mem_size / (1024 * 1024));
    log::info!("Hello SimpleKernel");
    logging::flush();
}

pub fn arch_init_smp(_argc: i32, _argv: *const *const u8) {}
