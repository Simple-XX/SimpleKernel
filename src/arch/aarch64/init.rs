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

    // SAFETY: U-Boot bootm 传入的 argv 至少包含 3 个元素
    let argv2 = unsafe { *argv.add(2) };
    if argv2.is_null() {
        return 0;
    }

    let mut buf = [0u8; 32];
    let mut len = 0;
    for i in 0..buf.len() {
        // SAFETY: 在 null 终止的 C 字符串范围内读取，受 buf 大小限制
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

    // SAFETY: argv 由 _start 传入，_start 通过 boot.S 从 U-Boot 接收
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

    // SAFETY: 链接器定义的符号，地址在内核生命周期内有效
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
    });

    log::info!("FDT: found {} nodes, {} CPUs", node_count, core_count);
    log::info!("Memory: {} MB", mem_size / (1024 * 1024));
    log::info!("Hello SimpleKernel");
    logging::flush();
}

/// TODO(P4): 从核架构初始化（当前为空）
#[allow(dead_code)]
pub fn arch_init_smp(_argc: i32, _argv: *const *const u8) {}
