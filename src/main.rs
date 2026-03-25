#![cfg_attr(not(test), no_std)]
#![cfg_attr(not(test), no_main)]
#![cfg_attr(not(test), feature(alloc_error_handler))]
#![feature(sync_unsafe_cell)]
// 测试模式下部分模块不编译（arch, fdt, lang_items），导致它们的消费者
// 产生 dead_code 警告。这些代码在目标架构上被正常使用。
#![cfg_attr(test, allow(dead_code))]

#[cfg(not(test))]
mod arch;
mod config;
mod elf;
mod error;
#[cfg(not(test))]
mod fdt;
mod fmt_buf;
mod halt;
#[cfg(not(test))]
mod lang_items;
mod logging;
mod panic;
mod per_cpu;
mod scope_guard;
mod sync;

#[cfg(not(test))]
use core::sync::atomic::{AtomicU64, Ordering};

#[cfg(not(test))]
#[used]
static RODATA_SENTINEL: [u8; 1] = [0x42];

#[cfg(not(test))]
#[used]
static DATA_SENTINEL: AtomicU64 = AtomicU64::new(1);

#[cfg(not(test))]
#[used]
static BSS_SENTINEL: AtomicU64 = AtomicU64::new(0);

#[cfg(not(test))]
#[unsafe(no_mangle)]
pub extern "C" fn _start(argc: i32, argv: *const *const u8) -> ! {
    DATA_SENTINEL.store(2, Ordering::Relaxed);
    // SAFETY: 验证 .rodata 段正确加载；volatile 防止优化消除读取
    let _ = unsafe { core::ptr::read_volatile(&RODATA_SENTINEL[0]) };
    let _ = BSS_SENTINEL.load(Ordering::Relaxed);

    arch::bootstrap(argc, argv);
}

#[cfg(not(test))]
pub fn phase2_smoke_test() {
    use sync::SpinLock;

    log::info!("Testing SpinLock...");
    let lock = SpinLock::new(42u32, "smoke_test");
    {
        let mut guard = lock.lock();
        assert_eq!(*guard, 42);
        *guard = 99;
    }
    {
        let guard = lock.lock();
        assert_eq!(*guard, 99);
    }
    assert!(!lock.is_locked());
    log::info!("SpinLock OK");

    log::info!("Initializing ELF parser...");
    let elf_addr = per_cpu::BASIC_INFO
        .get()
        .expect("BASIC_INFO not initialized")
        .elf_addr;
    // SAFETY: elf_addr 是内核自身的 ELF 基地址，在内核生命周期内有效
    unsafe { panic::init_elf(elf_addr) };
    log::info!("ELF parser OK");

    log::info!("TCB ownership prototype validated (host-only unit tests)");
    log::info!("Phase 2 complete");

    log::info!("Testing panic handler (will halt)...");
    panic!("test panic");
}

#[cfg(test)]
mod tcb_ownership_prototype;
