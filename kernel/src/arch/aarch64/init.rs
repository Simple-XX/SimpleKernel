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

unsafe fn get_dtb_addr_from_argv(argv: *const *const u8) -> u64 {
    if argv.is_null() {
        return 0;
    }

    // SAFETY: U-Boot bootm passes argv with at least 3 elements
    let argv2 = unsafe { *argv.add(2) };
    if argv2.is_null() {
        return 0;
    }

    let mut buf = [0u8; 32];
    let mut len = 0;
    for i in 0..buf.len() {
        // SAFETY: reading within null-terminated C string, bounded by buf size
        let c = unsafe { *argv2.add(i) };
        if c == 0 {
            break;
        }
        buf[i] = c;
        len += 1;
    }

    let s = core::str::from_utf8(&buf[..len]).unwrap_or("");
    let hex = s
        .strip_prefix("0x")
        .or_else(|| s.strip_prefix("0X"))
        .unwrap_or(s);
    u64::from_str_radix(hex, 16).unwrap_or(0)
}

pub fn arch_init(_argc: i32, argv: *const *const u8) {
    logging::init();

    // SAFETY: argv is passed from _start which receives it from U-Boot via boot.S
    let dtb_addr = unsafe { get_dtb_addr_from_argv(argv) } as usize;

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

    // SAFETY: linker-defined symbols, addresses valid throughout kernel lifetime
    let kernel_start = unsafe { &__executable_start as *const u8 as u64 };
    let kernel_end = unsafe { &_end as *const u8 as u64 };

    let interval: u64;
    // SAFETY: CNTFRQ_EL0 is always readable at EL1
    unsafe {
        core::arch::asm!("mrs {interval}, cntfrq_el0", interval = out(reg) interval);
    }

    BASIC_INFO.call_once(|| BasicInfo {
        physical_memory_addr: mem_addr,
        physical_memory_size: mem_size,
        kernel_addr: kernel_start,
        kernel_size: (kernel_end - kernel_start) as usize,
        elf_addr: kernel_start,
        fdt_addr: dtb_addr as u64,
        core_count,
        interval: interval as usize,
    });

    log::info!("FDT: found {} nodes, {} CPUs", node_count, core_count);
    log::info!("Memory: {} MB", mem_size / (1024 * 1024));
    log::info!("Hello SimpleKernel");
    logging::flush();
}

pub fn arch_init_smp(_argc: i32, _argv: *const *const u8) {}
