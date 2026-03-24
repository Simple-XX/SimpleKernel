#![no_std]
#![no_main]
#![feature(alloc_error_handler)]
#![feature(sync_unsafe_cell)]

mod arch;
mod config;
mod error;
mod fdt;
mod lang_items;
mod logging;
mod per_cpu;

use core::sync::atomic::{AtomicU64, Ordering};

#[used]
static RODATA_SENTINEL: [u8; 1] = [0x42];

#[used]
static DATA_SENTINEL: AtomicU64 = AtomicU64::new(1);

#[used]
static BSS_SENTINEL: AtomicU64 = AtomicU64::new(0);

#[unsafe(no_mangle)]
pub extern "C" fn _start(argc: i32, argv: *const *const u8) -> ! {
    DATA_SENTINEL.store(2, Ordering::Relaxed);
    // SAFETY: verifies .rodata section loaded correctly; volatile prevents optimization
    let _ = unsafe { core::ptr::read_volatile(&RODATA_SENTINEL[0]) };
    let _ = BSS_SENTINEL.load(Ordering::Relaxed);

    arch::bootstrap(argc, argv);
}
